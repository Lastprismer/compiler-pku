#include "ir_symtable.h"

namespace ir {

#pragma region Symbol Table

SymbolTableEntry::SymbolTableEntry()
    : symbol_type(SymbolType::UNUSED), var_type(VarType::UNUSED) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _var_name,
                                   int _const_value)
    : symbol_type(_stype),
      var_type(_vtype),
      var_name(_var_name),
      value(_const_value) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _var_name)
    : symbol_type(_stype), var_type(_vtype), var_name(_var_name) {}

string SymbolTableEntry::GetAllocName() const {
  return "@" + var_name;
}

string SymbolTableEntry::GetAllocInst() const {
  assert(symbol_type == SymbolType::VAR);
  if (var_type == VarType::INT) {
    // int
    // @x = alloc i32
    stringstream ss;
    ss << GetAllocName() << " = alloc i32";
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
  }
}

string SymbolTableEntry::GetLoadInst(string sym_name_loadto) const {
  assert(symbol_type == SymbolType::VAR);
  if (var_type == VarType::INT) {
    // int
    // %0 = load @x
    stringstream ss;
    ss << sym_name_loadto << " = load " << GetAllocName();
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
  }
}

string SymbolTableEntry::GetStoreInst(string from_sym_name) const {
  assert(symbol_type == SymbolType::VAR);
  if (var_type == VarType::INT) {
    // int
    // store %0, @x
    stringstream ss;
    ss << "store " << from_sym_name << ", " << GetAllocName();
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
  }
}

string SymbolTableEntry::GetStoreInst(int imm) const {
  assert(symbol_type == SymbolType::VAR);
  if (var_type == VarType::INT) {
    // int
    // store 0, @x
    stringstream ss;
    ss << "store " << imm << ", " << GetAllocName();
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
  }
}

SymbolTable::SymbolTable() {
  table = map<string, SymbolTableEntry>();
  dproc = DeclaimProcessor();
}

SymbolTableEntry& SymbolTable::getEntry(string _symbol_name) {
  if (table.count(_symbol_name) == 1) {
    return table[_symbol_name];
  }
  cerr << "symbol table: key not in table:" << _symbol_name << endl;
  assert(false);
}

void SymbolTable::insertEntry(SymbolTableEntry entry) {
  if (table.count(entry.var_name) > 0) {
    cerr << "Symbol Table insert error: symbol with name \"" << entry.var_name
         << "\" has already inserted in the table. It will be overwritten by "
            "default"
         << endl;
    table[entry.var_name] = entry;
  }
  // do actual insert
  table.emplace(make_pair(entry.var_name, entry));
}

#pragma endregion

#pragma region BaseProcessor

BaseProcessor::BaseProcessor() : _enable(false) {}

void BaseProcessor::Enable() {
  _enable = true;
}

void BaseProcessor::Disable() {
  _enable = false;
}

const bool& BaseProcessor::IsEnabled() {
  return _enable;
}

#pragma endregion

#pragma region DeclaimProcessor

DeclaimProcessor& SymbolTable::getDProc() {
  return dproc;
}

AssignmentProcessor& SymbolTable::getAProc() {
  return aproc;
}

void SymbolTable::clearTable() {
  table.clear();
}

DeclaimProcessor::DeclaimProcessor()
    : BaseProcessor(),
      _current_symbol_type(SymbolType::UNUSED),
      _current_var_type(VarType::UNUSED) {}

void DeclaimProcessor::SetSymbolType(SymbolType type) {
  assert(IsEnabled());
  _current_symbol_type = type;
}

void DeclaimProcessor::SetVarType(VarType type) {
  assert(IsEnabled());
  _current_var_type = type;
}

void DeclaimProcessor::resetState() {
  _current_symbol_type = SymbolType::UNUSED;
  _current_var_type = VarType::UNUSED;
}

SymbolTableEntry DeclaimProcessor::GenerateConstEntry(string var_name,
                                                      int value) {
  assert(IsEnabled() && _current_symbol_type == SymbolType::CONST &&
         _current_var_type != VarType::UNUSED);
  return SymbolTableEntry(SymbolType::CONST, _current_var_type, var_name,
                          value);
}

SymbolTableEntry DeclaimProcessor::GenerateVarEntry(string var_name) {
  assert(IsEnabled() && _current_symbol_type != SymbolType::UNUSED &&
         _current_var_type != VarType::UNUSED);
  return SymbolTableEntry(_current_symbol_type, _current_var_type, var_name);
}

#pragma endregion

AssignmentProcessor::AssignmentProcessor() : BaseProcessor() {}

void AssignmentProcessor::SetCurrentVar(const string& var_name) {
  current_var_name = var_name;
}

const string& AssignmentProcessor::GetCurrentVar() const {
  return current_var_name;
}

}  // namespace ir