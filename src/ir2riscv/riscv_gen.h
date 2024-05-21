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

namespace riscv {

typedef koopa_raw_binary_op_t OpType;

enum class ValueType { e_unused, e_imm, e_reg, e_stack };
// 指令结果信息
struct InstResultInfo {
  ValueType ty;
  union {
    int imm;
    int addr;
    Reg reg;
  } content;
  const string Output() const;
  InstResultInfo();
  InstResultInfo(const Reg& reg);
  InstResultInfo(const ValueType& ty, int value);
};
// 寄存器模块
class RegisterModule {
 private:
  // 当前可用寄存器
  set<Reg> available_regs;

 public:
  RegisterModule();
  // 取出一个当前可用的寄存器
  Reg GetAvailableReg();
  // 释放一个占用寄存器
  void ReleaseReg(Reg reg);
  // 取出特定寄存器
  bool GetReg(const Reg& reg);
};

// 栈内存管理模块
class StackMemoryModule {
 private:
 public:
  // 占用的栈空间
  int stack_memory;
  // 当前使用的栈空间
  int stack_used;

  map<koopa_raw_value_t, InstResultInfo> InstResult;

  StackMemoryModule();

  void SetStackMem(const int& mem);

  void WriteStoreInst(const InstResultInfo& src, const InstResultInfo& dest);

  void WriteLI(const Reg& rs, int imm);
  // 从addr地址读出存入rd，不用imm12
  void WriteLW(const Reg& rd, int addr);
  // 从rs写入addr地址，不用imm12
  void WriteSW(const Reg& rs, int addr);

  void Debug_OutputInstResult();

  // 多分配4byte，返回应该用的addr
  int IncreaseStackUsed();

  // 消除临时分配的内存（比如函数调用后保存的ra只用一次）
  int DecreaseStackUsed();
  // 清空记录
  void Clear();
};

class BBModule {
 private:
 public:
  BBModule();
  void WriteBBName(const string& label);

  void WriteJumpInst(const string& label);

  void WriteBranch(const Reg& cond,
                   const string& trueLabel,
                   const string& falseLabel);
};

class FuncModule {
 public:
  // 不再调用其他函数
  bool is_leaf_func;
  // 函数名
  string func_name;

  FuncModule();

  // 生成函数开头
  void WritePrologue();
  // 生成函数屁股
  void WriteEpilogue(const InstResultInfo& retValueInfo);
  // 输出call指令
  void WriteCallInst(const string& name);
  // 清空信息
  void Clear();
};

class RiscvGenerator {
 private:
  RiscvGenerator();
  RiscvGenerator(const RiscvGenerator&) = delete;
  RiscvGenerator(const RiscvGenerator&&) = delete;
  RiscvGenerator& operator=(const RiscvGenerator&) = delete;

 public:
  GenSettings setting;
  RegisterModule regCore;
  StackMemoryModule stackCore;
  BBModule bbCore;
  FuncModule funcCore;
  static RiscvGenerator& getInstance();

  // 输入运算符，输出指令
  void WriteBinaInst(OpType op, const Reg& left, const Reg& right);
  // 清除函数相关记录
  void Clear();
};
};  // namespace riscv
