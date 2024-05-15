#include "ir_symbol.h"

namespace ir {

#pragma region Symbol Table

SymbolTableEntry::SymbolTableEntry()
    : symbol_type(SymbolType::UNUSED), var_type(VarType::UNUSED), layer(-1) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _var_name,
                                   int _const_value,
                                   int ly)
    : symbol_type(_stype),
      var_type(_vtype),
      var_name(_var_name),
      value(_const_value),
      layer(ly) {}

SymbolTableEntry::SymbolTableEntry(SymbolType _stype,
                                   VarType _vtype,
                                   string _var_name,
                                   int ly)
    : symbol_type(_stype), var_type(_vtype), var_name(_var_name), layer(ly) {}

string SymbolTableEntry::GetAllocName() const {
  return "@" + var_name + '_' + std::to_string(layer);
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
  return SymbolTableEntry(SymbolType::CONST, _current_var_type, var_name, value,
                          manager->currentTable->id);
}

SymbolTableEntry DeclaimProcessor::GenerateVarEntry(string var_name) {
  assert(IsEnabled() && _current_symbol_type != SymbolType::UNUSED &&
         _current_var_type != VarType::UNUSED);
  return SymbolTableEntry(_current_symbol_type, _current_var_type, var_name,
                          manager->currentTable->id);
}

#pragma endregion

AssignmentProcessor::AssignmentProcessor() : BaseProcessor() {}

void AssignmentProcessor::SetCurrentVar(const string& var_name) {
  current_var_name = var_name;
}

const string& AssignmentProcessor::GetCurrentVar() const {
  return current_var_name;
}

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