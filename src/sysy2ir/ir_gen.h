#pragma once

#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include "ir_symtable.h"
#include "ir_util.h"
#include "output_setting.h"

namespace ir {
// 栈内元素类型
enum class NodeTag { UNUSED, SYMBOL, IMM, VAR };

// 栈内元素
struct Node {
  NodeTag tag;
  union {
    int symbol_id;
    int imm;
  } content;
  Node();
  Node(int i);
  Node(const Node& n);
  Node(Node&& n);
};

class IRGenerator {
 private:
  IRGenerator();
  IRGenerator(const IRGenerator&) = delete;
  IRGenerator(const IRGenerator&&) = delete;
  IRGenerator& operator=(const IRGenerator&) = delete;

 public:
  static IRGenerator& getInstance();
  int variable_pool;
  string function_name;
  string return_type;
  deque<Node> node_stack;
  GenSettings setting;
  SymbolTable symbol_table;

  // 生成函数开头
  void writeFuncPrologue();
  // 生成函数屁股
  void writeFuncEpilogue();
  // 生成块开头
  void writeBlockPrologue();
  // 推入符号
  void pushSymbol(int symbol);
  // 推入立即数
  void pushImm(int int_const);
  // 获取栈顶节点
  const Node& checkFrontNode() const;
  // 弹出栈顶节点
  Node getFrontNode();
  // 输入单目运算符，输出指令
  void writeUnaryInst(OpID op);
  // 输入双目运算符，输出指令
  void writeBinaryInst(OpID op);
  // 输入逻辑运算符and or，输出指令
  void writeLogicInst(OpID op);

 private:
  int registerNewSymbol();
  void parseNode(const Node& node);
  // 计算常数表达式
  int calcConstExpr(const Node& left, const Node& right, OpID op);
};

}  // namespace ir