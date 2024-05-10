#pragma once
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace ir {
using namespace std;

enum class SymbolType { UNUSED, CONST, VAR };
enum class VarType { UNUSED, INT };

struct SymbolTableEntry {
  // 常数或变量
  SymbolType symbol_type;
  // 变量类型
  VarType var_type;
  string var_name;

  int value;
  SymbolTableEntry();
  SymbolTableEntry(SymbolType _stype,
                   VarType _vtype,
                   string _var_name,
                   int _value);
  SymbolTableEntry(SymbolType _stype, VarType _vtype, string _var_name);
  // @ + name
  string GetAllocName() const;
  // 声明的指令
  string GetAllocInst() const;
  // 加载的指令
  string GetLoadInst(string in_sym_name) const;
  // 赋值的指令
  string GetStoreInst(string from_sym_name) const;
  string GetStoreInst(int imm) const;
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
  SymbolType _current_symbol_type;
  VarType _current_var_type;

 public:
  DeclaimProcessor();
  void SetSymbolType(SymbolType type);
  void SetVarType(VarType type);
  // 重置
  void resetState();
  // 生成常数变量表项
  SymbolTableEntry GenerateConstEntry(string var_name, int value);
  // 生成无初始化的变量表项
  SymbolTableEntry GenerateVarEntry(string var_name);
};

class AssignmentProcessor : public BaseProcessor {
 private:
  // 保证只在处理赋值的语句时启用
  string current_var_name;

 public:
  AssignmentProcessor();
  void SetCurrentVar(const string& var_name);
  const string& GetCurrentVar() const;
};

class SymbolTable {
 public:
  // 表
  map<string, SymbolTableEntry> table;
  DeclaimProcessor dproc;
  AssignmentProcessor aproc;
  SymbolTable();
  // utility
  SymbolTableEntry& getEntry(string _symbol_name);
  void insertEntry(SymbolTableEntry entry);
  DeclaimProcessor& getDProc();
  AssignmentProcessor& getAProc();
  void clearTable();
};

}  // namespace ir