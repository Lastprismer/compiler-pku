#pragma once

#include <cassert>
#include <cstring>
#include <queue>
#include <sstream>
#include <string>

using namespace std;

class CompInfo {
 public:
  virtual ~CompInfo() = default;
};

class FuncInfo : public CompInfo {
 public:
  // 栈内元素类型
  typedef enum unary_exp_ssa_component_t { symbol, imm } uexpc_t;
  // 栈内元素
  typedef struct {
    uexpc_t tag;
    union {
      int symbol_id;
      int imm;
    } content;
  } comp_t;

  static int var_cnt;
  int cur_indent;
  string func_name;
  string ret_type;
  queue<comp_t> node_stack;

  FuncInfo();
  // 生成函数开头
  void write_prologue(ostream& os);
  // 生成函数屁股
  void write_epilogue(ostream& os);
  // 推入符号
  void push_symbol(int symbol);
  // 推入立即数
  void push_imm(int int_const);
  // 输入单目运算符，输出指令
  void write_inst(ostream& os, char op);

 private:
  int create_temp_symbol();
  inline string ind_sp();
};