#include "info.h"

int CodeFuncInfo::var_cnt = 0;

CodeFuncInfo::Node::Node() {}

CodeFuncInfo::Node::Node(int i) : tag(NodeTag::imm) {
  content.imm = i;
}

CodeFuncInfo::Node::Node(const Node& n) : tag(n.tag), content(n.content) {}

CodeFuncInfo::Node::Node(Node&& n) : tag(n.tag), content(n.content) {}

CodeFuncInfo::CodeFuncInfo() {
  cur_indent = 0;
  func_name = "";
  ret_type = "";
  node_stack = deque<Node>();
}

void CodeFuncInfo::write_prologue(ostream& os) {
  os << "fun @" << func_name << "(): " << ret_type << "{\n";
  cur_indent += 2;
  return;
}

void CodeFuncInfo::write_epilogue(ostream& os) {
  assert(node_stack.size() == 1);
  os << ind_sp() << "ret ";
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

void CodeFuncInfo::push_symbol(int syb) {
  Node comp;
  comp.tag = symbol;
  if (syb == -1) {
    comp.content.symbol_id = create_temp_symbol();
  }
  comp.content.symbol_id = syb;
  node_stack.push_front(comp);
}

void CodeFuncInfo::push_imm(int int_const) {
  Node comp;
  comp.tag = imm;
  comp.content.imm = int_const;
  node_stack.push_front(comp);
}

void CodeFuncInfo::write_unary_inst(ostream& os, char op) {
  if (op == '+') {
    return;
  }
  assert(node_stack.size() >= 1);
  // 只有两种运算
  // 1. a = -b，等效于 a = 0 - b，推出b，加入0，推入b，调用双目sub
  // 2. a = !b，原样处理
  if (op == '-') {
    Node node = node_stack.front();
    node_stack.pop_front();
    push_imm(0);
    node_stack.push_front(node);
    write_binary_inst(os, '-');
    return;
  }

  int new_symbol = create_temp_symbol();
  os << ind_sp() << "%" << new_symbol << " = eq 0, ";

  Node node = node_stack.front();
  node_stack.pop_front();
  parse_node(os, node);
  os << endl;
  push_symbol(new_symbol);
  return;
}

void CodeFuncInfo::write_binary_inst(ostream& os, char op) {
  assert(node_stack.size() >= 2);
  Node right = node_stack.front();
  node_stack.pop_front();
  Node left = node_stack.front();
  node_stack.pop_front();

  int new_symbol = create_temp_symbol();
  os << ind_sp() << "%" << new_symbol << " = ";
  switch (op) {
    case '+':
      os << "add";
      break;
    case '-':
      os << "sub";
      break;
    case '*':
      os << "mul";
      break;
    case '/':
      os << "div";
      break;
    case '%':
      os << "mod";
      break;
    default:
      assert(false);
  }
  os << ' ';
  parse_node(os, left);
  os << ", ";
  parse_node(os, right);
  os << endl;
  push_symbol(new_symbol);
}

int CodeFuncInfo::create_temp_symbol() {
  return var_cnt++;
}

inline string CodeFuncInfo::ind_sp() {
  return string(cur_indent, ' ');
}

void CodeFuncInfo::parse_node(ostream& os, const Node& node) {
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
