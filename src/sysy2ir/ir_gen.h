#pragma once

#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "ir_util.h"
#include "output_setting.h"

using std::vector, std::map;

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

// while相关信息
struct LoopInfo {
  int cond_label;
  int body_label;
  int next_label;
  LoopInfo();
};

#pragma region symbol

class SymbolManager;
struct SymbolTableEntry {
  // 常数或变量
  SymbolType symbol_type;
  // 变量类型
  VarType var_type;
  // 懒得union
  string var_name;
  int const_value;
  ArrInfo arr_info;
  int id;

  SymbolTableEntry();
  // @ + name
  const string GetAllocName() const;
  // 声明的指令
  const string GetAllocInst() const;
  // 加载的指令
  const string GetLoadInst(const string& loadToSymbolName) const;
  // 赋值的指令
  const string GetStoreInst(const string& storeFromSymbolName) const;
  const string GetStoreInst(const int& imm) const;
};

class BaseProcessor {
 private:
  bool _enable;

 public:
  BaseProcessor();
  void Enable();
  void Disable();
  const bool& IsEnabled();
};

// 变量声明处理器
class DeclaimProcessor : public BaseProcessor {
 private:
  // 保证只在处理声明的语句时启用
  SymbolType current_symbol_type;
  VarType current_var_type;
  int var_pool;

 public:
  // 当前声明的变量是不是全局变量
  bool global;

  DeclaimProcessor();
  void SetSymbolType(const SymbolType& type);
  void SetVarType(const VarType& type);
  // 重置，在函数调用完后（重置变量池）
  void Reset();

  // 生成常数变量表项
  SymbolTableEntry GenerateConstEntry(const string& varName, const int& value);

  // 生成数组变量表项
  // 常数数组也调用：表项中不存储常数数组初始化信息，因为没必要
  SymbolTableEntry GenerateArrEntry(const string& var_name,
                                    const ArrInfo& info);

  // 生成无初始化的变量表项
  SymbolTableEntry GenerateVarEntry(const string& varName,
                                    const VarType& var_ty);

  // 即时生成表项，用于短路运算临时变量，不插入符号表
  SymbolTableEntry QuickGenEntry(const SymbolType& st,
                                 const VarType& vt,
                                 string name);

  // 获取当前正在初始化的变量的类型（逻辑运算的编译时常数用）
  const SymbolType getCurSymType() const;

  // 获取当前正在初始化的变量的类型（逻辑运算的编译时常数用）
  const VarType getCurVarType() const;
  const int RegisterVar();
};

// 变量赋值处理器
class AssignmentProcessor : public BaseProcessor {
 public:
  AssignmentProcessor();
  // 保证在有值的时候才会用到
  SymbolTableEntry current_var;
  // 对数组变量赋值时的地址信息
  RetInfo arr_addr;

  // 对存储变量赋值
  void WriteAssign(const RetInfo& value) const;
};

class SymbolTable {
 public:
  // 表
  map<string, SymbolTableEntry> table;
  SymbolTable();
  // utility
  bool TryGetEntry(string _symbol_name, SymbolTableEntry& out) const;
  void InsertEntry(const SymbolTableEntry& entry);
  void ClearTable();

  // lv5
  // 到父表的指针
  // 不需要存子表
  SymbolTable* parent;
};

class SymbolManager {
 public:
  // 变量声明处理器
  DeclaimProcessor dproc;
  // 变量赋值处理器
  AssignmentProcessor aproc;

  // lv5
  // 根表
  SymbolTable RootTable;
  // 当前的表
  SymbolTable* currentTable;

  SymbolManager();
  // 递归从当前的表向根表查询
  const SymbolTableEntry getEntry(string symbolName);
  // 向当前的表插入
  void InsertEntry(SymbolTableEntry entry);
  // 推入一个新表
  void PushScope();
  // 弹出一个表
  void PopScope();
};

#pragma endregion

#pragma region Branch

class BranchManager {
 public:
  // 本块中已经返回过
  bool hasRetThisBB;

  BranchManager();
  // 生成新标签ID
  const int registerNewBB();
  // 将当前while循环信息推入栈
  void PushInfo(const LoopInfo info);
  // 弹出一个while循环信息
  void PopInfo();
  // 获取当前while循环信息
  const LoopInfo& GetCurInfo() const;
  // 判断当前是否在循环中
  const bool IsInALoop() const;
  // 为了break和continue生成一个不可达的label
  const string GenerateLabelFromBranchedLoop();
  // 刷新状态
  void Reset();

 private:
  int bbPool;
  deque<LoopInfo> loopStack;
};

#pragma endregion

#pragma region Func

class FuncManager {
 public:
  FuncManager();

  string func_name;
  VarType ret_ty;
  RetInfo ret_info;
  // 函数参数
  vector<SymbolTableEntry> params;

  // 生成函数开头
  void WriteFuncPrologue();
  // 生成函数屁股
  void WriteFuncEpilogue();
  // 生成返回指令
  void WriteRetInst();
  // 获取临时变量ID
  const int registerNewSymbol();
  // 插入参数信息
  void InsertParam(VarType ty, string name);
  // 打印参数信息，在函数定义处
  void WriteParamsDefine();
  // 为参数分配新的变量，函数定义完后
  void WriteAllocParams();
  // 刷新状态
  void Reset();
  // 设置返回值为函数对应返回值的默认retinfo
  void SetDefaultRetInfo();
  // 返回函数表
  const map<string, VarType>& GetFuncTable() const;
  // 将库函数加入函数表
  void AddLibFuncs();

 private:
  // 临时符号ID
  int symbolPool;
  // 函数表，包含函数名和返回值类型
  // 返回值类型决定是否用符号存储其返回值
  // 不考虑参数，因为给定的程序语法一定正确
  map<string, VarType> func_table;
  // 参数特有名称name_p
  const string getParamVarName(const string& name) const;
};

#pragma endregion

class IRGenerator {
 private:
  IRGenerator();
  IRGenerator(const IRGenerator&) = delete;
  IRGenerator(const IRGenerator&&) = delete;
  IRGenerator& operator=(const IRGenerator&) = delete;

 public:
  static IRGenerator& getInstance();
  GenSettings setting;

  SymbolManager symbolCore;
  BranchManager branchCore;
  FuncManager funcCore;

#pragma region lv3

  // 生成函数开头
  void WriteFuncPrologue();
  // 生成函数屁股
  void WriteFuncEpilogue();
  // 生成返回指令
  void WriteRetInst();
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
  // 输出加载变量的指令
  const RetInfo WriteLoadInst(const SymbolTableEntry& entry);
  // 输出写入变量的指令
  void WriteStoreInst(const RetInfo& value, const SymbolTableEntry& entry);

#pragma endregion

#pragma region lv6

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

#pragma region lv7

  // 初始化LoopInfo（设置cond label）
  void InitLoopInfo(LoopInfo& info);
  // 生成while相关跳转指令（br），label存在loopInfo中
  void WriteBrInst(const RetInfo& cond, LoopInfo& loopInfo);

#pragma endregion

#pragma region lv8

  // 生成call指令
  const RetInfo WriteCallInst(const string& func_name,
                              const vector<RetInfo>& params);
  // 生成库函数定义
  void WriteLibFuncDecl();
  // 生成全局变量定义
  void WriteGlobalVar(const SymbolTableEntry& entry, const RetInfo& init);

#pragma endregion

#pragma region lv9
  // 生成全局数组变量定义
  void WriteGlobalArrVar(const SymbolTableEntry& entry,
                         const vector<RetInfo>& init);

  // 生成局部数组变量定义
  void WriteAllocInst(const SymbolTableEntry& entry,
                      const bool& has_init,
                      const vector<RetInfo>& init);

  // 生成getelemptr语句
  // 语法：{symbol} = getelemptr {arr_var}, {addr}
  // 行为：取出arr_var[addr]的地址存入symbol
  void WriteGetelemptrInst(const string& symbol,
                           const string& arr_var,
                           const RetInfo& addr) const;

  // 生成从数组中load值的语句
  const RetInfo WriteLoadArrInst(const SymbolTableEntry& entry,
                                 const RetInfo& addr);

  // 生成向数组中store值的语句
  void WriteStoreArrInst(const SymbolTableEntry& entry,
                         const RetInfo& value,
                         const RetInfo& addr);

 private:
  const int registerNewVar();
  const string getSymbolName(const int& symbol) const;
  const string getLabelName(const int& bb_id) const;
  // 计算常数表达式
  int calcConstExpr(const int& left, const int& right, OpID op);
};

}  // namespace ir