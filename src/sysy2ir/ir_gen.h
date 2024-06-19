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

// 数组信息
struct ArrInfo {
  vector<int> shape;
  int size;
  ArrInfo();
  ArrInfo(const vector<int>& _shape);
  // 获取koopa变量类型名称，如[i32, 2]
  const string GetType() const;
  // lv9-3 获取koopa指针变量类型名，如*i32, *[i32, 3]
  const string GetPtrType() const;
  // shape.len()
  const int Dim() const;
  // 获取大小，shape累乘
  const int GetSize() const;
  // 截取一个新的shape
  ArrInfo GetFrag(const int& begin, const int& end) const;
};

// 数组初始化多叉树节点
struct ArrInitNode {
  enum node_t { e_arr, e_value } ty;
  RetInfo value;
  vector<ArrInitNode> nodes;
  ArrInitNode* parent;

  ArrInitNode();
  ArrInitNode(const RetInfo& val);
  ArrInitNode(const node_t& type);
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

  // 生成指针变量表项
  SymbolTableEntry GeneratePtrEntry(const string& var_name,
                                    const ArrInfo& ptr_info);

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
  vector<RetInfo> arr_addr;

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
  // 插入参数信息，数组用
  // 蛤蛤，大屎山来喽
  void InsertParam(const SymbolTableEntry& entry);
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

#pragma region ArrInit

class ArrInitManager {
 public:
  ArrInitNode root;
  ArrInitNode* current;

  // dump中最外层大括号不用插入
  bool should_insert;
  // 使用zeroinit
  bool zero_init;
  // 初始化的是全局数组
  bool global;

  ArrInitManager();
  // 向当前指向的列表中推入一个值
  void PushInfo(const RetInfo& info);
  // 向当前指向列表中推入一个列表，并指向下层列表
  void PushArr();
  // 回到上级列表
  void PopArr();
  // 初始化
  void Clear();
  // 给定目标arrsize，输出初始化信息
  const vector<RetInfo> GetInits(const ArrInfo& shape);
  // 给定初始化信息和数组shape，输出格式化的大括号表达式
  void GetInitString(const ArrInfo& shape,
                     const vector<RetInfo>& inits,
                     stringstream& output);

 private:
  // 递归处理：给定arr node和数组的size
  void RecursionGetInits(const ArrInitNode& node,
                         const ArrInfo& shape,
                         vector<RetInfo>& appendto,
                         bool first_layer);
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
  ArrInitManager arrinitCore;

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
  void WriteGlobalArrVar(const SymbolTableEntry& entry);

  // 生成局部数组变量定义
  void WriteAllocArrInst(const SymbolTableEntry& entry, const bool& has_init);

  // 生成getelemptr语句
  // 语法：{symbol} = getelemptr {arr_var}, {addr}
  // 行为：取出arr_var[addr]的地址存入symbol
  void WriteGetelemptrInst(const string& symbol,
                           const string& arr_var,
                           const string& addr) const;

  // 获取指向数组给定地址处的指针，并写下相关指令
  const string WriteGetPtrFromArr(const string& arr_var,
                                  const vector<RetInfo>& addr);

  // 获取存放指向数组给定地址处的指针的变量名，并写下相关一连串指令
  // int版
  const string WriteGetPtrFromArrInt(const string& arr_var,
                                     const vector<int>& addr);

  // 生成从数组中load值的语句
  const RetInfo WriteLoadArrInst(const SymbolTableEntry& entry,
                                 const vector<RetInfo>& addr);

  // 生成向数组中store值的语句
  void WriteStoreArrInst(const SymbolTableEntry& entry,
                         const RetInfo& value,
                         const vector<RetInfo>& addr);

  // 生成getptr语句
  // 语法：{symbol} = getelemptr {ptr_var}, {addr}
  // 行为：取出ptr_var[addr]的地址存入symbol
  void WriteGetPtrInst(const string& symbol,
                       const string& arr_var,
                       const string& addr) const;

  // 生成指针变量函数参数定义
  void WriteAllocPtrInst(const SymbolTableEntry& entry);

  // 获取指向指针+index给定地址处的指针，并写下相关一连串指令
  const string WriteGetPtrFromPtr(const string& arr_var,
                                  const vector<RetInfo>& addr);

  // 生成从指针中load值的语句
  const RetInfo WriteLoadPtrInst(const SymbolTableEntry& entry,
                                 const vector<RetInfo>& addr);

  // 生成向指针中store值的语句
  void WriteStorePtrInst(const SymbolTableEntry& entry,
                         const RetInfo& value,
                         const vector<RetInfo>& addr);

 private:
  const int registerNewVar();
  const string getSymbolName(const int& symbol) const;
  const string getLabelName(const int& bb_id) const;
  // 计算常数表达式
  int calcConstExpr(const int& left, const int& right, OpID op);
};

}  // namespace ir