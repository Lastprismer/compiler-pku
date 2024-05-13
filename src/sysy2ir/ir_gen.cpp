#include "ir_gen.h"

namespace ir {

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

IRGenerator::IRGenerator() : sbmanager() {
  setting.setOs(cout).setIndent(0);

  variable_pool = 0;
  function_name = "";
  return_type = "";
  function_retInfo = RetInfo();
}

IRGenerator& IRGenerator::getInstance() {
  static IRGenerator gen;
  return gen;
}

void IRGenerator::WriteFuncPrologue() {
  setting.getOs() << "fun @" << function_name << "(): " << return_type << "{\n";
  return;
}

void IRGenerator::WriteFuncEpilogue() {
  if (setting.shouldWriting) {
    ostream& os = setting.getOs();
    os << setting.getIndentStr() << "ret " << parseRetInfo(function_retInfo)
       << "\n"
       << "}" << endl;
    setting.shouldWriting = false;
  }
  return;
}

void IRGenerator::WriteBlockPrologue() {
  if (setting.shouldWriting) {
    ostream& os = setting.getOs();
    os << setting.getIndentStr()
       << "%"
          "entry:"
       << endl;
    setting.getIndent() += 2;
  }
  return;
}

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
  if (left.ty == RetInfo::RetTy::ty_int && right.ty == RetInfo::RetTy::ty_int) {
    return RetInfo(calcConstExpr(left.GetValue(), right.GetValue(), op));
  }

  const string newSymbolName = getSymbolName(registerNewSymbol());
  if (setting.shouldWriting)
    os << setting.getIndentStr() << newSymbolName << " = " << BiOp2koopa(op)
       << ' ' << parseRetInfo(left) << ", " << parseRetInfo(right) << endl;
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

void IRGenerator::WriteAllocInst(const SymbolTableEntry& entry) {
  if (!setting.shouldWriting)
    return;
  ostream& os = setting.getOs();
  os << setting.getIndentStr() << entry.GetAllocInst() << endl;
}

const RetInfo IRGenerator::WriteLoadInst(const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();
  const string newSymbolName = getSymbolName(registerNewSymbol());
  if (setting.shouldWriting)
    os << setting.getIndentStr() << entry.GetLoadInst(newSymbolName) << endl;
  return RetInfo(newSymbolName);
}

void IRGenerator::WriteStoreInst(const RetInfo& value,
                                 const SymbolTableEntry& entry) {
  if (!setting.shouldWriting)
    return;
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

int IRGenerator::registerNewSymbol() {
  return variable_pool++;
}

string IRGenerator::getSymbolName(const int& symbol) const {
  return string("%") + to_string(symbol);
}

const string IRGenerator::parseRetInfo(const RetInfo& info) const {
  stringstream ss;
  switch (info.ty) {
    case RetInfo::RetTy::ty_int:
      ss << info.GetValue();
      break;
    case RetInfo::RetTy::ty_sbl:
      ss << info.GetSym();
      break;
    default:
      cerr << "void is not supported";
      assert(false);
      break;
  }
  return ss.str();
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
}  // namespace ir