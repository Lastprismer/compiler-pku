#include "ir_symbol.h"

namespace ir {
#pragma region STE

SymbolTableEntry::SymbolTableEntry()
    : symbol_type(SymbolType::e_unused), var_type(VarType::e_unused), id(-1) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _var_name,
                                   int _const_value,
                                   int uuid)
    : symbol_type(_stype),
      var_type(_vtype),
      var_name(_var_name),
      const_value(_const_value),
      id(uuid) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _var_name,
                                   int uuid)
    : symbol_type(_stype), var_type(_vtype), var_name(_var_name), id(uuid) {}

const string SymbolTableEntry::GetAllocName() const {
  return "@" + var_name + '_' + std::to_string(id);
}

const string SymbolTableEntry::GetAllocInst() const {
  assert(symbol_type == SymbolType::e_var);
  if (var_type == VarType::e_int) {
    // int
    // @x = alloc i32
    stringstream ss;
    ss << GetAllocName() << " = alloc i32";
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
    return "";
  }
}

const string SymbolTableEntry::GetLoadInst(
    const string& loadToSymbolName) const {
  assert(symbol_type == SymbolType::e_var);
  if (var_type == VarType::e_int) {
    // int
    // %0 = load @x
    stringstream ss;
    ss << loadToSymbolName << " = load " << GetAllocName();
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
    return "";
  }
}

const string SymbolTableEntry::GetStoreInst(
    const string& storeFromSymbolName) const {
  assert(symbol_type == SymbolType::e_var);
  if (var_type == VarType::e_int) {
    // int
    // store %0, @x
    stringstream ss;
    ss << "store " << storeFromSymbolName << ", " << GetAllocName();
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
    return "";
  }
}

const string SymbolTableEntry::GetStoreInst(const int& imm) const {
  // (symbolType == SymbolType::VAR);
  if (var_type == VarType::e_int) {
    // int
    // store 0, @x
    stringstream ss;
    ss << "store " << imm << ", " << GetAllocName();
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
    return "";
  }
}

#pragma endregion

#pragma region Symbol Table

SymbolTable::SymbolTable() : table(), parent(nullptr) {}

bool SymbolTable::TryGetEntry(string symbol_name, SymbolTableEntry& out) const {
  if (table.find(symbol_name) != table.end()) {
    out = table.at(symbol_name);
    return true;
  }
  return false;
}

void SymbolTable::InsertEntry(const SymbolTableEntry& entry) {
  if (table.find(entry.var_name) != table.end()) {
    cerr << "Symbol Table insert error: symbol with name \"" << entry.var_name
         << "\" has already inserted in the table. It will be overwritten by "
            "default"
         << endl;
    table[entry.var_name] = entry;
    return;
  }
  // do actual insert
  table.emplace(make_pair(entry.var_name, entry));
}

void SymbolTable::ClearTable() {
  table.clear();
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

DeclaimProcessor::DeclaimProcessor()
    : BaseProcessor(),
      current_symbol_type(SymbolType::e_unused),
      current_var_type(VarType::e_unused) {}

void DeclaimProcessor::SetSymbolType(const SymbolType& type) {
  assert(IsEnabled());
  current_symbol_type = type;
}

void DeclaimProcessor::SetVarType(const VarType& type) {
  assert(IsEnabled());
  current_var_type = type;
}

void DeclaimProcessor::Reset() {
  current_symbol_type = SymbolType::e_unused;
  current_var_type = VarType::e_unused;
}

SymbolTableEntry DeclaimProcessor::GenerateConstEntry(const string& varName,
                                                      const int& value) {
  assert(IsEnabled() && current_symbol_type == SymbolType::e_const &&
         current_var_type != VarType::e_unused);
  return SymbolTableEntry(SymbolType::e_const, current_var_type, varName, value,
                          -1);
}

SymbolTableEntry DeclaimProcessor::GenerateVarEntry(const string& varName) {
  assert(IsEnabled() && current_symbol_type != SymbolType::e_unused &&
         current_var_type != VarType::e_unused);
  return SymbolTableEntry(current_symbol_type, current_var_type, varName,
                          RegisterVar());
}

SymbolTableEntry DeclaimProcessor::QuickGenEntry(const SymbolType& st,
                                                 const VarType& vt,
                                                 string name) {
  return SymbolTableEntry(st, vt, name, RegisterVar());
}

const SymbolType DeclaimProcessor::getCurSymType() const {
  return current_symbol_type;
}

const VarType DeclaimProcessor::getCurVarType() const {
  return current_var_type;
}

const int DeclaimProcessor::RegisterVar() {
  return var_pool++;
}

#pragma endregion

#pragma region Assignment

AssignmentProcessor::AssignmentProcessor() : BaseProcessor() {}

void AssignmentProcessor::SetCurrentVar(const string& var_name) {
  CurrentVarName = var_name;
}

const string& AssignmentProcessor::GetCurrentVar() const {
  return CurrentVarName;
}

#pragma endregion

SymbolManager::SymbolManager() : dproc(), aproc(), RootTable() {
  currentTable = &RootTable;
}

const SymbolTableEntry SymbolManager::getEntry(string symbol_name) {
  SymbolTableEntry entry;
  SymbolTable* search = currentTable;
  while (search != nullptr) {
    if (search->TryGetEntry(symbol_name, entry))
      return entry;
    search = search->parent;
  }
  cerr << "SymbolManager: don't find symbol with name " << symbol_name << endl;
  assert(false);
  return SymbolTableEntry();
}

void SymbolManager::InsertEntry(SymbolTableEntry entry) {
  currentTable->InsertEntry(entry);
}

void SymbolManager::PushScope() {
  SymbolTable* tmp = new SymbolTable();
  tmp->parent = currentTable;
  currentTable = tmp;
}

void SymbolManager::PopScope() {
  SymbolTable* tmp = currentTable;
  currentTable = currentTable->parent;
  delete tmp;
}

}  // namespace ir