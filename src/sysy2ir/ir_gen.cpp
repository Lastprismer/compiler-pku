#include "ir_gen.h"

namespace ir {

#pragma region RetInfo

RetInfo::RetInfo() : ty(ty_void) {}

RetInfo::RetInfo(int _value) : ty(ty_int), value(_value) {}

RetInfo::RetInfo(string _symbol) : ty(ty_sbl), name(_symbol) {}

const int& RetInfo::GetValue() const {
  assert(ty == ty_int);
  return value;
}

const string& RetInfo::GetSym() const {
  assert(ty == ty_sbl);
  return name;
}

const string RetInfo::GetInfo() const {
  switch (ty) {
    case retty_t::ty_int:
      return to_string(GetValue());
    case retty_t::ty_sbl:
      return GetSym();
    default:
      return "";
  }
}

#pragma endregion

#pragma region IfInfo

IfInfo::IfInfo()
    : ty(ifty_t::i), then_label(-1), else_label(-1), next_label(-1) {}
IfInfo::IfInfo(ifty_t _ty)
    : ty(_ty), then_label(-1), else_label(-1), next_label(-1) {}

IfInfo::IfInfo(int then, int next)
    : ty(ifty_t::i), then_label(then), next_label(next) {}

IfInfo::IfInfo(int then, int _else, int next)
    : ty(ifty_t::ie), then_label(then), else_label(_else), next_label(next) {}

#pragma endregion

#pragma region LoopInfo

LoopInfo::LoopInfo() {}

#pragma endregion

#pragma region STE

SymbolTableEntry::SymbolTableEntry()
    : symbol_type(SymbolType::e_unused), var_type(VarType::e_unused), id(-1) {}

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
  table.emplace(entry.var_name, entry);
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

SymbolTableEntry DeclaimProcessor::GenerateConstEntry(const string& var_name,
                                                      const int& value) {
  assert(IsEnabled() && current_symbol_type == SymbolType::e_const &&
         current_var_type != VarType::e_unused);
  SymbolTableEntry ste;
  ste.symbol_type = SymbolType::e_const;
  ste.var_type = VarType::e_int;
  ste.var_name = var_name;
  ste.const_value = value;
  ste.id = RegisterVar();
  return ste;
}

SymbolTableEntry DeclaimProcessor::GenerateArrEntry(const string& var_name,
                                                    const ArrInfo& info) {
  SymbolTableEntry ste;
  ste.symbol_type = SymbolType::e_const;
  ste.var_type = VarType::e_arr;
  ste.var_name = var_name;
  ste.arr_info = info;
  ste.id = RegisterVar();
  return ste;
}

SymbolTableEntry DeclaimProcessor::GenerateVarEntry(const string& var_name,
                                                    const VarType& var_ty) {
  assert(IsEnabled() && current_symbol_type != SymbolType::e_unused &&
         current_var_type != VarType::e_unused);
  SymbolTableEntry ste;
  ste.symbol_type = SymbolType::e_var;
  ste.var_type = VarType::e_int;
  ste.var_name = var_name;
  ste.id = RegisterVar();
  return ste;
}

SymbolTableEntry DeclaimProcessor::QuickGenEntry(const SymbolType& st,
                                                 const VarType& vt,
                                                 string name) {
  SymbolTableEntry ste;
  ste.symbol_type = st;
  ste.var_type = vt;
  ste.var_name = name;
  ste.id = RegisterVar();
  return ste;
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

void AssignmentProcessor::WriteAssign(const RetInfo& value) const {
  auto& gen = IRGenerator::getInstance();
  if (current_var.var_type == VarType::e_int) {
    gen.WriteStoreInst(value, current_var);
  } else if (current_var.var_type == VarType::e_arr) {
    gen.WriteStoreArrInst(current_var, value, arr_addr);
  }
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

#pragma region Branch

BranchManager::BranchManager() : hasRetThisBB(false), bbPool(0), loopStack() {}

const int BranchManager::registerNewBB() {
  return bbPool++;
}

void BranchManager::PushInfo(const LoopInfo info) {
  loopStack.push_back(info);
}

void BranchManager::PopInfo() {
  loopStack.pop_back();
}

const LoopInfo& BranchManager::GetCurInfo() const {
  return loopStack.back();
}

const bool BranchManager::IsInALoop() const {
  return loopStack.size() > 0;
}

const string BranchManager::GenerateLabelFromBranchedLoop() {
  return '%' + string("unreachable_in_loop_") + to_string(registerNewBB());
}

void BranchManager::Reset() {
  assert(loopStack.size() == 0);
  bbPool = 0;
  hasRetThisBB = false;
}

#pragma endregion

#pragma region func

FuncManager::FuncManager()
    : func_name(), ret_ty(), ret_info(), symbolPool(0), func_table() {}

void FuncManager::WriteFuncPrologue() {
  // 记入函数表
  func_table.emplace(func_name, ret_ty);

  auto& setting = IRGenerator::getInstance().setting;
  auto& os = setting.getOs();

  os << "fun @" << func_name << "(";
  WriteParamsDefine();
  os << ")" << (ret_ty == VarType::e_void ? "" : ": ")  // 格式
     << GetVarType(ret_ty) << " "                       // 格式
     << "{\n"
     << setting.getIndentStr()
     << "%"
        "entry:"
     << endl;
  setting.getIndent() += 2;
  WriteAllocParams();
  return;
}

void FuncManager::WriteFuncEpilogue() {
  auto& setting = IRGenerator::getInstance().setting;
  auto& os = setting.getOs();
  os << "}\n" << endl;
  setting.getIndent() -= 2;
  return;
}

void FuncManager::WriteRetInst() {
  auto& setting = IRGenerator::getInstance().setting;
  auto& os = setting.getOs();
  os << setting.getIndentStr() << "ret " << ret_info.GetInfo() << endl;
  IRGenerator::getInstance().branchCore.hasRetThisBB = true;
}

const int FuncManager::registerNewSymbol() {
  return symbolPool++;
}

void FuncManager::InsertParam(VarType ty, string name) {
  auto& gen = IRGenerator::getInstance();
  assert(ty == VarType::e_int);
  params.push_back(
      gen.symbolCore.dproc.QuickGenEntry(SymbolType::e_var, ty, name));
}

void FuncManager::WriteParamsDefine() {
  auto& os = IRGenerator::getInstance().setting.getOs();
  if (params.size() == 0)
    return;
  os << getParamVarName(params[0].var_name) << ": "
     << GetVarType(params[0].var_type);

  if (params.size() > 1) {
    for (auto it = ++params.begin(); it != params.end(); it++) {
      os << ", " << getParamVarName(it->var_name) << ": "
         << GetVarType(it->var_type);
    }
  }
}

void FuncManager::WriteAllocParams() {
  auto& gen = IRGenerator::getInstance();
  auto& dproc = gen.symbolCore.dproc;
  for (auto it = params.begin(); it != params.end(); it++) {
    SymbolTableEntry entry =
        dproc.QuickGenEntry(it->symbol_type, it->var_type, it->var_name);
    gen.WriteAllocInst(entry);
    gen.symbolCore.InsertEntry(entry);
    RetInfo tmp(getParamVarName(it->var_name));
    gen.WriteStoreInst(tmp, entry);
  }
}

void FuncManager::Reset() {
  func_name = string();
  ret_ty = VarType::e_int;
  ret_info = RetInfo();
  symbolPool = 0;
  params.clear();
}

void FuncManager::SetDefaultRetInfo() {
  switch (ret_ty) {
    case VarType::e_int:
      ret_info = RetInfo(0);
      break;
    case VarType::e_void:
    default:
      ret_info = RetInfo();
  }
}

const map<string, VarType>& FuncManager::GetFuncTable() const {
  return func_table;
}

void FuncManager::AddLibFuncs() {
  func_table.emplace(string("getint"), VarType::e_int);
  func_table.emplace(string("getch"), VarType::e_int);
  func_table.emplace(string("getarray"), VarType::e_int);
  func_table.emplace(string("putint"), VarType::e_void);
  func_table.emplace(string("putch"), VarType::e_void);
  func_table.emplace(string("putarray"), VarType::e_void);
  func_table.emplace(string("starttime"), VarType::e_void);
  func_table.emplace(string("stoptime"), VarType::e_void);
}

const string FuncManager::getParamVarName(const string& name) const {
  // %符号命名不会和默认@符号冲突，且变量名不可能是数字
  return "%" + name;
}
#pragma endregion

IRGenerator::IRGenerator() : symbolCore(), branchCore(), funcCore() {
  setting.setOs(cout).setIndent(0);
}

IRGenerator& IRGenerator::getInstance() {
  static IRGenerator gen;
  return gen;
}

#pragma region lv3

void IRGenerator::WriteFuncPrologue() {
  funcCore.WriteFuncPrologue();
}

void IRGenerator::WriteFuncEpilogue() {
  funcCore.WriteFuncEpilogue();
}

void IRGenerator::WriteRetInst() {
  funcCore.WriteRetInst();
}

const RetInfo IRGenerator::WriteUnaryInst(const RetInfo& left, OpID op) {
  if (op == OpID::UNARY_POS) {
    return left;
  }
  // 只有两种运算
  // 1. a = -b，等效于 a = 0 - b，推出b，加入0，推入b，调用sub
  // 2. a = !b，等效于a = 0 == b，推出b，加入0，推入b，调用eq
  else if (op == OpID::UNARY_NEG) {
    return WriteBinaryInst(RetInfo(0), left, OpID::BI_SUB);
  } else {
    return WriteBinaryInst(RetInfo(0), left, OpID::LG_EQ);
  }
}

const RetInfo IRGenerator::WriteBinaryInst(const RetInfo& left,
                                           const RetInfo& right,
                                           OpID op) {
  ostream& os = setting.getOs();

  // const expr
  if (left.ty == RetInfo::retty_t::ty_int &&
      right.ty == RetInfo::retty_t::ty_int) {
    return RetInfo(calcConstExpr(left.GetValue(), right.GetValue(), op));
  }

  const string newSymbolName = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << newSymbolName << " = " << BiOp2koopa(op)
     << ' ' << left.GetInfo() << ", " << right.GetInfo() << endl;
  return RetInfo(newSymbolName);
}

const RetInfo IRGenerator::WriteLogicInst(const RetInfo& left,
                                          const RetInfo& right,
                                          OpID op) {
  // left -> bool
  const RetInfo n_left = WriteBinaryInst(RetInfo(0), left, OpID::LG_NEQ);
  const RetInfo n_right = WriteBinaryInst(RetInfo(0), right, OpID::LG_NEQ);
  return WriteBinaryInst(n_left, n_right, op);
}

#pragma endregion

#pragma region lv4

void IRGenerator::WriteAllocInst(const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();
  os << setting.getIndentStr() << entry.GetAllocInst() << endl;
}

const RetInfo IRGenerator::WriteLoadInst(const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();
  const string newSymbolName = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << entry.GetLoadInst(newSymbolName) << endl;
  return RetInfo(newSymbolName);
}

void IRGenerator::WriteStoreInst(const RetInfo& value,
                                 const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();

  os << setting.getIndentStr();
  if (value.ty == RetInfo::ty_int) {
    // 常量赋值
    os << entry.GetStoreInst(value.GetValue()) << endl;
  } else if (value.ty == RetInfo::ty_sbl) {
    // 符号赋值
    os << entry.GetStoreInst(value.GetSym()) << endl;
  }
}

#pragma endregion

#pragma region lv6

void IRGenerator::WriteBasicBlockPrologue(const int& bb_id) {
  ostream& os = getInstance().setting.getOs();
  os << "\n%" << getLabelName(bb_id) << ":" << endl;
}

void IRGenerator::WriteBrInst(const RetInfo& cond, IfInfo& info) {
  ostream& os = getInstance().setting.getOs();
  switch (info.ty) {
    case IfInfo::ifty_t::i:
      info.then_label = branchCore.registerNewBB();
      info.next_label = branchCore.registerNewBB();
      os << setting.getIndentStr() << "br " << cond.GetInfo() << ", "
         << getLabelName(info.then_label) << ", "
         << getLabelName(info.next_label) << endl;
      break;
    case IfInfo::ifty_t::ie:
    default:
      info.then_label = branchCore.registerNewBB();
      info.else_label = branchCore.registerNewBB();
      info.next_label = branchCore.registerNewBB();
      os << setting.getIndentStr() << "br " << cond.GetInfo() << ", "
         << getLabelName(info.then_label) << ", "
         << getLabelName(info.else_label) << endl;
      break;
  }
  return;
}

void IRGenerator::WriteJumpInst(const int& labelID) {
  WriteJumpInst(getLabelName(labelID));
}

void IRGenerator::WriteJumpInst(const string& labelName) {
  ostream& os = getInstance().setting.getOs();
  os << setting.getIndentStr() << "jump " << labelName << endl;
}

void IRGenerator::WriteLabel(const int& BBId) {
  WriteLabel(getLabelName(BBId));
}

void IRGenerator::WriteLabel(const string& labelName) {
  ostream& os = getInstance().setting.getOs();
  os << "\n" << labelName << ":" << endl;
  // 新块的开始
  branchCore.hasRetThisBB = false;
}

const string IRGenerator::registerShortCircuitVar() {
  return string("if_") + to_string(registerNewVar());
}

#pragma endregion

#pragma region lv7

void IRGenerator::InitLoopInfo(LoopInfo& info) {
  info.cond_label = branchCore.registerNewBB();
}

void IRGenerator::WriteBrInst(const RetInfo& cond, LoopInfo& loopInfo) {
  ostream& os = getInstance().setting.getOs();
  loopInfo.body_label = branchCore.registerNewBB();
  loopInfo.next_label = branchCore.registerNewBB();
  os << setting.getIndentStr() << "br " << cond.GetInfo() << ", "
     << getLabelName(loopInfo.body_label) << ", "
     << getLabelName(loopInfo.next_label) << endl;
}

#pragma endregion

#pragma region lv8

#pragma endregion

const RetInfo IRGenerator::WriteCallInst(const string& func_name,
                                         const vector<RetInfo>& params) {
  auto& os = setting.getOs();
  os << setting.getIndentStr();

  // 查函数表，保存值
  bool hasRetValue = false;
  string newSymbolName;
  auto& funcTbl = funcCore.GetFuncTable();
  if (funcTbl.at(func_name) == VarType::e_int) {
    newSymbolName = getSymbolName(funcCore.registerNewSymbol());
    os << newSymbolName << " = ";
    hasRetValue = true;
  }

  os << "call @" << func_name << "(";

  int len = params.size();
  if (len > 0) {
    os << params[0].GetInfo();
    for (int i = 1; i < len; i++) {
      os << ", " << params[i].GetInfo();
    }
  }

  os << ")" << endl;

  if (hasRetValue) {
    return RetInfo(newSymbolName);
  } else {
    return RetInfo();
  }
}

void IRGenerator::WriteLibFuncDecl() {
  auto& os = setting.getOs();
  os <<
      R"EOF(decl @getint(): i32
decl @getch(): i32
decl @getarray(*i32): i32
decl @putint(i32)
decl @putch(i32)
decl @putarray(i32, *i32)
decl @starttime()
decl @stoptime())EOF"
     << endl;
  funcCore.AddLibFuncs();
}

void IRGenerator::WriteGlobalVar(const SymbolTableEntry& entry,
                                 const RetInfo& init) {
  auto& os = setting.getOs();
  os << "global " << entry.GetAllocName() << " = alloc "
     << GetVarType(entry.var_type) << ", ";
  if (init.ty == RetInfo::ty_void) {
    os << "zeroinit" << endl;
  } else {
    os << init.GetValue() << endl;
  }
}

#pragma endregion

#pragma region lv9

void IRGenerator::WriteGlobalArrVar(const SymbolTableEntry& entry,
                                    const vector<RetInfo>& init) {
  assert(entry.var_type == VarType::e_arr);
  // 暂时只能处理一维数组
  assert(entry.arr_info.dimension == 1);
  // TODO

  auto& os = setting.getOs();
  // 定义
  os << "global " << entry.GetAllocName() << " = alloc "
     << entry.arr_info.GetType() << ", ";
  // 初始化
  int init_len = init.size();
  if (init_len == 0) {
    // 使用zeroinit
    os << "zeroinit" << endl;
  } else {
    // 使用聚合init
    os << "{" << init[0].GetInfo();
    for (int i = 1; i < entry.arr_info.shape[0]; i++) {
      if (i < init_len) {
        os << ", " << init[i].GetInfo();
      } else {
        // 使用零初始化
        os << ", 0";
      }
    }
    os << "}" << endl;
  }
}

void IRGenerator::WriteAllocInst(const SymbolTableEntry& entry,
                                 const bool& has_init,
                                 const vector<RetInfo>& init) {
  assert(entry.var_type == VarType::e_arr);
  // TODO
  // 暂时只能处理一维数组
  assert(entry.arr_info.dimension == 1);

  auto& os = setting.getOs();
  const string indent = setting.getIndentStr();

  // 定义
  os << indent << entry.GetAllocName() << " = alloc "
     << entry.arr_info.GetType() << endl;

  // 初始化
  int init_len = init.size();
  if (!has_init) {
    // 不初始化
  } else {
    for (int i = 0; i < entry.arr_info.shape[0]; i++) {
      const string newSymbolName = getSymbolName(funcCore.registerNewSymbol());
      // 选择零初始化或者给定值
      WriteGetelemptrInst(newSymbolName, entry.GetAllocName(), RetInfo(i));
      os << indent << "store " << (i < init_len ? init[i].GetInfo() : "0")
         << ", " << newSymbolName << endl;
    }
  }
}

void IRGenerator::WriteGetelemptrInst(const string& symbol,
                                      const string& arr_var,
                                      const RetInfo& addr) const {
  auto& os = setting.getOs();
  const string indent = setting.getIndentStr();
  os << indent << symbol << " = getelemptr " << arr_var << ", "
     << addr.GetInfo() << endl;
}

const RetInfo IRGenerator::WriteLoadArrInst(const SymbolTableEntry& entry,
                                            const RetInfo& addr) {
  auto& os = setting.getOs();
  const string tmp_addr = getSymbolName(funcCore.registerNewSymbol());
  WriteGetelemptrInst(tmp_addr, entry.GetAllocName(), addr);

  const string ret = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << ret << " = load " << tmp_addr << endl;
  return RetInfo(ret);
}

void IRGenerator::WriteStoreArrInst(const SymbolTableEntry& entry,
                                    const RetInfo& value,
                                    const RetInfo& addr) {
  auto& os = setting.getOs();
  const string tmp_addr = getSymbolName(funcCore.registerNewSymbol());
  WriteGetelemptrInst(tmp_addr, entry.GetAllocName(), addr);

  const string ret = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << "store " << value.GetInfo() << ", "
     << tmp_addr << endl;
  return;
}

const int IRGenerator::registerNewVar() {
  return symbolCore.dproc.RegisterVar();
}

#pragma endregion

#pragma region IRGen - private

const string IRGenerator::getSymbolName(const int& symbol) const {
  return string("%") + to_string(symbol);
}

const string IRGenerator::getLabelName(const int& bb_id) const {
  return string("%") + string("label_") + to_string(bb_id);
}

int IRGenerator::calcConstExpr(const int& l, const int& r, OpID op) {
  switch (op) {
    case BI_ADD:
      return l + r;
    case BI_SUB:
      return l - r;
    case BI_MUL:
      return l * r;
    case BI_DIV:
      return l / r;
    case BI_MOD:
      return l % r;
    case LG_GT:
      return l > r;
    case LG_GE:
      return l >= r;
    case LG_LT:
      return l < r;
    case LG_LE:
      return l <= r;
    case LG_EQ:
      return l == r;
    case LG_NEQ:
      return l != r;
    case LG_AND:
      return l && r;
    case LG_OR:
      return l || r;
    default:
      return 0;
  }
  return 0;
}

#pragma endregion

}  // namespace ir