#pragma once

#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include "util.h"

using namespace std;

class CodeCompInfo {
 public:
  virtual ~CodeCompInfo() = default;
};

class CodeFuncInfo : public CodeCompInfo {
 public:
  // 栈内元素类型
  typedef enum unary_exp_ssa_component_t { symbol, imm, unused } NodeTag;
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

  static int var_cnt;
  int cur_indent;
  string func_name;
  string ret_type;
  deque<Node> node_stack;

  CodeFuncInfo();
  // 生成函数开头
  void write_prologue(ostream& os);
  // 生成函数屁股
  void write_epilogue(ostream& os);
  // 推入符号
  void push_symbol(int symbol);
  // 推入立即数
  void push_imm(int int_const);
  // 输入单目运算符，输出指令
  void write_unary_inst(ostream& os, OpID op);
  // 输入双目运算符，输出指令
  void write_binary_inst(ostream& os, OpID op);
  // 输入逻辑运算符and or，输出指令
  void write_logic_inst(ostream& os, OpID op);

 private:
  int create_temp_symbol();
  inline string ind_sp();
  void parse_node(ostream& os, const Node& node);
};