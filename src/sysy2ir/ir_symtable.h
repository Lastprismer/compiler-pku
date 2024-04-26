#pragma once
#include <cassert>
#include <iostream>
#include <map>
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
  string symbol_name;
  bool has_value;
  int value;
  SymbolTableEntry();
  SymbolTableEntry(SymbolType _stype, VarType _vtype, string _name, int _value);
  SymbolTableEntry(SymbolType _stype, VarType _vtype, string _name);
};

class DeclaimProcessor {
 private:
  // 保证只在处理声明的语句时启用
  bool _enable;
  SymbolType _current_symbol_type;
  VarType _current_var_type;

 public:
  DeclaimProcessor();
  void Enable();
  void Disable();
  void SetSymbolType(SymbolType type);
  void SetVarType(VarType type);
  // 重置
  void resetState();
  // 生成常数变量表项
  SymbolTableEntry GenerateConstEntry(string var_name, int value);
  // 生成有初始化的变量表项
  SymbolTableEntry GenerateVarEntry(string var_name, int value);
  // 生成无初始化地变量表项
  SymbolTableEntry GenerateVarEntry(string var_name);
};

class SymbolTable {
 public:
  // 表
  map<string, SymbolTableEntry> table;
  DeclaimProcessor processer;
  SymbolTable();
  // utility
  SymbolTableEntry& getEntry(string _symbol_name);
  void insertEntry(SymbolTableEntry entry);
  DeclaimProcessor& getProcessor();
  void clearTable();
};

}  // namespace ir