#include "ir_symtable.h"

namespace ir {

SymbolTableEntry::SymbolTableEntry()
    : symbol_type(SymbolType::UNUSED), var_type(VarType::UNUSED) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _name,
                                   int _const_value)
    : symbol_type(_stype),
      var_type(_vtype),
      symbol_name(_name),
      has_value(true),
      value(_const_value) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _name)
    : symbol_type(_stype),
      var_type(_vtype),
      symbol_name(_name),
      has_value(false) {}

SymbolTable::SymbolTable() {
  table = map<string, SymbolTableEntry>();
  processer = DeclaimProcessor();
}

SymbolTableEntry& SymbolTable::getEntry(string _symbol_name) {
  if (table.count(_symbol_name) == 1) {
    return table[_symbol_name];
  }
  cerr << "symbol table: key not in table:" << _symbol_name << endl;
  assert(false);
}

void SymbolTable::insertEntry(SymbolTableEntry entry) {
  if (table.count(entry.symbol_name) > 0) {
    cerr << "Symbol Table insert error: symbol with name \""
         << entry.symbol_name
         << "\" has already inserted in the table. It will be overwritten by "
            "default"
         << endl;
    table[entry.symbol_name] = entry;
  }
  // do actual insert
  table.emplace(make_pair(entry.symbol_name, entry));
}

DeclaimProcessor& SymbolTable::getProcessor() {
  return processer;
}

void SymbolTable::clearTable() {
  table.clear();
}

DeclaimProcessor::DeclaimProcessor()
    : _enable(false),
      _current_symbol_type(SymbolType::UNUSED),
      _current_var_type(VarType::UNUSED) {}

void DeclaimProcessor::Enable() {
  _enable = true;
  resetState();
}

void DeclaimProcessor::Disable() {
  _enable = false;
}

void DeclaimProcessor::SetSymbolType(SymbolType type) {
  assert(_enable);
  _current_symbol_type = type;
}

void DeclaimProcessor::SetVarType(VarType type) {
  assert(_enable);
  _current_var_type = type;
}

void DeclaimProcessor::resetState() {
  _current_symbol_type = SymbolType::UNUSED;
  _current_var_type = VarType::UNUSED;
}

SymbolTableEntry DeclaimProcessor::GenerateConstEntry(string var_name,
                                                      int value) {
  assert(_enable && _current_symbol_type == SymbolType::CONST &&
         _current_var_type != VarType::UNUSED);
  return SymbolTableEntry(SymbolType::CONST, _current_var_type, var_name,
                          value);
}

SymbolTableEntry DeclaimProcessor::GenerateVarEntry(string var_name,
                                                    int value) {
  assert(_enable && _current_symbol_type != SymbolType::UNUSED &&
         _current_var_type != VarType::UNUSED);
  return SymbolTableEntry(_current_symbol_type, _current_var_type, var_name,
                          value);
}

SymbolTableEntry DeclaimProcessor::GenerateVarEntry(string var_name) {
  assert(_enable && _current_symbol_type != SymbolType::UNUSED &&
         _current_var_type != VarType::UNUSED);
  return SymbolTableEntry(_current_symbol_type, _current_var_type, var_name);
}

}  // namespace ir