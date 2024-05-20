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
  func_table.emplace(make_pair(func_name, ret_ty));

  auto& setting = IRGenerator::getInstance().setting;
  auto& os = setting.getOs();

  os << "fun @" << func_name << "(";
  WriteParamsDefine();
  os << ") " << (ret_ty == VarType::e_void ? "" : ":")                // 格式
     << getVarType(ret_ty) << (ret_ty == VarType::e_void ? "" : " ")  // 格式
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
  assert(ty == VarType::e_int);
  params.push_back(SymbolTableEntry(SymbolType::e_var, ty, name, -1));
}

void FuncManager::WriteParamsDefine() {
  auto& os = IRGenerator::getInstance().setting.getOs();
  if (params.size() == 0)
    return;
  os << getParamVarName(params[0].var_name) << ": "
     << getVarType(params[0].var_type);

  if (params.size() > 1) {
    for (auto it = ++params.begin(); it != params.end(); it++) {
      os << ", " << getParamVarName(it->var_name) << ": "
         << getVarType(it->var_type);
    }
  }
}

void FuncManager::WriteAllocParams() {
  auto& gen = IRGenerator::getInstance();
  auto& dproc = gen.symbolCore.getDProc();
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
  func_table.emplace(make_pair(string("getint"), VarType::e_int));
  func_table.emplace(make_pair(string("getch"), VarType::e_int));
  func_table.emplace(make_pair(string("getarray"), VarType::e_int));
  func_table.emplace(make_pair(string("putint"), VarType::e_void));
  func_table.emplace(make_pair(string("putch"), VarType::e_void));
  func_table.emplace(make_pair(string("putarray"), VarType::e_void));
  func_table.emplace(make_pair(string("starttime"), VarType::e_void));
  func_table.emplace(make_pair(string("stoptime"), VarType::e_void));
}

const string FuncManager::getVarType(const VarType& ty) const {
  switch (ty) {
    case VarType::e_int:
      return string("i32");
    case VarType::e_void:
    default:
      return string();
  }
}

const string FuncManager::getParamVarName(const string& name) const {
  // %符号命名不会和默认@符号冲突，且变量名不可能是数字
  return "%" + name;
}
#pragma endregion

#pragma region IRGen - Class

IRGenerator::IRGenerator() : symbolCore(), branchCore(), funcCore() {
  setting.setOs(cout).setIndent(0);
}

IRGenerator& IRGenerator::getInstance() {
  static IRGenerator gen;
  return gen;
}

#pragma endregion

#pragma region IRGen - lv3

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

#pragma region IRGen - lv4

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

#pragma region IRGen - lv6

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