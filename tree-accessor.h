#ifndef TREE_ACCESSOR_H
#define TREE_ACCESSOR_H

#include "tree-vector.h"
#include "HuaweiAST/HuaweiAST.h"
#include <iomanip>
#include <iostream>
#include <list>
#include <filesystem> // Added for filesystem operations
#include <sstream>
namespace fs = std::filesystem;
using namespace HMCM;
struct CloneDetectionParams
{
  int minSubtreeSize = 4;
  int maxSubtreeSize = 5000;
  int minMergeUnitSize = 70;
  float importantTokenWeight = 1.5f;
};
class TreeAccessor
{
public:
  static CloneDetectionParams params;
  static const size_t size = static_cast<size_t>(ASTNodeCategory::Other) + 1;
  static BaseAST *get_serialized_next_neighbor(BaseAST *node)
  {
    auto attr_itr =
        node->attributes.find(NodeAttributeName_t::NODE_SERIALIZED_NEIGHBOR);
    if (attr_itr != node->attributes.end() && attr_itr->second.has_value())
    {
      // 安全地检查并转换为 std::pair<BaseAST*, BaseAST*>* 类型
      if (auto pairPtr = std::any_cast<std::pair<BaseAST *, BaseAST *> *>(
              &attr_itr->second))
      {
        return (*pairPtr)->second; // 返回 pair 的第二个元素 (BaseAST*)
      }
    }
    return nullptr; // 如果没有找到或类型不匹配，返回 nullptr
  };

  static TreeVector *get_node_vector(BaseAST *t)
  {
    auto attr_itr = t->attributes.find(NodeAttributeName_t::NODE_VECTOR);
    if (attr_itr != t->attributes.end() && attr_itr->second.has_value())
    {
      // 安全检查并转换为 TreeVector* 类型
      try
      {
        return std::any_cast<TreeVector *>(attr_itr->second);
      }
      catch (const std::bad_any_cast &)
      {
        // 处理类型不匹配的情况
        std::cerr << "错误：属性 NODE_VECTOR 不包含 TreeVector* 类型!"
                  << std::endl;
        return nullptr; // 或抛出异常
      }
    }
    return nullptr; // 如果没有找到属性或类型不匹配，返回 nullptr
  }

  // 打印context的ast树

  static void printAST(BaseAST *root, std::ostream &os = std::cout)
  {
    if (root == nullptr)
    {
      return;
    }
    // root->print(os);
    TreeAccessor::print(root, os);
    if (root->getDown() != nullptr)
    {
      printAST(root->getDown(), os);
    }
    if (root->getRight() != nullptr)
    {
      printAST(root->getRight(), os);
    }
  }
  static void printASTAdjustIdent(BaseAST *root, int diff=0,std::ostream &os = std::cout)
  {
    if (root == nullptr)
    {
      return;
    }
    // root->print(os);
    TreeAccessor::printAdjustIdent(root, diff, os);
    if (root->getDown() != nullptr)
    {
      printASTAdjustIdent(root->getDown(), diff, os);
    }
    if (root->getRight() != nullptr)
    {
      printASTAdjustIdent(root->getRight(), diff, os);
    }
  }
  // 打印孩子，不打印兄弟
  static void printSubTree(BaseAST *root, std::ostream &os = std::cout)
  {
    if (root == nullptr)
    {
      return;
    }
    
    // root->print(os);
    TreeAccessor::print(root, os);
    if (root->getDown() != nullptr)
    {
      printAST(root->getDown(), os);
    }
  }
  static void printSubTreeAdjustIdent(BaseAST *root, std::ostream &os = std::cout)
  {
    if (root == nullptr)
    {
      return;
    }
    
    // root->print(os);
    int diff=root->getIndent();
    TreeAccessor::printAdjustIdent(root, diff, os);
    if (root->getDown() != nullptr)
    {
      printASTAdjustIdent(root->getDown(), diff, os);
    }
  }
  // 打印一个ast容器的所有ast
  static void printASTs(const std::vector<BaseAST *> &roots,
                        std::ostream &os = std::cout)
  {
    for (auto root : roots)
    {
      // printSubTree(root,os);
      printSubTree(root, os);
    }
    os << std::endl;
    // os << std::endl;
  }
  static void printASTFromRootPre(BaseAST *root)
  {
    if (root == nullptr)
    {
      return;
    }
    std::cout << root->getText() << " ";
    if (root->getNumChildren() > 0)
    {
      std::cout << "numChildren: " << root->getNumChildren() << std::endl;
    }
    if (root->getDown() != nullptr)
    {
      printASTFromRootPre(root->getDown());
    }
    if (root->getRight() != nullptr)
    {
      printASTFromRootPre(root->getRight());
    }
  }
  static void callPrintASTFromRootPre(BaseAST *root)
  {

    printASTFromRootPre(root);
    std::cout << std::endl;
  }
  static void printAST(HMCM::HuaweiContext *context)
  {
    for (auto root : context->getRootLists())
    {
      std::cout << "******打印根结点为： " << root->getText()
                << "的ast树  *********" << std::endl;
      callPrintASTFromRootPre(root);
    }
  }

  // 打印vector<vector<int>>
  static void printVecs(std::vector<std::vector<int>> &vec)
  {
    std::cout << "*******************printVecs*******************" << std::endl;
    for (auto v : vec)
    {
      for (auto i : v)
      {
        std::cout << i << " ";
      }
      std::cout << std::endl;
    }
  }
  // 打印一个向量，传入一个vector<int>
  static void printVecFromVec(std::vector<int> &vec)
  {
    std::cout << "******************* Token Vector *******************"
              << std::endl;

    int max_len = strlen(to_string(ASTNodeCategory::ArithmeticAddSub));
    int padding = 4;
    // 第一行：category names（只显示非零项）
    for (int i = 0; i < size && i < (int)vec.size(); ++i)
    {
      if (vec[i] == 0)
        continue;
      ASTNodeCategory category = static_cast<ASTNodeCategory>(i);
      std::cout.width(max_len + padding); // 设置宽度为 max_len+padding 字符
      std::cout << std::left << to_string(category);
    }
    std::cout << std::endl;

    // 第二行：counter values（只显示非零项）
    for (int i = 0; i < size && i < (int)vec.size(); ++i)
    {
      if (vec[i] == 0)
        continue;
      std::cout.width(max_len + padding);
      std::cout << std::left << vec[i];
    }
    std::cout << std::endl;
  }
  // 原baseast的print，加一个os
  static void print(BaseAST *root, std::ostream &os = std::cout)
  {
    // 输出 indent 个空格
    // os<<"ident = "<< root->getIndent() << std::endl;
    for (int i = 0; i < root->getIndent(); i++)
    {
      os << " ";
    }
    if (root->getBB() == -1)
    {
      os << "-> " << root->getText() << " ("
         << HuaweiContext::getTokenName(root->getType()) << ") ("
         << root->getLoc() << ")" << std::endl;
    }
    else
    {
      os << "-> " << root->getText() << " ("
         << HuaweiContext::getTokenName(root->getType())
         << ")  (BB:" << root->getBB() << ") (" << root->getLoc() << ")"
         << std::endl;
    }
  }

  static void printAdjustIdent(BaseAST *root, int diff=0,std::ostream &os = std::cout)
  {
    // 输出 indent 个空格
    // os<<"ident = "<< root->getIndent() << std::endl;
    int ident=root->getIndent()-diff;
    root->setIndent(ident>0?ident:0);
    for (int i = 0; i < root->getIndent(); i++)
    {
      os << " ";
    }
    if (root->getBB() == -1)
    {
      os << "-> " << root->getText() << " ("
         << HuaweiContext::getTokenName(root->getType()) << ") ("
         << root->getLoc() << ")" << std::endl;
    }
    else
    {
      os << "-> " << root->getText() << " ("
         << HuaweiContext::getTokenName(root->getType())
         << ")  (BB:" << root->getBB() << ") (" << root->getLoc() << ")"
         << std::endl;
    }
  }
  // 打印baseast的vector,传入一个ast结点
  static void printVec(BaseAST *node)
  {
    // 找vector
    auto attr_itr = node->attributes.find(NodeAttributeName_t::NODE_VECTOR);
    if (attr_itr == node->attributes.end()) {
      std::cout << "Node has no vector attribute" << std::endl;
      return;
    }

    TreeVector *vec = std::any_cast<TreeVector *>(attr_itr->second);
    if (!vec) {
      std::cout << "Vector is null" << std::endl;
      return;
    }

    printVec(vec);
  }

// 打印TreeVector的重构版本，直接传入TreeVector指针
  static void printVec(TreeVector *vec)
  {
    if (!vec) {
      std::cout << "Vector is null" << std::endl;
      return;
    }

    int max_len = strlen(to_string(ASTNodeCategory::ArithmeticAddSub));
    int padding = 1;
    const std::vector<int> &counters = vec->counters;

    std::cout << "******************* Token Vector *******************"
              << std::endl;

    // 第一行：token names
    for (int i = 0; i < size; ++i)
    {
      // if (counters[i] > 0 && TOKEN_NAMES[i] != nullptr) {
      //     std::cout.width(8);  // 设置宽度为 8 字符
      //     std::cout << std::left << TOKEN_NAMES[i];
      // }
      ASTNodeCategory category = static_cast<ASTNodeCategory>(i);
      std::cout << std::setw(max_len + padding) << std::left
                << to_string(category);
    }
    std::cout << std::endl;

    // 第二行：counter values
    for (int i = 0; i < size; ++i)
    {
      if (counters[i] >= 0)
      {
        std::cout.width(max_len + padding); // 设置宽度为 8 字符
        std::cout << std::left << counters[i];
      }
    }
    std::cout << std::endl;
  }

  static void printWithIdent(BaseAST *root,int diff,std::ostream &os){
    if(root==nullptr){
      return;
    }
    root->setIndent(root->getIndent()-diff);
    print(root,os);
    printWithIdent(root->getDown(),diff,os);
    printWithIdent(root->getRight(),diff,os);
  }
  static void printSubTreeWithIdent(BaseAST *root,int ident,std::ostream &os){
    if(root==nullptr){
      return;
    }
    int diff=root->getIndent()-ident;
    root->setIndent(ident);
    print(root,os);
    printWithIdent(root->getDown(),diff,os);
    //printWithIdent(root->getRight(),diff,os);

  }

static void printBundleTree(BaseAST *root,int ident,std::ostream &os){
  std::cout << "DEBUG: printBundleTree called with root=" << (root ? root->getText() : "nullptr") << std::endl;
  if(root==nullptr){
    return;
  }
  for(int i=0;i<root->getIndent()-ident;i++){
    os<<" ";
  }
  if(root->getBB() == -1)
  {
    os << "-> " << root->getText() << " ("
       << HuaweiContext::getTokenName(root->getType()) << ") ("
       << root->getLoc() << ")" << std::endl;
  }
  else{
    os << "-> " << root->getText() << " ("
       << HuaweiContext::getTokenName(root->getType())
       << ")  (BB:" << root->getBB() << ") (" << root->getLoc() << ")"
       << std::endl;
  }
  printBundleTree(root->getDown(),ident,os);
  printBundleTree(root->getRight(),ident,os);
}
  static void printBundleParam(BaseAST *root,std::ostream &os){
    if (root == nullptr) {
      return;
    }
    int ident=root->getIndent();
    printBundleTree(root->getDown(),ident,os);
  }
  //找funccall和buddle
  static void printFunccall(BaseAST *root,std::ostream &os=std::cout){
    std::string funccallName=root->getDown()->getText();
    os << "-> ; (FUNCTION_PROTO) (0, 0, 635, 635, 33)" << std::endl;
    printBundleParam(root,os);
    os << "  -> extern (\"extern\") (0, 0, 635, 635, 1)" << std::endl;
    os << "  -> void (\"void\")  (109, 431, 45, 41, 1)" << std::endl;
  }
  static void printBundle(BaseAST *root,std::ostream &os=std::cout){
    std::string bundleName=root->getDown()->getText();
    std::cout << "DEBUG: printBundle called with bundleName=" << bundleName << std::endl;
    //打印子树，不打印兄弟，并且调整缩进至顶级的缩进为2
    os << "-> ; (BUNDLE_PROTO) (0, 0, 635, 635, 33)" << std::endl;
    printBundleParam(root,os);
    os << "  -> extern (\"extern\") (0, 0, 635, 635, 1)" << std::endl;
  }

  static void findFunccallAndBundle(BaseAST *root,std::ostream &os){
    if (root == nullptr) {
      return;
    }
    if (root->getType() == HMCM::AST_NODE_TYPE::AST_FUNC_CALL) {
      std::cout << "DEBUG: Calling printFunccall" << std::endl;
      printFunccall(root,os);
    }else if(root->getType() == HMCM::AST_NODE_TYPE::AST_jmp){
      std::cout << "DEBUG: Calling printBundle" << std::endl;
      //打印子树，不打印兄弟
      printBundle(root,os);
    }
    findFunccallAndBundle(root->getDown(),os);
    findFunccallAndBundle(root->getRight(),os);
  }



  // categoryto string
  static const char *to_string(ASTNodeCategory category)
  {
    switch (category)
    {
    case ASTNodeCategory::Irrelevant:
      return "Irrelevant";
    case ASTNodeCategory::Identifier:
      return "Identifier";
    case ASTNodeCategory::ControlFlow:
      return "ControlFlow";
    case ASTNodeCategory::ArithmeticAddSub:
      return "ArithmeticAddSub";
    case ASTNodeCategory::ArithmeticMulDiv:
      return "ArithmeticMulDiv";
    case ASTNodeCategory::LogicalOperation:
      return "LogicalOperation";
    case ASTNodeCategory::Bitwise:
      return "Bitwise";
    case ASTNodeCategory::Comparison:
      return "Comparison";
    case ASTNodeCategory::Registers:
      return "Registers";
    case ASTNodeCategory::Assignment:
      return "Assignment";
    case ASTNodeCategory::Function:
      return "Function";
    case ASTNodeCategory::DataStructure:
      return "DataStructure";
    case ASTNodeCategory::Declaration:
      return "Declaration";
    case ASTNodeCategory::Condition:
      return "Condition";
    case ASTNodeCategory::Other:
      return "Other";
    default:
      return "Unknown";
    }
  }

  // 重新定义子树结点
  static bool isCompleteSemanticUnit(BaseAST *node)
  {
    AST_NODE_TYPE type = node->getType();

    // 可独立转换的节点类型
    static const std::set<AST_NODE_TYPE> completeUnits = {
        AST_NODE_TYPE::AST_FUNCTION_LABEL, // 函数
        AST_NODE_TYPE::AST_if,             // if
        AST_NODE_TYPE::AST_else,           // else
        // AST_NODE_TYPE::AST_STMT_BLOCK,         // stmt
        // AST_NODE_TYPE::AST_typedef, // typedef
        AST_NODE_TYPE::AST_switch, // switch
        AST_NODE_TYPE::AST_bitcmp, AST_NODE_TYPE::AST_mskcmp,
        AST_NODE_TYPE::AST_cmp,
        // AST_NODE_TYPE::AST_move, AST_NODE_TYPE::AST_sub,
        // AST_NODE_TYPE::AST_add, AST_NODE_TYPE::AST_xor, AST_NODE_TYPE::AST_case,
        // AST_NODE_TYPE::AST_movezr, AST_NODE_TYPE::AST_sll,

        // 剩余的可以补充
    };

    // 验证子树完整性
    return completeUnits.count(type);
  }
  // 打印链表信息
};

#endif // TREE_ACCESSOR_H