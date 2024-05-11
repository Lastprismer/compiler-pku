#include "ir_gen.h"

namespace ir {

RetInfo::RetInfo() : ty(ty_void) {}

RetInfo::RetInfo(int _value) : ty(ty_int), value(_value) {}

RetInfo::RetInfo(string _symbol) : ty(ty_sbl), name(_symbol) {}

RetInfo::RetInfo(int _, string _var) : ty(ty_var), name(_var) {}

const int& RetInfo::GetValue() const {
  assert(ty == ty_int);
  return value;
}

const string& RetInfo::GetSym() const {
  assert(ty == ty_sbl);
  return name;
}

const string& RetInfo::GetVar() const {
  assert(ty == ty_var);
  return name;
}

Node::Node() {}

Node::Node(int i) : tag(NodeTag::IMM), imm(i) {}

Node::Node(const Node& n) : tag(n.tag) {
  if (tag == NodeTag::IMM) {
    imm = n.imm;
  } else if (tag == NodeTag::SYMBOL) {
    symbol_name = n.symbol_name;
  }
}

Node::Node(Node&& n) : tag(n.tag) {
  if (tag == NodeTag::IMM) {
    imm = n.imm;
  } else if (tag == NodeTag::SYMBOL) {
    symbol_name = n.symbol_name;
  }
}

IRGenerator::IRGenerator() {
  setting.setOs(cout).setIndent(0);

  variable_pool = 0;
  function_name = "";
  return_type = "";
  node_stack = deque<Node>();
  symbol_table = SymbolTable();
  FunctionRetInfo = RetInfo();
}

IRGenerator& IRGenerator::getInstance() {
  static IRGenerator gen;
  return gen;
}

void IRGenerator::writeFuncPrologue() {
  setting.getOs() << "fun @" << function_name << "(): " << return_type << "{\n";
  return;
}

void IRGenerator::writeFuncEpilogue() {
  ostream& os = setting.getOs();
  os << setting.getIndentStr() << "ret " << parseRetInfo(FunctionRetInfo)
     << "\n"
     << "}" << endl;
  return;
}

void IRGenerator::writeBlockPrologue() {
  ostream& os = setting.getOs();
  os << setting.getIndentStr()
     << "%"
        "entry:"
     << endl;
  setting.getIndent() += 2;
  return;
}

void IRGenerator::pushSymbol(int syb) {
  Node comp;
  comp.tag = NodeTag::SYMBOL;
  if (syb == -1) {
    comp.symbol_name = getSymbolName(registerNewSymbol());
  }
  comp.symbol_name = getSymbolName(syb);
  node_stack.push_front(comp);
}

void IRGenerator::pushImm(int int_const) {
  Node comp;
  comp.tag = NodeTag::IMM;
  comp.imm = int_const;
  node_stack.push_front(comp);
}

const Node& IRGenerator::checkFrontNode() const {
  return node_stack.front();
}

Node IRGenerator::getFrontNode() {
  Node node = node_stack.front();
  node_stack.pop_front();
  return node;
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

void IRGenerator::writeAllocInst(const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();
  os << setting.getIndentStr() << entry.GetAllocInst() << endl;
}

void IRGenerator::writeLoadInst(const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();
  // 申请新符号
  int new_symbol = registerNewSymbol();
  os << setting.getIndentStr() << entry.GetLoadInst(getSymbolName(new_symbol))
     << endl;
  // 推入节点
  pushSymbol(new_symbol);
}

void IRGenerator::writeStoreInst(const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();
  // 弹出栈顶节点
  const Node& node = getFrontNode();

  os << setting.getIndentStr();
  if (node.tag == NodeTag::IMM) {
    // 常量赋值
    os << entry.GetStoreInst(node.imm) << endl;
  } else if (node.tag == NodeTag::SYMBOL) {
    os << entry.GetStoreInst(node.symbol_name) << endl;
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