//===- src/lib/TranslationValidator.cpp - TranslationValidator -*- C++
//-*-===//
//
// 华为微码映射项目
//
//===----------------------------------------------------------------------===//
//
// 创建日期：2025-04-11
// 作者：张子涵，高猛
// 描述：翻译验证，用于验证两个MLIR模块之间是否完全等价
//
//===----------------------------------------------------------------------===//

#include "TranslationValidation/TranslationValidator.h"
#include "Analysis/PDG.h"
#include "Atom/Passes.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace HMCM {
#define GEN_PASS_DEF_TRANSLATIONVALIDATION
#include "Atom/Passes.h.inc"
} // namespace HMCM

namespace HMCM {
using namespace mlir;
class TranslationValidationPass
    : public HMCM::impl::TranslationValidationBase<TranslationValidationPass> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TranslationValidationPass)
  ~TranslationValidationPass() override = default;

  void runOnOperation() override;

  void applayMemorySSA(Operation *root);

private:
  auto getCompairedFuncs() -> std::tuple<func::FuncOp, func::FuncOp>;
  void getNodes(const MatchResult &match, std::vector<Vertex> &nodes1,
                std::vector<Vertex> &nodes2) {
    for (auto &pair : match) {
      nodes1.push_back(pair.first);
      nodes2.push_back(pair.second);
    }
  }
};

void TranslationValidationPass::runOnOperation() {
  // applayMemorySSA();
  auto secondMilrPath = secondModulePath.getValue();
  auto baseNameMappingTablePath = baseNameMappingPath.getValue();
  auto moduleOp = getOperation();

  // 使用固定路径作为输出文件
  std::string outputFilePath =
      "/Users/jiahui/code/huawei-microcode-mapping/data/result.txt";

  // applayMemorySSA(moduleOp);
  // 读取目录下所有mlir文件（兼容 C++11，使用 LLVM filesystem）
  std::vector<std::string> mlirFiles;
  std::error_code ec;
  for (llvm::sys::fs::directory_iterator it(secondMilrPath, ec), end;
       it != end && !ec; it.increment(ec)) {
    llvm::StringRef pathRef = it->path();
    // 手动判断后缀，避免依赖较新API
    if (pathRef.size() >= 5 && pathRef.substr(pathRef.size() - 5) == ".mlir") {
      mlirFiles.push_back(pathRef.str());
    }
  }
  llvm::outs() << "mlirFiles.size(): " << mlirFiles.size() << "\n";
  for (auto &file : mlirFiles) {
    OwningOpRef<ModuleOp> littleModuleOp =
        parseSourceFile<ModuleOp>(file, &getContext());
    auto littlemodule = littleModuleOp->clone();
    moduleOp.push_back(littlemodule);
  }
  // moduleOp.dump();

  // 获取函数列表
  std::vector<func::FuncOp> softFuncList;
  std::vector<func::FuncOp> hardFuncList;
  for (auto func : moduleOp.getOps<mlir::func::FuncOp>()) {
    softFuncList.push_back(func);
  }
  for (auto submodule : moduleOp.getOps<ModuleOp>()) {
    for (auto func : submodule.getOps<mlir::func::FuncOp>()) {
      hardFuncList.push_back(func);
    }
  }

  llvm::outs() << "hardFuncList.size(): " << hardFuncList.size() << "\n";
  llvm::outs() << "softFuncList.size(): " << softFuncList.size() << "\n";

  // 获取函数列表函数名和PDG的映射
  std::map<std::string, PDG> hardPDGMap;
  std::map<std::string, PDG> softPDGMap;
  // std::vector<PDG> hardPDGList;
  // std::vector<PDG> softPDGList;
  for (auto hardFunc : hardFuncList) {
    llvm::outs() << "hardFunc: " << hardFunc.getName().str() << "\n";
    auto &pdg1 = getChildAnalysis<PDGGenerator>(hardFunc).constructPDG();
    // hardPDGList.push_back(pdg1);
    hardPDGMap[hardFunc.getName().str()] = pdg1;
  }
  for (auto softFunc : softFuncList) {
    llvm::outs() << "softFunc: " << softFunc.getName().str() << "\n";
    auto &pdg2 = getChildAnalysis<PDGGenerator>(softFunc).constructPDG();
    // softPDGList.push_back(pdg2);
    softPDGMap[softFunc.getName().str()] = pdg2;
  }
  int count = 0;

  std::ofstream outFile;
  if (!outputFilePath.empty()) {
    llvm::outs() << "outputFilePath: " << outputFilePath << "\n";
    // 确保父目录存在
    llvm::SmallString<256> parentDir(outputFilePath);
    llvm::sys::path::remove_filename(parentDir);
    if (!parentDir.empty()) {
      std::error_code dirEc =
          llvm::sys::fs::create_directories(parentDir, /*IgnoreExisting=*/true);
      if (dirEc) {
        llvm::errs() << "创建目录失败: " << parentDir
                     << ", error: " << dirEc.message() << "\n";
      }
    }
    outFile.open(outputFilePath, std::ios::out | std::ios::app);
    if (!outFile.is_open()) {
      llvm::errs() << "打开输出文件失败: " << outputFilePath
                   << "，将回退到标准输出\n";
    }
  } else {
    llvm::outs() << "outputFilePath is empty\n";
  }
  auto writeLine = [&](const std::string &s) {
    if (outFile.is_open()) {
      outFile << s << "\n";
    } else {
      llvm::outs() << s << "\n";
    }
  };
  for (auto &hardPDG : hardPDGMap) {
    for (auto &softPDG : softPDGMap) {
      hardPDG.second.dump("tmp/output/smallPDG.dot");
      softPDG.second.dump("tmp/output/largePDG.dot");
      auto &matchs = getAnalysis<PDGIsomorphismDetector>().findIsomorphism(
          hardPDG.second, softPDG.second, baseNameMappingTablePath);
      if (matchs.size() > 0) {
        count++;
        llvm::outs() << "hardPDG: " << hardPDG.first << "\n";
        llvm::outs() << "softPDG: " << softPDG.first << "\n";
        std::cout << "加速器匹配到的软件微码片段数目: " << matchs.size()
                  << "\n";
        for (size_t mi = 0; mi < matchs.size(); ++mi) {
          auto &match = matchs[mi];
          std::cout << "每个加速器匹配到的节点数目: " << match.size() << "\n";
          z3::context ctx;
          SMTConverter smtConverter = SMTConverter(ctx);
          std::vector<std::pair<std::string, std::string>> globalNamePairs;
          bool isEquivalent = smtConverter.verify(
              hardPDG.second, softPDG.second, match, globalNamePairs);

          if (isEquivalent) {
            std::cout << "匹配等价\n";
            std::cout << "✅ 验证通过：此匹配可以安全替换为硬件加速器调用\n";
            // 输出一组
            std::ostringstream header;
            header << "$ " << hardPDG.first << " ~ " << softPDG.first;
            writeLine(header.str());
            for (auto &p : globalNamePairs) {
              writeLine(p.first + " ~ " + p.second);
            }
            // 组尾添加一个空行作为分隔
            writeLine("");
            // 不再添加额外空行分隔
          } else {
            std::cout << "匹配不等价\n";
            std::cout << "❌ 验证失败：此匹配不能替换为硬件加速器调用\n";
          }
        }
      }
    }
  }
  llvm::outs() << "匹配个数: " << count << "\n";
  if (outFile.is_open()) {
    outFile.close();
    llvm::outs() << "SMT 验证完成 " << outputFilePath << "\n";
  }
}
// void TranslationValidationPass::runOnOperation() {
//   applayMemorySSA();
//   auto moduleOp = getOperation();
//   // 获取module里的module
//   // auto module = moduleOp.getOps<mlir::ModuleOp>();
//   // module.dump();

//   auto [smallFunc, largeFunc] = getCompairedFuncs();
//   if (!smallFunc || !largeFunc) {
//     llvm::errs() << "Error: Functions not found for comparison.\n";
//     return;
//   }

//   auto &pdg1 = getChildAnalysis<PDGGenerator>(smallFunc).constructPDG();
//   auto &pdg2 = getChildAnalysis<PDGGenerator>(largeFunc).constructPDG();
//   pdg1.dump("tmp/output/smallPDG.dot");
//   pdg2.dump("tmp/output/largePDG.dot");
//   auto &matchs =
//       getAnalysis<PDGIsomorphismDetector>().findIsomorphism(pdg1, pdg2);
//   llvm::outs() << "找到: " << matchs.size() << "潜在的匹配\n";
//   for (auto &match : matchs) {
//     llvm::outs() << "匹配规模:" << match.size() << "\n";
//     std::vector<Vertex> nodes1, nodes2;
//     PDG subpdg1, subpdg2;
//     getNodes(match, nodes1, nodes2);
//     pdg1.getSubPDG(nodes1, subpdg1, smallFunc);
//     pdg2.getSubPDG(nodes2, subpdg2, largeFunc);
//     subpdg1.dump("tmp/output/subpdg1.dot");
//     subpdg2.dump("tmp/output/subpdg2.dot");
//     z3::context ctx;
//     SMTConverter smtConverter = SMTConverter(ctx);
//     smtConverter.verify(subpdg1, subpdg2);
//   }
// }

void TranslationValidationPass::applayMemorySSA(Operation *root) {
  OpPassManager pm(ModuleOp::getOperationName());
  pm.addPass(createMemorySSAPass());
  if (failed(runPipeline(pm, root))) {
    this->signalPassFailure();
  }
}

auto TranslationValidationPass::getCompairedFuncs()
    -> std::tuple<func::FuncOp, func::FuncOp> {
  func::FuncOp smallfunc, largefunc;
  if (func1.empty() || func2.empty()) {
    llvm::errs() << "Error: small-pdg or large-pdg is not specified.\n";
    return std::make_tuple(smallfunc, largefunc);
  }
  auto moduleOp = getOperation();
  for (auto func : moduleOp.getOps<mlir::func::FuncOp>()) {
    if (func.getName().str() == func1) {
      smallfunc = func;
    }
    if (func.getName().str() == func2) {
      largefunc = func;
    }
  }
  if (!smallfunc || !smallfunc) {
    llvm::errs() << "Error: small-pdg or large-pdg not found in PDG list.\n";
  }
  return std::make_tuple(smallfunc, largefunc);
}

std::unique_ptr<OperationPass<ModuleOp>> createTranslationValidationPass() {
  return std::make_unique<TranslationValidationPass>();
}
} // namespace HMCM
