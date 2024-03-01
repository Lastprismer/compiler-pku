#include "info.h"

int FuncInfo::var_cnt = 0;

FuncInfo::FuncInfo() {
  cur_indent = 0;
  func_name = "";
  ret_type = "";
  node_stack = queue<comp_t>();
}

void FuncInfo::write_prologue(ostream& os) {
  os << "fun @" << func_name << "(): " << ret_type << "{\n";
  cur_indent += 2;
  return;
}

void FuncInfo::write_epilogue(ostream& os) {
  assert(node_stack.size() == 1);
  os << ind_sp() << "ret ";
  comp_t comp = node_stack.front();
  if (comp.tag == imm) {
    os << comp.content.imm;
  } else {
    os << "%" << comp.content.symbol_id;
  }
  os << "\n"
     << "}" << endl;
  return;
}

void FuncInfo::push_symbol(int syb) {
  comp_t comp;
  comp.tag = symbol;
  if (syb == -1) {
    comp.content.symbol_id = create_temp_symbol();
  }
  comp.content.symbol_id = syb;
  node_stack.push(comp);
}

void FuncInfo::push_imm(int int_const) {
  comp_t comp;
  comp.tag = imm;
  comp.content.imm = int_const;
  node_stack.push(comp);
}

void FuncInfo::write_inst(ostream& os, char op) {
  if (op == '+') {
    return;
  }
  assert(node_stack.size() >= 1);
  int new_symbol = create_temp_symbol();
  os << ind_sp() << "%" << new_symbol << " = ";

  if (op == '-') {
    os << "sub 0, ";
  } else if (op == '!') {
    os << "eq 0, ";
  }

  comp_t comp = node_stack.front();
  node_stack.pop();
  if (comp.tag == uexpc_t::imm) {
    os << comp.content.imm << endl;
  } else {
    os << "%" << comp.content.symbol_id << endl;
  }
  push_symbol(new_symbol);
  return;
}

int FuncInfo::create_temp_symbol() {
  return var_cnt++;
}

inline string FuncInfo::ind_sp() {
  return string(cur_indent, ' ');
}
