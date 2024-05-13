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

enum NodeTag { reg, imm };
typedef koopa_raw_binary_op_t OpType;

class BaseModule {
 private:
  bool active;

 public:
  BaseModule();
  void Activate();
  void Deactivate();
  bool IsActive();
};

// 寄存器模块
class RegisterModule : public BaseModule {
 private:
  // 当前可用寄存器
  set<Reg> availableRegs;

 public:
  RegisterModule();
  // 取出一个当前可用的寄存器
  Reg getAvailableReg();
  // 释放一个占用寄存器
  void releaseReg(Reg reg);
};

// 栈内存管理模块
class StackMemoryModule : public BaseModule {
 private:
  int stackMemoryNeeded;

 public:
  enum ValueType { imm, reg, stack };
  struct InstResultInfo {
    ValueType ty;
    union {
      int imm;
      int addr;
      Reg reg;
    } content;
    void Output();
  };
  struct StoreInfo {
    const InstResultInfo& dest;
    const InstResultInfo& src;
    StoreInfo(const InstResultInfo& dstInfo, const InstResultInfo& srcInfo);
  };
  map<koopa_raw_value_t, InstResultInfo> InstResult;
  StackMemoryModule();
  const int& GetStackMem();
  void SetStackMem(const int& mem);
  void WriteStoreInst(const StoreInfo& info);
  void WriteLI(const Reg& rs, int imm);
  void WriteLW(const Reg& rs, const Reg& rd, int addr);
  void Debug_OutputInstResult();
  // 返回应该用的addr
  int IncreaseStackUsed();

 private:
  int StackUsed;
};

class RiscvGenerator {
 private:
  RiscvGenerator();
  RiscvGenerator(const RiscvGenerator&) = delete;
  RiscvGenerator(const RiscvGenerator&&) = delete;
  RiscvGenerator& operator=(const RiscvGenerator&) = delete;

 public:
  string FunctionName;
  GenSettings Setting;
  RegisterModule RegManager;
  StackMemoryModule StackMemManager;
  static RiscvGenerator& getInstance();

  // 生成函数开头
  void WritePrologue();
  // 生成函数屁股
  void WriteEpilogue(const StackMemoryModule::InstResultInfo& retValueInfo);
  // 输入运算符，输出指令
  void WriteBinaInst(OpType op, const Reg& left, const Reg& right);
};
};  // namespace riscv
