#pragma once

#include <cassert>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include "koopa.h"
#include "output_setting.h"
#include "riscv_build.h"
#include "riscv_util.h"

using namespace std;

namespace riscv {

enum NodeTag { reg, imm };
typedef koopa_raw_binary_op_t OpType;

struct Node {
  NodeTag tag;
  union {
    Reg reg;
    int imm;
  } content;
  Node();
  Node(int i);
  Node(Reg reg);
  Node(const Node& n);
  Node(Node&& n);
};

class RiscvGenerator {
 private:
  RiscvGenerator();
  RiscvGenerator(const RiscvGenerator&) = delete;
  RiscvGenerator(const RiscvGenerator&&) = delete;
  RiscvGenerator& operator=(const RiscvGenerator&) = delete;

 public:
  string function_name;
  deque<Node> node_stack;
  // 当前可用寄存器
  set<Reg> available_regs;
  GenSettings setting;
  static RiscvGenerator& getInstance();

  // 生成函数开头
  void writePrologue();
  // 生成函数屁股
  void writeEpilogue();
  // 推入寄存器
  void pushReg(Reg reg);
  // 推入立即数
  void pushImm(int imm);
  // 输入运算符，输出指令
  void writeInst(OpType op);

 private:
  // 取出一个当前可用的寄存器
  Reg getAvailableReg();
  // 释放一个占用寄存器
  void releaseReg(Reg reg);
  // 生成指令，内部，返回rd寄存器
  Reg genInst(Node& left, Node& right, OpType op);
  // 将为0的立即数变为寄存器x0
  void immZero2Regx0(Node& node);
};
};  // namespace riscv
