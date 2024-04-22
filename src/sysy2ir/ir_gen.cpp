#include "ir_gen.h"

namespace ir {

Node::Node() {}

Node::Node(int i) : tag(NodeTag::imm) {
  content.imm = i;
}

Node::Node(const Node& n) : tag(n.tag), content(n.content) {}

Node::Node(Node&& n) : tag(n.tag), content(n.content) {}

IRGenerator::IRGenerator() {
  setting.setOs(cout).setIndent(0);

  variable_pool = 0;
  function_name = "";
  return_type = "";
  node_stack = deque<Node>();
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
  assert(node_stack.size() == 1);
  os << setting.getIndentStr() << "ret ";
  Node node = node_stack.front();
  if (node.tag == imm) {
    os << node.content.imm;
  } else if (node.tag == symbol) {
    os << "%" << node.content.symbol_id;
  } else {
    assert(false);
  }
  os << "\n"
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
  comp.tag = symbol;
  if (syb == -1) {
    comp.content.symbol_id = registerNewSymbol();
  }
  comp.content.symbol_id = syb;
  node_stack.push_front(comp);
}

void IRGenerator::pushImm(int int_const) {
  Node comp;
  comp.tag = imm;
  comp.content.imm = int_const;
  node_stack.push_front(comp);
}

void IRGenerator::writeUnaryInst(OpID op) {
  if (op == OpID::UNARY_POS) {
    return;
  }
  assert(node_stack.size() >= 1);
  // 只有两种运算
  // 1. a = -b，等效于 a = 0 - b，推出b，加入0，推入b，调用sub
  // 2. a = !b，等效于a = 0 == b，推出b，加入0，推入b，调用eq
  if (op == OpID::UNARY_NEG) {
    Node node = node_stack.front();
    node_stack.pop_front();
    pushImm(0);
    node_stack.push_front(node);
    writeBinaryInst(OpID::BI_SUB);
    return;
  } else if (op == OpID::UNARY_NOT) {
    Node node = node_stack.front();
    node_stack.pop_front();
    pushImm(0);
    node_stack.push_front(node);
    writeBinaryInst(OpID::LG_EQ);
    return;
  } else {
    assert(false);
  }
}

void IRGenerator::writeBinaryInst(OpID op) {
  assert(node_stack.size() >= 2);
  ostream& os = setting.getOs();
  Node right = node_stack.front();
  node_stack.pop_front();
  Node left = node_stack.front();
  node_stack.pop_front();

  int new_symbol = registerNewSymbol();
  os << setting.getIndentStr() << "%" << new_symbol << " = ";
  os << BiOp2koopa(op) << ' ';
  parseNode(left);
  os << ", ";
  parseNode(right);
  os << endl;
  pushSymbol(new_symbol);
}

void IRGenerator::writeLogicInst(OpID op) {
  assert(node_stack.size() >= 2);
  assert(op == OpID::LG_AND || op == OpID::LG_OR);

  Node right = node_stack.front();
  node_stack.pop_front();

  // left -> bool
  pushImm(0);
  writeBinaryInst(OpID::LG_NEQ);

  // right -> bool
  node_stack.push_front(right);
  pushImm(0);
  writeBinaryInst(OpID::LG_NEQ);

  // logic
  writeBinaryInst(op);
}

int IRGenerator::registerNewSymbol() {
  return variable_pool++;
}

void IRGenerator::parseNode(const Node& node) {
  ostream& os = setting.getOs();
  switch (node.tag) {
    case NodeTag::imm:
      os << node.content.imm;
      break;
    case NodeTag::symbol:
      os << "%" << node.content.symbol_id;
      break;
    default:
      os << node.tag;
      assert(false);
      break;
  }
}

}