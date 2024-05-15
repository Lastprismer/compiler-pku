#include "ir_symbol.h"

namespace ir {

#pragma region Symbol Table

SymbolTableEntry::SymbolTableEntry()
    : symbol_type(SymbolType::UNUSED), var_type(VarType::UNUSED), id(-1) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _var_name,
                                   int _const_value,
                                   int uuid)
    : symbol_type(_stype),
      var_type(_vtype),
      VarName(_var_name),
      value(_const_value),
      id(uuid) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _var_name,
                                   int uuid)
    : symbol_type(_stype), var_type(_vtype), VarName(_var_name), id(uuid) {}

string SymbolTableEntry::GetAllocName() const {
  return "@" + VarName + '_' + std::to_string(id);
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

SymbolTable::SymbolTable(int ID) : table(), id(ID), parent(nullptr) {}

bool SymbolTable::TryGetEntry(string symbol_name, SymbolTableEntry& out) const {
  if (table.find(symbol_name) != table.end()) {
    out = table.at(symbol_name);
    return true;
  }
  return false;
}

void SymbolTable::InsertEntry(const SymbolTableEntry& entry) {
  if (table.find(entry.VarName) != table.end()) {
    cerr << "Symbol Table insert error: symbol with name \"" << entry.VarName
         << "\" has already inserted in the table. It will be overwritten by "
            "default"
         << endl;
    table[entry.VarName] = entry;
    return;
  }
  // do actual insert
  table.emplace(make_pair(entry.VarName, entry));
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
      CurrentSymbolType(SymbolType::UNUSED),
      CurrentVarType(VarType::UNUSED) {}

void DeclaimProcessor::SetSymbolType(SymbolType type) {
  assert(IsEnabled());
  CurrentSymbolType = type;
}

void DeclaimProcessor::SetVarType(VarType type) {
  assert(IsEnabled());
  CurrentVarType = type;
}

void DeclaimProcessor::resetState() {
  CurrentSymbolType = SymbolType::UNUSED;
  CurrentVarType = VarType::UNUSED;
}

SymbolTableEntry DeclaimProcessor::GenerateConstEntry(string var_name,
                                                      int value) {
  assert(IsEnabled() && CurrentSymbolType == SymbolType::CONST &&
         CurrentVarType != VarType::UNUSED);
  return SymbolTableEntry(SymbolType::CONST, CurrentVarType, var_name, value,
                          manager->currentTable->id);
}

SymbolTableEntry DeclaimProcessor::GenerateVarEntry(string var_name) {
  assert(IsEnabled() && CurrentSymbolType != SymbolType::UNUSED &&
         CurrentVarType != VarType::UNUSED);
  return SymbolTableEntry(CurrentSymbolType, CurrentVarType, var_name,
                          manager->currentTable->id);
}

SymbolTableEntry DeclaimProcessor::QuickGenEntry(SymbolType st,
                                                 VarType vt,
                                                 string name) {
  return SymbolTableEntry(st, vt, name, manager->currentTable->id);
}

const SymbolType& DeclaimProcessor::getCurSymType() const {
  return CurrentSymbolType;
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

SymbolManager::SymbolManager()
    : dproc(), aproc(), tableIDPool(0), RootTable(registerNewTable()) {
  currentTable = &RootTable;
  dproc.manager = this;
  aproc.manager = this;
}

DeclaimProcessor& SymbolManager::getDProc() {
  return dproc;
}

AssignmentProcessor& SymbolManager::getAProc() {
  return aproc;
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
}

void SymbolManager::InsertEntry(SymbolTableEntry entry) {
  currentTable->InsertEntry(entry);
}

void SymbolManager::PushScope() {
  SymbolTable* tmp = new SymbolTable(registerNewTable());
  tmp->parent = currentTable;
  currentTable = tmp;
}

void SymbolManager::PopScope() {
  SymbolTable* tmp = currentTable;
  currentTable = currentTable->parent;
  delete tmp;
}

int SymbolManager::registerNewTable() {
  return tableIDPool++;
}

}  // namespace ir