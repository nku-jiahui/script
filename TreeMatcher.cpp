//===- src/lib/HuaweiAST/TreeMatcher.cpp - 树匹配器实现 -*- C++ -*-===//
//
// 华为微码映射项目 - 树匹配器实现
//
//===----------------------------------------------------------------------===//

#include "MicrocodeMapping/MicrocodeMappingEngine.h"
#include "MicrocodeMapping/tree-vector.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace HMCM
{

// 通用函数：计算相似度并更新最佳匹配结果
void updateBestMatch(TreeVector* treeVector, TreeVector* targetVector, std::vector<TreeMatchResult>& results,std::string location,int mergedCount)
{
  
  if (!treeVector)
  {
    return;
  }
  
  double similarity = TreeMatcher::calculateTreeVectorSimilarity(treeVector, targetVector);
  
  // 创建匹配结果
  std::cout<<"mergedCount: "<<mergedCount<<std::endl;
  TreeMatchResult result;
  result.similarity = similarity;
  result.location = location;
  results.push_back(result);
  
  // 按相似度排序
  std::sort(results.begin(), results.end(),
            [](const TreeMatchResult& a, const TreeMatchResult& b)
            {
              return a.similarity > b.similarity;
            });

  // 取排名第一的结果
  if (results.size() > 1)
  {
    results.resize(1);
  }
  return;
}

  void foo(BaseAST *node, TreeVector *targetVector, std::vector<TreeMatchResult> &newresults,int accNodeCount=0,
    int mergedCount=1,TreeVector *mergedVector=nullptr,std::string location="",int swNodeCount=0)
  {
    //1.是有意义的结点
    bool isSubtree = node->getIsSubtreeNode();
    if(!isSubtree){
      return;
    }
    if(swNodeCount==0){
      swNodeCount = node->getNumChildren();
    }
    else{
      swNodeCount+=node->getNumChildren();
    }
    if(mergedVector==nullptr){
      TreeVector* nodeVector = node->getVector();
      if(nodeVector != nullptr){
        mergedVector = nodeVector;
      }
    }
    else{
      TreeVector* nodeVector = node->getVector();
      if(nodeVector != nullptr){
        *mergedVector += *nodeVector;
      }
    }
    if(location==""){
      location = node->getLoc();
    }
    else{
      location=node->getLoc();
    }

    if(swNodeCount>accNodeCount*1.5){
      std::cout<<"swNodeCount>accNodeCount*1.5"<<std::endl;
      std::cout<<"node text: "<<node->getText()<<std::endl;
      std::cout<<"swNodeCount: "<<swNodeCount<<std::endl;
      std::cout<<"accNodeCount: "<<accNodeCount<<std::endl;
      updateBestMatch(mergedVector, targetVector, newresults,location,mergedCount);
      return;
    }
    else if(swNodeCount<accNodeCount*0.5){
      std::cout<<"swNodeCount<accNodeCount*0.5"<<std::endl;
      std::cout<<"node text: "<<node->getText()<<std::endl;
      std::cout<<"swNodeCount: "<<swNodeCount<<std::endl;
      std::cout<<"accNodeCount: "<<accNodeCount<<std::endl;
      if(node->getRight()){
        std::cout<<"node getRight"<<std::endl;
        foo(node->getRight(), targetVector, newresults,accNodeCount,mergedCount+1,mergedVector,location,swNodeCount);
      }
      return;
    }
    else{
      updateBestMatch(mergedVector, targetVector, newresults,location,mergedCount);
      std::cout<<"符合条件"<<std::endl;
      
      if(node->getRight()){
        foo(node->getRight(), targetVector, newresults,accNodeCount,mergedCount+1,mergedVector,location,swNodeCount);
      }
      return;
    }
    // 使用封装的函数计算相似度并更新最佳匹配
  }
  void DFS(BaseAST *node, TreeVector *targetVector, BaseAST *swNode, std::vector<TreeMatchResult> &newresults,int accNodeCount)
  {
    if (!node)
    {
      return;
    }
    auto child = node->getDown();
    while(child)
    {
      DFS(child, targetVector, swNode, newresults,accNodeCount);
      child = child->getRight();
    }
    int mergedCount = 1;
    foo(node, targetVector, newresults,accNodeCount,mergedCount);
  }

  TreeMatcher::TreeMatcher(const HardwareASTStats &hwStats, size_t maxMatchResults)
      : hwStats(hwStats), maxMatchResults(maxMatchResults)
  {
  }

  TreeMatcher::~TreeMatcher()
  {
  }

  std::vector<TreeMatchResult> TreeMatcher::matchTrees(const std::vector<BaseAST *> &softwareNodes)
  {
    std::vector<TreeMatchResult> results;

    // std::cout << "开始树匹配，软件节点数: " << softwareNodes.size() << std::endl;

    // 获取硬件AST的根节点
    BaseAST *hardwareRoot = hwStats.root;
    if (!hardwareRoot)
    {
      std::cerr << "错误：硬件AST根节点为空" << std::endl;
      return results;
    }

    // 生成硬件AST的TreeVector作为目标向量
    TreeVector *targetVector = TreeVector::nodeVectorGen(hardwareRoot);
    int accNodeCount = hardwareRoot->getNumChildren()-13;//acc的节点数
    std::cout<<"accNodeCount: "<<accNodeCount<<std::endl;
    if (!targetVector)
    {
      std::cerr << "错误：无法生成硬件AST的TreeVector" << std::endl;
      return results;
    }
    TreeAccessor::printVec(hardwareRoot);
    // 打印name_counters
    for (const auto &pair : targetVector->name_counters)
    {
      std::cout << pair.first << ": " << pair.second << "       ";
    }
    std::cout << std::endl;

    // std::cout << "硬件AST TreeVector生成成功" << std::endl;
    // 3.2 软件AST生成各节点向量并与硬件AST比较
    for (BaseAST *swNode : softwareNodes)
    {
      //为一个bundle的所有结点生成特征向量
      TreeVector *treeVector = TreeVector::nodeVectorGen(swNode);
      //newresults是针对一个bundle的，存放bundle里面结果最好的子树结点的位置，和相似度
      std::vector<TreeMatchResult> newresults;
      //DFS针对一个bundle，深度优先遍历，遍历bundle的所有结点，计算相似度并收集结果
      DFS(swNode, targetVector, swNode, newresults,accNodeCount);
      
      if (!newresults.empty())
      {
        TreeMatchResult result;
        result.matchedNode = swNode;
        result.similarity = newresults[0].similarity;
        result.location = newresults[0].location;
        results.push_back(result);
      }
    }
  
    
    // 按相似度排序（只有在有结果时才排序）
    if (!results.empty())
    {
      std::sort(results.begin(), results.end(),
                [](const TreeMatchResult &a, const TreeMatchResult &b)
                {
                  return a.similarity > b.similarity;
                });
      //前20个bundle，node，simility，location         
      // 取排名前N的结果（使用配置参数）
      if (results.size() > maxMatchResults)
      {
        results.resize(maxMatchResults);
      }
    }
    // for (const auto &result : results)
    // {
    //   auto bundle / function = result.bundle；
    //                                newresult.match
    //                                    newresult.sim
    //                                        newresult.location = result.matchnode.loc,
    //                 newresult.pushback(newresult)
    // }

    // 清理TreeVector内存 - 新增内存清理
    // cleanupTreeVectors(softwareNodes);

    // 打印排名前N的结果信息
    std::cout << "树匹配完成，找到 " << results.size() << " 个匹配结果（取排名前" << maxMatchResults << "）" << std::endl;
    if (!results.empty())
    {
      std::cout << "相似度范围: " << results.back().similarity << " - " << results.front().similarity << std::endl;
    }

    return results;
  }

  // 新增：清理TreeVector内存的方法
  void TreeMatcher::cleanupTreeVectors(const std::vector<BaseAST *> &nodes)
  {
    for (BaseAST *node : nodes)
    {
      if (node)
      {
        cleanupTreeVectorsRecursive(node);
      }
    }
  }

  // 递归清理AST树中所有节点的TreeVector（与生成逻辑保持一致）
  void TreeMatcher::cleanupTreeVectorsRecursive(BaseAST *node)
  {
    if (!node)
      return;

    // 清理当前节点的TreeVector
    auto attr_itr = node->attributes.find(NodeAttributeName_t::NODE_VECTOR);
    if (attr_itr != node->attributes.end())
    {
      try
      {
        TreeVector *treeVector = std::any_cast<TreeVector *>(attr_itr->second);
        if (treeVector)
        {
          delete treeVector; // 调用析构函数自动清理
        }
      }
      catch (const std::bad_any_cast &)
      {
        // 忽略类型转换错误
      }
      node->attributes.erase(attr_itr);
    }

    // 递归清理子节点（与生成逻辑保持一致，不访问兄弟节点）
    for (BaseAST *child = node->getDown(); child != nullptr; child = child->getRight())
    {
      cleanupTreeVectorsRecursive(child);
    }
  }

  std::vector<double> TreeMatcher::generateNodeVector(BaseAST *node)
  {
    std::vector<double> vector;

    if (!node)
      return vector;

    // 使用TreeVector生成特征向量
    TreeVector *treeVector = TreeVector::nodeVectorGen(node);
    if (!treeVector)
    {
      return vector;
    }

    // 将TreeVector的counters转换为double向量
    for (int count : treeVector->counters)
    {
      vector.push_back(static_cast<double>(count));
    }

    // 清理TreeVector（避免内存泄漏）
    // 注意：TreeVector已经存储在node的attributes中，这里不需要手动删除

    return vector;
  }

  double TreeMatcher::calculateEuclideanDistance(const std::vector<double> &vec1, const std::vector<double> &vec2)
  {
    if (vec1.size() != vec2.size())
    {
      // 如果向量长度不同，进行填充或截断
      size_t maxSize = std::max(vec1.size(), vec2.size());
      std::vector<double> paddedVec1 = vec1;
      std::vector<double> paddedVec2 = vec2;

      paddedVec1.resize(maxSize, 0.0);
      paddedVec2.resize(maxSize, 0.0);

      return calculateEuclideanDistance(paddedVec1, paddedVec2);
    }

    double sum = 0.0;
    for (size_t i = 0; i < vec1.size(); ++i)
    {
      double diff = vec1[i] - vec2[i];
      sum += diff * diff;
    }

    return std::sqrt(sum);
  }

  double TreeMatcher::calculateSimilarity(const std::vector<double> &vec1, const std::vector<double> &vec2)
  {
    if (vec1.empty() || vec2.empty())
    {
      return 0.0;
    }

    // 使用余弦相似度
    double dotProduct = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;

    size_t maxSize = std::max(vec1.size(), vec2.size());
    for (size_t i = 0; i < maxSize; ++i)
    {
      double val1 = (i < vec1.size()) ? vec1[i] : 0.0;
      double val2 = (i < vec2.size()) ? vec2[i] : 0.0;

      dotProduct += val1 * val2;
      norm1 += val1 * val1;
      norm2 += val2 * val2;
    }

    if (norm1 == 0.0 || norm2 == 0.0)
    {
      return 0.0;
    }

    return dotProduct / (std::sqrt(norm1) * std::sqrt(norm2));
  }

  double TreeMatcher::calculateVectorSimilarity(TreeVector *treeVector)
  {
    if (!treeVector)
    {
      return 0.0;
    }

    // 这里需要一个目标TreeVector来进行比较
    // 由于当前设计中没有目标向量，我们暂时使用一个简单的评分方法
    // 后续可以传入目标TreeVector进行比较

    const std::vector<int> &vec = treeVector->counters;

    // 计算向量的复杂度作为相似度指标
    double totalCount = 0.0;
    int nonZeroCount = 0;

    for (size_t i = 1; i < vec.size(); ++i)
    { // 从1开始，忽略无关向量
      if (vec[i] > 0)
      {
        nonZeroCount++;
        totalCount += vec[i];
      }
    }

    // 基于非零元素比例和总计数计算相似度
    double nonZeroRatio = static_cast<double>(nonZeroCount) / (vec.size() - 1);
    double normalizedCount = totalCount / 100.0; // 假设最大值为100

    return (nonZeroRatio + normalizedCount) / 2.0;
  }

  // 添加一个新的静态方法用于两个TreeVector之间的相似度计算
  double TreeMatcher::calculateTreeVectorSimilarity(TreeVector *sourceVector, TreeVector *targetVector)
  {
    if (!sourceVector || !targetVector)
    {
      return 0.0;
    }

    const std::vector<int> &vec1 = sourceVector->counters;
    const std::vector<int> &vec2 = targetVector->counters;

    // 1. 比较TreeVector的向量
    if (vec1.size() != vec2.size())
    {
      return 0.0;
    }

    // 计算欧几里得距离的倒数作为相似度
    double distance = 0.0;
    int count = 0; // count为vec2的维度
    for (size_t i = 1; i < vec1.size(); ++i)
    { // 从1开始，忽略无关向量
      distance += std::pow(vec1[i] - vec2[i], 2);
      count += vec2[i];
    }
    distance = std::sqrt(distance);

    // 转换为相似度分数（距离越小，相似度越高）
    double maxDistance = count;
    // double similarity = 1.0 - std::min(distance, maxDistance) / maxDistance;
    double similarity =1/(distance+1);
    // 2. 比较name_counters
    double nameSimilarity = 0.0;
    // int totalNames = 0;
    // int matchedNames = 0;
    // for (const auto &pair : targetVector->name_counters)
    // {
    //   totalNames += pair.second;
    //   if (sourceVector->name_counters.find(pair.first) != sourceVector->name_counters.end())
    //   {
    //     matchedNames += std::min(pair.second, sourceVector->name_counters[pair.first]);
    //   }
    // }
    // if (totalNames > 0)
    // {
    //   double mismatchPenalty = 0; // 可以调整惩罚系数
    //   int unmatchedNames = totalNames - matchedNames;
    //   nameSimilarity = (matchedNames - mismatchPenalty * unmatchedNames) / totalNames;
    // }

    // 3. 比较control_vars（暂时不考虑，后续可以加上）
    // double controlVarSimilarity = 0.0;

    // 4. 综合相似度
    //double finalSimilarity = 0.8 * similarity + 0.2 * nameSimilarity;
    double finalSimilarity = similarity;
    // if (finalSimilarity > 0.5)
    // {
    //   std::cout << "特征向量相似度: " << similarity << std::endl;
    //   std::cout << "name相似度: " << nameSimilarity << std::endl;
    // }
    return finalSimilarity;
  }

  bool TreeMatcher::compareGlobalVars(const std::vector<std::string> &hwVars, const std::vector<std::string> &swVars)
  {
    // 简单的全局变量比较

    if (hwVars.empty() && swVars.empty())
    {
      return true; // 都为空，认为匹配
    }

    if (hwVars.empty() || swVars.empty())
    {
      return false; // 一个为空一个不为空，不匹配
    }

    // 计算交集大小
    std::set<std::string> hwSet(hwVars.begin(), hwVars.end());
    std::set<std::string> swSet(swVars.begin(), swVars.end());

    std::vector<std::string> intersection;
    std::set_intersection(hwSet.begin(), hwSet.end(),
                          swSet.begin(), swSet.end(),
                          std::back_inserter(intersection));

    // 如果交集大小超过一定比例，认为匹配
    double hwRatio = static_cast<double>(intersection.size()) / hwVars.size();
    double swRatio = static_cast<double>(intersection.size()) / swVars.size();

    return (hwRatio >= 0.5 && swRatio >= 0.5);
  }

} // namespace HMCM