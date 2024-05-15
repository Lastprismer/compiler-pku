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
  enum retty_t { ty_void, ty_int, ty_sbl } ty;
  int value;
  string name;
  RetInfo();
  RetInfo(int value);
  RetInfo(string symbol);
  const int& GetValue() const;
  const string& GetSym() const;
  const string GetInfo() const;
};

// if可能的类型：单if或if-else
struct IfInfo {
  enum ifty_t { i, ie } ty;
  int then_label;
  int else_label;
  int next_label;
  IfInfo();
  IfInfo(ifty_t ty);
  IfInfo(int then, int next);
  IfInfo(int then, int _else, int next);
};

class IRGenerator {
 private:
  IRGenerator();
  IRGenerator(const IRGenerator&) = delete;
  IRGenerator(const IRGenerator&&) = delete;
  IRGenerator& operator=(const IRGenerator&) = delete;

 public:
  static IRGenerator& getInstance();
  string funcName;
  string returnType;
  GenSettings setting;
  SymbolManager symbolman;
  RetInfo funcRetInfo;

  // 生成函数开头
  void WriteFuncPrologue();
  // 生成函数屁股
  void WriteFuncEpilogue();
  // 生成返回指令
  void WriteRetInst();

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

#pragma region lv6

  // 本块中已经返回过
  bool hasRetThisBB;
  // 生成块开头（仅标签）
  void WriteBasicBlockPrologue(const int& bb_id);
  // 生成if判断指令（br），label存在Ifinfo中
  void WriteBrInst(const RetInfo& cond, IfInfo& info);
  // 生成无条件跳转
  void WriteJumpInst(const int& labelID);
  void WriteJumpInst(const string& labelName);
  // 生成标签（%label_n: ）并刷新块返回状态，进入新块
  void WriteLabel(const int& labelID);
  void WriteLabel(const string& labelName);
  // 生成一个用于短路的变量名
  const string registerShortCircuitVar();
#pragma endregion

 private:
  // 临时符号ID
  int symbolPool;
  int bbPool;
  const int registerNewSymbol();
  const int registerNewBB();
  const int registerNewVar();
  const string getSymbolName(const int& symbol) const;
  const string getLabelName(const int& bb_id) const;
  // 计算常数表达式
  int calcConstExpr(const int& left, const int& right, OpID op);
};

}  // namespace ir