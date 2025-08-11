//===- src/tools/microcode-mapping-test/microcode-mapping-test.cpp - 微码映射测试 -*- C++ -*-===//
//
// 华为微码映射项目 - 微码映射测试工具
//
// hwcontext(ast)->SoftwareASTAnalyzer->SoftwareASTStats->InlineProcessor->bundles
//
//===----------------------------------------------------------------------===//

#include "MicrocodeMapping/MicrocodeMappingEngine.h"
#include "CloneDetection/tree-accessor.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <functional>

using namespace HMCM;
namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
  // 开始计时
  auto start_time = std::chrono::high_resolution_clock::now();

  if (argc != 5)
  {
    std::cerr << "用法: " << argv[0] << " <硬件AST文件/目录> <软件AST文件> <结果输出目录> <符号表读取文件>" << std::endl;
    std::cerr << "注意: 如果硬件AST文件是目录，将批量处理该目录下的所有.txt文件" << std::endl;
    return 1;
  }

  std::string hardwareInput = argv[1];
  std::string softwareFile = argv[2];
  std::string resultDir = argv[3];
  std::string symbolTableFile = argv[4];
  // 检查输入文件是否存在
  if (!fs::exists(hardwareInput))
  {
    std::cerr << "硬件AST文件/目录不存在: " << hardwareInput << std::endl;
    return 1;
  }
  if (!fs::exists(softwareFile))
  {
    std::cerr << "软件AST文件不存在: " << softwareFile << std::endl;
    return 1;
  }

  // 创建
  MicrocodeMappingEngine engine;

  // 设置参数
  engine.setInlineThresholds(2, 2); // 深度差2，节点数比例2
  engine.setMatchingThreshold(0.8); // 匹配阈值0.8

  try
  {
    // 判断是文件还是目录
    bool isDirectory = fs::is_directory(hardwareInput);
    
    if (isDirectory) {
      // 批量处理模式
      std::cout << "检测到硬件目录，启用批量处理模式" << std::endl;
      
      // 1. 批量解析AST
      if (!engine.parseASTBatch(hardwareInput, softwareFile, symbolTableFile))
      {
        std::cerr << "批量AST解析失败" << std::endl;
        return 1;
      }
      std::cout << "✓ 批量AST解析成功" << std::endl;
      std::cout << std::endl;

      // 2. 前期准备（只需要一次）
      if (!engine.preprocessASTs())
      {
        std::cerr << "前期准备失败" << std::endl;
        return 1;
      }
      
      // 3. 内联处理（只需要一次）
      if (!engine.performInlineProcessing())
      {
        std::cerr << "内联处理失败" << std::endl;
        return 1;
      }
      
      // 4. 为每个硬件文件进行树匹配
      std::vector<std::string> hardwareFiles = engine.getHardwareFileList(hardwareInput);
      
      // 存储所有硬件文件的匹配结果信息
      struct HardwareMatchInfo {
          std::string name;
          int nodeCount;
          std::vector<std::pair<std::string, int>> matchedSoftware; // (软件名称, 节点数)
      };
      std::vector<HardwareMatchInfo> allResults;
      
      for (const auto& hardwareFile : hardwareFiles) {
        std::string basename = fs::path(hardwareFile).stem().string();
        std::string filename = fs::path(hardwareFile).filename().string();
        
        std::cout << "处理硬件文件: " << basename << "..." << std::endl;
        
        // 为当前硬件文件进行树匹配
        if (!engine.performTreeMatchingForHardware(basename))
        {
          std::cerr << "硬件 " << basename << " 的树匹配失败" << std::endl;
          continue;
        }
        
        const auto &matchResults = engine.getMatchResults();

        if (matchResults.empty())
        {
          std::cout << "硬件 " << basename << " 没有找到匹配结果" << std::endl;
          continue;
        }
        else
        {
          // 获取硬件文件的节点数
          int hardwareNodeCount = engine.getHardwareNodeCount(basename);
          
          // 收集匹配的软件信息
          HardwareMatchInfo hwInfo;
          hwInfo.name = basename;
          hwInfo.nodeCount = hardwareNodeCount;
          
          for (const auto& result : matchResults) {
              if (result.matchedNode) {
                  // 获取软件节点名称
                  std::string softwareNodeName = "未知";
                  if (result.matchedNode->getDown() && result.matchedNode->getDown()->getDown()) {
                      softwareNodeName = result.matchedNode->getDown()->getDown()->getText();
                  }
                  
                  // 计算软件节点数
                  int softwareNodeCount = 0;
                  std::function<void(BaseAST*)> countNodes = [&](BaseAST* node) {
                      if (!node) return;
                      softwareNodeCount++;
                      countNodes(node->getDown());
                      countNodes(node->getRight());
                  };
                  countNodes(result.matchedNode);
                  
                  hwInfo.matchedSoftware.push_back({softwareNodeName, softwareNodeCount});
              }
          }
          
          allResults.push_back(hwInfo);
          
          // 创建结果输出目录
          if (!fs::exists(resultDir))
          {
            fs::create_directories(resultDir);
          }
          
          // 清理旧的输出目录（如果存在）
          fs::path oldTestDirPath = fs::path(resultDir) / basename;
          if (fs::exists(oldTestDirPath))
          {
            fs::remove_all(oldTestDirPath);
          }
          
          // 创建以硬件文件名为名的目录
          fs::path testDirPath = fs::path(resultDir) / basename;
          if (!fs::exists(testDirPath))
          {
            fs::create_directories(testDirPath);
          }
          
          // 复制硬件AST文件到测试目录，使用原文件名
          fs::path hardAccFilePath = testDirPath / filename;
          try {
            fs::copy_file(hardwareFile, hardAccFilePath, fs::copy_options::overwrite_existing);
          } catch (const fs::filesystem_error& e) {
            std::cerr << "复制硬件AST文件失败: " << e.what() << std::endl;
            continue;
          }
          
          // 创建与文件名同名的目录
          fs::path hardAccDirPath = testDirPath / basename;
          if (!fs::exists(hardAccDirPath))
          {
            fs::create_directories(hardAccDirPath);
          }

          // 为每个匹配结果创建单独的文件
          for (size_t i = 0; i < matchResults.size(); ++i)
          {
            const auto &result = matchResults[i];
            engine.getSymbolTableManager()->collectReferences(result.matchedNode);
            // 生成结果文件名 - 使用软件代码的实际名字
            std::string softwareNodeName = "unknown";
            if (result.matchedNode->getDown() && result.matchedNode->getDown()->getDown()) {
                softwareNodeName = result.matchedNode->getDown()->getDown()->getText();
            }
            std::string resultFileName = softwareNodeName + ".txt";
            fs::path resultFilePath = hardAccDirPath / resultFileName;

            // 创建输出文件
            std::ofstream resultOut(resultFilePath);
            if (!resultOut.is_open())
            {
              std::cerr << "无法创建结果文件: " << resultFilePath << std::endl;
              continue;
            }

            // 写入结果信息
            resultOut << "=== 匹配结果 " << (i + 1) << " ===" << std::endl;
            resultOut << "相似度: " << result.similarity << std::endl;
            resultOut << "匹配的Bundle/Function: " << std::endl;
            engine.getSymbolTableManager()->patchAndInsert(resultOut);
            // 打印匹配的AST树
            TreeAccessor::printSubTreeAdjustIdent(result.matchedNode, resultOut);
            
            resultOut.close();
          }
        }
      }
      
      // 5. 输出总结报告
      std::cout << "\n" << std::string(60, '=') << std::endl;
      std::cout << "批量处理完成 - 总结报告" << std::endl;
      std::cout << std::string(60, '=') << std::endl;
      
      if (allResults.empty()) {
          std::cout << "没有找到任何匹配结果" << std::endl;
      } else {
          std::cout << "找到 " << allResults.size() << " 个硬件文件存在匹配结果：" << std::endl;
          std::cout << std::endl;
          
          for (const auto& hwInfo : allResults) {
              std::cout << "硬件文件: " << hwInfo.name << " (节点数: " << hwInfo.nodeCount << ")" << std::endl;
              std::cout << "  匹配的软件: ";
              for (size_t i = 0; i < hwInfo.matchedSoftware.size(); ++i) {
                  const auto& sw = hwInfo.matchedSoftware[i];
                  std::cout << sw.first << "(" << sw.second << "节点)";
                  if (i < hwInfo.matchedSoftware.size() - 1) {
                      std::cout << ", ";
                  }
              }
              std::cout << std::endl;
              std::cout << std::endl;
          }
      }
    } else {
      // 单文件处理模式（原有逻辑）
      std::cout << "检测到硬件文件，启用单文件处理模式" << std::endl;
      
      // 1. 解析AST
      if (!engine.parseAST(hardwareInput, softwareFile, symbolTableFile))
      {
        std::cerr << "AST解析失败" << std::endl;
        return 1;
      }
      std::cout << "✓ AST解析成功" << std::endl;
      std::cout << std::endl;

      // 2. 前期准备
      if (!engine.preprocessASTs())
      {
        std::cerr << "前期准备失败" << std::endl;
        return 1;
      }

    // 2.1增加一个环节，目前只是找到了function和bundle，还有几个问题，
    //  3. 内联处理
    // std::cout << "步骤3: 内联处理" << std::endl;
    if (!engine.performInlineProcessing())
    {
      std::cerr << "内联处理失败" << std::endl;
      return 1;
    }
    // 4. 树匹配
    // std::cout << "步骤4: 树匹配" << std::endl;
    if (!engine.performTreeMatching())
    {
      std::cerr << "树匹配失败" << std::endl;
      return 1;
    }
    // 打印硬件名称以及最高的分数及对应的软件名称
    engine.printBestMatch();
    const auto &matchResults = engine.getMatchResults();

    if (matchResults.empty())
    {
      std::cout << "没有找到匹配结果" << std::endl;
      return 2; // 返回2表示没有匹配结果
    }
    else
    {
      std::cout << "找到 " << matchResults.size() << " 个匹配结果" << std::endl;
      // 创建结果输出目录
      if (!fs::exists(resultDir))
      {
        fs::create_directories(resultDir);
      }
      // 创建以硬件文件名为名的文件夹
      std::string folderName = engine.getHardwareFileName();

      // 如果文件夹名为空，使用默认名称
      if (folderName.empty())
      {
        folderName = "output";
      }

      // 创建文件夹
      fs::path folderPath = fs::path(resultDir) / folderName;
      if (!fs::exists(folderPath))
      {
        if (fs::create_directories(folderPath))
        {
          std::cout << "成功创建文件夹: " << folderPath << std::endl;
        }
        else
        {
          std::cerr << "创建文件夹失败: " << folderPath << std::endl;
          return 1;
        }
      }
      else
      {
        std::cout << "文件夹已存在: " << folderPath << std::endl;
      }

      // 打印树匹配结果并输出到文件
      std::cout << "=== 树匹配结果 ===" << std::endl;
      // 为每个匹配结果创建单独的文件
      for (size_t i = 0; i < matchResults.size(); ++i)
      {
        const auto &result = matchResults[i];
        engine.getSymbolTableManager()->collectReferences(result.matchedNode);
        // 生成结果文件名
        std::string resultFileName = result.matchedNode->getDown()->getDown()->getText() + ".txt";
        fs::path resultFilePath = folderPath / resultFileName;

        // 创建输出文件
        std::ofstream resultOut(resultFilePath);
        if (!resultOut.is_open())
        {
          std::cerr << "无法创建结果文件: " << resultFilePath << std::endl;
          continue;
        }

        // 写入结果信息
        resultOut << "=== 匹配结果 " << (i + 1) << " ===" << std::endl;
        resultOut << "相似度: " << result.similarity << std::endl;
        resultOut << "匹配的Bundle/Function: " << std::endl;
        engine.getSymbolTableManager()->patchAndInsert(resultOut);
        // 打印匹配的AST树
        TreeAccessor::printSubTreeAdjustIdent(result.matchedNode, resultOut);
        
        resultOut.close();
      }
    }
  }}
  catch (const std::exception &e)
  {
    std::cerr << "错误: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}