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

enum class ValueType {
  e_unused,
  e_imm,
  e_reg,
  e_stack,
};
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

// 数组初始化单个节点
struct ArrInit {
  enum { e_int, e_zero } ty;
  union {
    int value;
    int zerolen;
  } content;
  ArrInit(const int& val);
};

// 数组信息
struct ArrInfo {
  // 存储shape
  vector<int> shape;
  // 初始化数据，表示一个数或者一堆0的序列
  vector<ArrInit> init;
  // 当前已经初始化的长度
  int init_len;
  // 对于临时数组，存储相对于sp的地址
  int stack_addr;

  ArrInfo();
  ArrInfo(const vector<int>& _shape);
  // 初始化时推入一个数
  void PushNum(const int& val);
  // 获取大小
  const int GetSize();
  // 给定维度i，获取从i到max的维度的总空间
  // 如：int a[2][3][5], (i) = 3*5*4, 也就是第一次getelemptr需要的步长
  const int GetSize(int i);
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

  // 用于数据转移，需要在外部管理寄存器
  // 支持：
  // imm -> reg, imm -> stack
  // reg -> stack, stack1 -> stack2
  // stack -> reg
  void WriteDataTranfer(const InstResultInfo& src, const InstResultInfo& dest);

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

enum class InitType { e_zeroinit, e_int };

struct InitInfo {
  InitType ty;
  int value;
};

class GlobalVarModule {
 public:
  GlobalVarModule();
  // 生成全局变量声明
  void WriteGlobalVarDecl(const string& name, const InitInfo& init);
  // 从全局变量load，返回最终存储这个load指令结果的地址
  const int WriteLoadGlobalVar(const string& name);
  // 存储到全局变量，返回全局变量的地址，info只支持int和reg
  void WriteStoreGlobalVar(const string& name, const InstResultInfo& src_info);

  // 生成全局数组声明
  void WriteGlobalArrDecl(const string& name, const ArrInfo& init);
};

// 记录alloc指令和数组关系的表
class ArrInfoModule {
 public:
  map<koopa_raw_value_t, ArrInfo> arrinfos;

  // 当前正在分析的数组，会在第一次调用从alloc中getelemptr时赋值
  ArrInfo current_arr;

  // 当前解析到的维度数，算指针用
  int current_dim;

  ArrInfoModule();

  // 用current dim计算current arr的分片size
  const int GetCurArrSize();
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
  GlobalVarModule globalCore;
  ArrInfoModule arrCore;
  static RiscvGenerator& getInstance();

  // 输入运算符，输出指令
  void WriteBinaInst(OpType op, const Reg& left, const Reg& right);
};
};  // namespace riscv
