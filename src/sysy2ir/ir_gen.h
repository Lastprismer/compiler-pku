#pragma once

#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include "ir_symbol.h"
#include "ir_util.h"
#include "output_setting.h"

namespace ir {
// 栈内元素类型

// Tagged Enums
struct RetInfo {
  enum RetTy { ty_void, ty_int, ty_sbl } ty;
  int value;
  string name;
  RetInfo();
  RetInfo(int _value);
  RetInfo(string _symbol);
  RetInfo(int _, string _var);
  const int& GetValue() const;
  const string& GetSym() const;
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
  GenSettings setting;
  SymbolManager sbmanager;
  RetInfo functionRetInfo;

  // 生成函数开头
  void WriteFuncPrologue();
  // 生成函数屁股
  void WriteFuncEpilogue();
  // 生成块开头
  void WriteBlockPrologue();

#pragma region lv3

  // 输入单目运算符，输出指令
  const RetInfo WriteUnaryInst(const RetInfo& left, OpID op);
  // 输入双目运算符，输出指令
  const RetInfo WriteBinaryInst(const RetInfo& left,
                                const RetInfo& right,
                                OpID op);
  // 输入逻辑运算符and or，输出指令
  const RetInfo WriteLogicInst(const RetInfo& left,
                               const RetInfo& right,
                               OpID op);

#pragma endregion

#pragma region lv4
  // 输出声明变量的指令
  void WriteAllocInst(const SymbolTableEntry& entry);
  // 输出加载变量的指令，加载得到的符号节点将推入栈顶
  const RetInfo WriteLoadInst(const SymbolTableEntry& entry);
  // 输出写入变量的指令，栈顶的节点将用于写入
  void WriteStoreInst(const RetInfo& value, const SymbolTableEntry& entry);

#pragma endregion

 private:
  int registerNewSymbol();
  string getSymbolName(const int& symbol) const;
  // 解析RetInfo
  const string parseRetInfo(const RetInfo& info) const;
  // 计算常数表达式
  int calcConstExpr(const int& left, const int& right, OpID op);
};

}  // namespace ir