#pragma once
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace ir {
using std::cerr, std::endl;
using std::string, std::map, std::stringstream;
class SymbolManager;

enum class SymbolType { UNUSED, CONST, VAR };
enum class VarType { UNUSED, INT };

struct SymbolTableEntry {
  // 常数或变量
  SymbolType symbolType;
  // 变量类型
  VarType varType;
  // 懒得union
  string varName;
  int value;

  int id;
  SymbolTableEntry();
  SymbolTableEntry(SymbolType symbol_ty,
                   VarType var_ty,
                   string var_name,
                   int value,
                   int layer);
  SymbolTableEntry(SymbolType symbol_ty,
                   VarType var_ty,
                   string var_name,
                   int layer);
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
class DeclaimProcessor : public BaseProcessor {
 private:
  // 保证只在处理声明的语句时启用
  SymbolType currentSymbolType;
  VarType currentVarType;
  int varPool;

 public:
  DeclaimProcessor();
  void SetSymbolType(const SymbolType& type);
  void SetVarType(const VarType& type);
  // 重置
  void resetState();
  // 生成常数变量表项
  SymbolTableEntry GenerateConstEntry(const string& varName, const int& value);
  // 生成无初始化的变量表项
  SymbolTableEntry GenerateVarEntry(const string& varName);
  // 即时生成表项
  SymbolTableEntry QuickGenEntry(const SymbolType& st,
                                 const VarType& vt,
                                 string name);
  // 获取当前正在初始化的变量的类型（逻辑运算的编译时常数用）
  const SymbolType& getCurSymType() const;
  const int RegisterVar();
};

class AssignmentProcessor : public BaseProcessor {
 private:
  // 保证只在处理赋值的语句时启用
  string CurrentVarName;

 public:
  AssignmentProcessor();
  void SetCurrentVar(const string& var_name);
  const string& GetCurrentVar() const;
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
  DeclaimProcessor dproc;
  AssignmentProcessor aproc;

  // lv5
  // 根表
  SymbolTable RootTable;
  // 当前的表
  SymbolTable* currentTable;

  SymbolManager();

  DeclaimProcessor& getDProc();
  AssignmentProcessor& getAProc();
  // 递归从当前的表向根表查询
  const SymbolTableEntry getEntry(string symbolName);
  // 向当前的表插入
  void InsertEntry(SymbolTableEntry entry);
  // 推入一个新表
  void PushScope();
  // 弹出一个表
  void PopScope();
};

}  // namespace ir