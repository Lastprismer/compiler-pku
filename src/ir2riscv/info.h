#pragma once

#include <cassert>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include "build.h"
#include "koopa.h"
#include "util.h"

using namespace std;
using namespace build;

class IRFuncInfo {
 public:
  typedef enum binary_elem_t { reg, imm } nodetype_t;
  typedef koopa_raw_binary_op_t biop_t;
  typedef struct {
    nodetype_t tag;
    union {
      reg_t reg;
      int imm;
    } content;
  } node_t;
  // 当前只有return后面的表达式是实际需要写入的
  bool start_writing;
  string func_name;
  deque<node_t> node_stack;
  // 当前可用寄存器
  set<reg_t> aval_regs;

  IRFuncInfo();
  // 生成函数开头
  void write_prologue(ostream& os);
  // 生成函数屁股
  void write_epilogue(ostream& os);
  // 推入寄存器
  void push_reg(reg_t reg);
  // 推入立即数
  void push_imm(int imm);
  // 输入运算符，输出指令
  void write_inst(ostream& os, biop_t op);

 private:
  // 取出一个当前可用的寄存器
  reg_t get_aval_reg();
  // 释放一个占用寄存器
  void release_reg(reg_t reg);
  // 生成指令，内部，返回rd寄存器
  reg_t gen_inst(ostream& os, node_t& left, node_t& right, biop_t op);
  // 将为0的立即数变为寄存器x0
  void imm_zero2rig_x0(node_t& node);
};