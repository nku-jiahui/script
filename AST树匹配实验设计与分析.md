# AST树匹配实验设计与分析

## 1. 实验背景

### 1.1 匹配策略选择
采用基于AST树的相似度匹配作为初筛方法，而非直接使用图匹配。虽然图匹配相对于基于AST树的匹配可以更精确，但复杂度更高。AST树匹配能够在保证一定精度的前提下，提供更高效的筛选效果。

### 1.2 筛选方式
采用**特征向量+变量名匹配**的双重筛选机制：
- **特征向量**：体现AST树的结构相似性，主要反映AST树包含的节点类型分布
- **变量名匹配**：由于微码结构相似性很大（存在大量if、move指令等），仅基于特征向量难以精确匹配，因此需要考虑具体操作的变量名

### 1.3 代码差异处理
680代码和88代码存在显著差异：
- **680特有**：cc寄存器（cc0-cc7）、bitcmp指令、mskcmp指令等
- **88特有**：m_开头的编译器生成变量
- **共同特征**：if、move、算术运算等基础指令

## 2. 预处理阶段：Inline处理

### 2.1 Inline处理目的
在特征向量生成之前，对bundle或function进行inline处理，处理内部的jmp和function_call指令。

### 2.2 Inline限制策略
依据目标ACC的深度和节点数量对inline次数进行限制，防止无限制的inline导致函数变得过大，影响匹配效率。

## 3. 特征向量生成

### 3.1 节点类型分类
特征向量维度为16，对应ASTNodeCategory枚举：
- Irrelevant：无关节点
- Identifier：标识符/常量  
- ControlFlow：控制流
- ArithmeticAddSub：加减运算
- ArithmeticMulDiv：乘除运算
- LogicalOperation：逻辑运算
- Bitwise：位运算
- Comparison：比较运算
- Registers：寄存器
- Assignment：赋值
- Function：函数相关
- DataStructure：数据结构
- Declaration：声明
- Condition：条件
- Other：其他

### 3.2 特殊处理
- **标签节点**：FUNCTION_LABEL/BUNDLE_LABEL直接跳过，生成空向量
- **if结构**：680代码的bitcmp按空处理，if第一个子节点作为Condition
- **变量过滤**：忽略cc开头、m_开头、acc相关变量

**if结构处理详解**：
- **680代码模式**：bitcmp/cmp → cc寄存器 → if判断cc值
- **88代码模式**：直接if判断条件
- **统一处理**：将if的第一个子节点作为Condition类型，收集其中的变量名

### 3.3 特征向量生成算法
采用**后序遍历**方法：
1. 递归处理所有子节点
2. 统计当前节点类型到对应维度
3. 收集符合条件的变量名到name_counters
4. 合并子节点的特征向量
5. 将结果存储在节点的NODE_VECTOR属性中

## 4. 相似度计算

### 4.1 特征向量相似度
```cpp
// 从索引1开始，忽略Irrelevant维度
double distance = 0.0;
int count = 0;
for (size_t i = 1; i < vec1.size(); ++i) {
    distance += std::pow(vec1[i] - vec2[i], 2);
    count += vec2[i];
}
distance = std::sqrt(distance);
double maxDistance = count;
double similarity = 1.0 - std::min(distance, maxDistance) / maxDistance;
```

### 4.2 变量名相似度
```cpp
int totalNames = 0;
int matchedNames = 0;
for(const auto &pair : targetVector->name_counters) {
    totalNames += pair.second;
    if(sourceVector->name_counters.find(pair.first) != sourceVector->name_counters.end()) {
        matchedNames += std::min(pair.second, sourceVector->name_counters[pair.first]);
    }
}
nameSimilarity = matchedNames / totalNames;
```

### 4.3 综合相似度
```cpp
finalSimilarity = 0.3 * similarity + 0.7 * nameSimilarity;
```

## 5. 子树筛选

### 5.1 大小限制
- 最小子树大小：4个节点（minSubtreeSize = 4）
- 最大子树大小：200个节点（maxSubtreeSize = 200）

### 5.2 有效性判断
子树必须满足以下条件：
1. 节点数量在最小和最大子树大小范围内
2. 是完整的语义单元（通过TreeAccessor::isCompleteSemanticUnit判断）

### 5.3 匹配阈值
- 相似度阈值：0.5
- 结果按相似度降序排列
- 取排名前N的结果作为最终输出

## 6. 实验优势

1. **效率高**：相比图匹配，AST树匹配计算复杂度更低
2. **精度适中**：通过特征向量+变量名双重匹配，在保证效率的同时提供合理的匹配精度
3. **可扩展性**：特征向量维度可根据需要调整，权重分配可优化
4. **鲁棒性**：通过过滤无关节点和特殊处理，减少噪声干扰

## 7. 实验结果

### 7.1 匹配效果
- 相似度阈值设置为0.5，有效过滤低质量匹配
- 通过双重匹配机制，提高了匹配的准确性
- 特殊处理策略有效减少了680和88代码差异的干扰

### 7.2 性能表现
- 特征向量生成采用后序遍历，时间复杂度O(n)
- 相似度计算复杂度较低，适合大规模匹配
- 子树筛选机制有效控制了计算量

## 8. 后续优化方向

1. **权重调优**：根据实际测试结果调整特征向量和变量名的权重比例
2. **阈值优化**：基于更多测试数据优化相似度阈值
3. **特征增强**：考虑添加更多语义特征，如控制流特征
4. **性能优化**：优化特征向量生成和相似度计算的性能 