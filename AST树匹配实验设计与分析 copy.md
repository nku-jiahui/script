# AST树匹配实验设计与分析

## 1. 实验目标

### 1.1 主要目标
1. **验证树匹配算法的有效性**：评估基于TreeVector的AST树匹配算法在微码映射中的准确性
2. **性能分析**：测量不同规模下的匹配时间和内存使用
3. **准确性评估**：比较树匹配结果与人工标注的匹配结果
4. **参数调优**：找到最优的相似度阈值和匹配参数

### 1.2 次要目标
1. **内联处理效果评估**：分析内联处理对匹配准确性的影响
2. **内存优化效果验证**：验证内存管理优化的效果
3. **可扩展性分析**：评估算法在大规模数据集上的表现

## 2. 实验设计

### 2.1 数据集设计

#### 2.1.1 硬件数据集
- **来源**：华为硬件加速器的AST文件
- **规模**：10-100个硬件函数
- **复杂度**：不同节点数（50-500节点）
- **类型**：不同类型的硬件加速器（计算、存储、网络等）

#### 2.1.2 软件数据集
- **来源**：对应的软件实现AST文件
- **规模**：100-1000个软件函数
- **复杂度**：不同节点数（100-2000节点）
- **类型**：包含bundle和function的混合数据集

#### 2.1.3 标注数据集
- **人工标注**：专家标注的硬件-软件对应关系
- **标注内容**：函数级别的对应关系、相似度分数
- **标注标准**：功能等价性、语义相似性

### 2.2 实验配置

#### 2.2.1 硬件配置
```
CPU: Intel Xeon E5-2680 v4 (14核28线程)
内存: 64GB DDR4
存储: SSD
操作系统: Linux CentOS 7
```

#### 2.2.2 软件配置
```
编译器: LLVM 15.0
MLIR版本: 最新稳定版
Boost版本: 1.79
Python版本: 3.8+
```

#### 2.2.3 算法参数
```
相似度阈值: [0.1, 0.3, 0.5, 0.7, 0.9]
最大匹配结果数: [5, 10, 20, 50]
内联深度阈值: [3, 5, 7, 10]
TreeVector维度: [50, 100, 200]
```

### 2.3 实验流程

#### 2.3.1 预处理阶段
1. **数据清洗**：去除无效的AST文件
2. **格式统一**：确保AST格式一致
3. **标注准备**：准备人工标注的对应关系

#### 2.3.2 实验执行阶段
1. **基线实验**：不进行内联处理的树匹配
2. **内联实验**：进行内联处理的树匹配
3. **参数调优实验**：不同参数组合的匹配
4. **性能测试**：大规模数据集的性能测试

#### 2.3.3 结果分析阶段
1. **准确性分析**：计算精确率、召回率、F1分数
2. **性能分析**：分析时间和内存使用
3. **参数分析**：找到最优参数组合

## 3. 评估指标

### 3.1 准确性指标

#### 3.1.1 精确率 (Precision)
```
Precision = TP / (TP + FP)
```
- TP: 正确匹配的硬件-软件对
- FP: 错误匹配的硬件-软件对

#### 3.1.2 召回率 (Recall)
```
Recall = TP / (TP + FN)
```
- FN: 未匹配但应该匹配的硬件-软件对

#### 3.1.3 F1分数
```
F1 = 2 × (Precision × Recall) / (Precision + Recall)
```

#### 3.1.4 平均相似度
```
Average Similarity = Σ(similarity_score) / N
```
- N: 匹配结果总数

### 3.2 性能指标

#### 3.2.1 时间性能
- **总处理时间**：从输入到输出的总时间
- **树匹配时间**：纯树匹配算法的时间
- **内联处理时间**：内联处理的时间
- **内存清理时间**：内存管理的时间

#### 3.2.2 内存性能
- **峰值内存使用**：处理过程中的最大内存使用
- **平均内存使用**：整个处理过程的平均内存使用
- **内存泄漏检测**：长时间运行后的内存增长

#### 3.2.3 可扩展性指标
- **时间复杂度**：处理时间与数据规模的关系
- **空间复杂度**：内存使用与数据规模的关系
- **并行效率**：多线程处理的加速比

### 3.3 质量指标

#### 3.3.1 匹配质量
- **匹配覆盖率**：成功匹配的硬件函数比例
- **匹配多样性**：匹配结果的多样性
- **匹配稳定性**：多次运行结果的一致性

#### 3.3.2 算法鲁棒性
- **参数敏感性**：结果对参数变化的敏感程度
- **数据噪声容忍度**：对AST噪声的容忍程度
- **边界情况处理**：极端情况的处理能力

## 4. 实验数据收集

### 4.1 需要收集的数据

#### 4.1.1 基础统计信息
```
硬件AST统计：
- 总节点数
- 最大深度
- 平均深度
- 节点类型分布
- 全局变量数量

软件AST统计：
- 总节点数
- 最大深度
- 平均深度
- Bundle数量
- Function数量
- 跳转指令数量
```

#### 4.1.2 匹配结果数据
```
匹配结果：
- 硬件函数ID
- 软件函数ID
- 相似度分数
- 匹配时间
- 匹配算法参数

内联处理数据：
- 内联次数
- 内联节点数
- 内联处理时间
- 内联前后节点数变化
```

#### 4.1.3 性能监控数据
```
时间数据：
- 各阶段处理时间
- 总处理时间
- 内存使用时间序列

内存数据：
- 峰值内存使用
- 内存使用时间序列
- 垃圾回收次数
- 内存泄漏检测
```

#### 4.1.4 质量评估数据
```
准确性数据：
- 人工标注的对应关系
- 算法预测的对应关系
- 精确率、召回率、F1分数
- 相似度分数分布

质量数据：
- 匹配覆盖率
- 匹配多样性
- 结果稳定性
```

### 4.2 数据收集方法

#### 4.2.1 自动化收集
```python
# 性能监控脚本
import time
import psutil
import json

class PerformanceMonitor:
    def __init__(self):
        self.start_time = time.time()
        self.memory_usage = []
        self.stage_times = {}
    
    def record_stage(self, stage_name):
        current_time = time.time()
        self.stage_times[stage_name] = current_time - self.start_time
    
    def record_memory(self):
        process = psutil.Process()
        memory_info = {
            'timestamp': time.time(),
            'rss': process.memory_info().rss,
            'vms': process.memory_info().vms
        }
        self.memory_usage.append(memory_info)
    
    def save_results(self, filename):
        results = {
            'stage_times': self.stage_times,
            'memory_usage': self.memory_usage,
            'total_time': time.time() - self.start_time
        }
        with open(filename, 'w') as f:
            json.dump(results, f, indent=2)
```

#### 4.2.2 结果分析脚本
```python
# 结果分析脚本
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

class ResultAnalyzer:
    def __init__(self, results_file):
        self.results = pd.read_csv(results_file)
    
    def calculate_metrics(self):
        # 计算精确率、召回率、F1分数
        precision = len(self.results[self.results['is_correct'] == True]) / len(self.results)
        recall = len(self.results[self.results['is_correct'] == True]) / len(self.results[self.results['should_match'] == True])
        f1 = 2 * (precision * recall) / (precision + recall)
        
        return {
            'precision': precision,
            'recall': recall,
            'f1_score': f1,
            'avg_similarity': self.results['similarity'].mean()
        }
    
    def plot_performance(self):
        # 绘制性能图表
        fig, axes = plt.subplots(2, 2, figsize=(12, 10))
        
        # 相似度分布
        axes[0, 0].hist(self.results['similarity'], bins=20)
        axes[0, 0].set_title('Similarity Score Distribution')
        
        # 处理时间vs数据规模
        axes[0, 1].scatter(self.results['data_size'], self.results['processing_time'])
        axes[0, 1].set_title('Processing Time vs Data Size')
        
        # 内存使用时间序列
        axes[1, 0].plot(self.results['timestamp'], self.results['memory_usage'])
        axes[1, 0].set_title('Memory Usage Over Time')
        
        # 参数敏感性分析
        axes[1, 1].boxplot([self.results[self.results['threshold'] == t]['f1_score'] 
                           for t in self.results['threshold'].unique()])
        axes[1, 1].set_title('F1 Score vs Threshold')
        
        plt.tight_layout()
        plt.savefig('performance_analysis.png')
```

## 5. 实验报告结构

### 5.1 实验报告大纲

#### 5.1.1 摘要
- 实验目的和背景
- 主要发现和结论
- 关键贡献

#### 5.1.2 引言
- 问题背景
- 相关工作
- 实验动机

#### 5.1.3 实验设计
- 数据集描述
- 实验配置
- 评估指标

#### 5.1.4 实验结果
- 准确性结果
- 性能结果
- 参数分析结果

#### 5.1.5 讨论
- 结果分析
- 局限性讨论
- 未来工作

### 5.2 关键图表

#### 5.2.1 准确性图表
- 精确率-召回率曲线
- F1分数vs相似度阈值
- 不同数据规模的准确性对比

#### 5.2.2 性能图表
- 处理时间vs数据规模
- 内存使用时间序列
- 并行加速比

#### 5.2.3 质量图表
- 相似度分数分布
- 匹配覆盖率vs参数
- 结果稳定性分析

## 6. 实验执行建议

### 6.1 实验准备
1. **数据预处理**：确保数据质量和格式一致性
2. **环境配置**：统一实验环境和参数设置
3. **基线建立**：建立对比基线（如随机匹配、简单字符串匹配）

### 6.2 实验执行
1. **分阶段执行**：先小规模验证，再大规模测试
2. **参数网格搜索**：系统性地测试参数组合
3. **多次重复**：确保结果的统计显著性

### 6.3 结果验证
1. **交叉验证**：使用不同的数据集验证结果
2. **人工验证**：对关键结果进行人工检查
3. **统计分析**：使用统计方法验证结果的显著性

## 7. 预期结果

### 7.1 准确性预期
- **精确率**：> 0.8（在相似度阈值0.7以上）
- **召回率**：> 0.6（能够找到大部分正确匹配）
- **F1分数**：> 0.7（平衡的准确性表现）

### 7.2 性能预期
- **处理时间**：< 10秒（1000节点规模）
- **内存使用**：< 4GB（峰值使用）
- **可扩展性**：线性或次线性增长

### 7.3 质量预期
- **匹配覆盖率**：> 80%（能够匹配大部分硬件函数）
- **结果稳定性**：标准差 < 0.1（多次运行结果一致）
- **参数鲁棒性**：在合理参数范围内结果稳定 