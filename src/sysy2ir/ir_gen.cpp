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
    case RetInfo::retty_t::ty_int:
      return to_string(GetValue());
    case RetInfo::retty_t::ty_sbl:
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

#pragma region IRGen - Class

IRGenerator::IRGenerator() : symbolman() {
  setting.setOs(cout).setIndent(0);

  symbolPool = 0;
  hasRetThisBB = false;
  funcName = "";
  returnType = "";
  funcRetInfo = RetInfo();
}

IRGenerator& IRGenerator::getInstance() {
  static IRGenerator gen;
  return gen;
}

#pragma endregion

#pragma region IRGen - lv1

void IRGenerator::WriteFuncPrologue() {
  setting.getOs() << "fun @" << funcName << "(): " << returnType << " {\n"
                  << setting.getIndentStr()
                  << "%"
                     "entry:"
                  << endl;
  setting.getIndent() += 2;
  return;
}

void IRGenerator::WriteFuncEpilogue() {
  ostream& os = setting.getOs();
  os << "}" << endl;
  return;
}

void IRGenerator::WriteRetInst() {
  ostream& os = setting.getOs();
  os << setting.getIndentStr() << "ret " << funcRetInfo.GetInfo() << endl;
  hasRetThisBB = true;
}

#pragma endregion

#pragma region IRGen - lv3

const RetInfo IRGenerator::WriteUnaryInst(const RetInfo& left, OpID op) {
  if (op == OpID::UNARY_POS) {
    return left;
  }
  // 只有两种运算
  // 1. a = -b，等效于 a = 0 - b，推出b，加入0，推入b，调用sub
  // 2. a = !b，等效于a = 0 == b，推出b，加入0，推入b，调用eq
  if (op == OpID::UNARY_NEG) {
    return WriteBinaryInst(RetInfo(0), left, OpID::BI_SUB);
  } else if (op == OpID::UNARY_NOT) {
    return WriteBinaryInst(RetInfo(0), left, OpID::LG_EQ);
  } else {
    assert(false);
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

  const string newSymbolName = getSymbolName(registerNewSymbol());
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
  const string newSymbolName = getSymbolName(registerNewSymbol());
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
      info.then_label = registerNewBB();
      info.next_label = registerNewBB();
      os << setting.getIndentStr() << "br " << cond.GetInfo() << ", "
         << getLabelName(info.then_label) << ", "
         << getLabelName(info.next_label) << endl;
      break;
    case IfInfo::ifty_t::ie:
    default:
      info.then_label = registerNewBB();
      info.else_label = registerNewBB();
      info.next_label = registerNewBB();
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
  hasRetThisBB = false;
}

const string IRGenerator::registerShortCircuitVar() {
  return string("if_") + to_string(registerNewVar());
}

void IRGenerator::InitLoopInfo(LoopInfo& info) {
  info.cond_label = registerNewBB();
}

void IRGenerator::WriteBrInst(const RetInfo& cond, LoopInfo& loopInfo) {
  ostream& os = getInstance().setting.getOs();
  loopInfo.body_label = registerNewBB();
  loopInfo.next_label = registerNewBB();
  os << setting.getIndentStr() << "br " << cond.GetInfo() << ", "
     << getLabelName(loopInfo.body_label) << ", "
     << getLabelName(loopInfo.next_label) << endl;
}

const int IRGenerator::registerNewSymbol() {
  return symbolPool++;
}

const int IRGenerator::registerNewBB() {
  return bbPool++;
}

const int IRGenerator::registerNewVar() {
  return symbolman.dproc.RegisterVar();
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
      assert(false);
  }
  return 0;
}

#pragma endregion

}  // namespace ir