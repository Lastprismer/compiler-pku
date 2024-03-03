#include "ast.h"

void CompUnitAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "CompUnitAST {" << endl;
  func_def->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void CompUnitAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  // 这里info应该是nullptr
  func_def->Dump(os, info, indent + 1);
}

void FuncDefAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncDefAST {" << endl;
  func_type->Print(os, indent + 1);
  make_indent(os, indent + 1);
  os << "IDENT: \"" << ident << "\"," << endl;
  block->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void FuncDefAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  func_info = make_shared<CodeFuncInfo>();
  func_info->func_name = ident;
  func_type->Dump(os, func_info, indent + 1);
  func_info->write_prologue(os);
  block->Dump(os, func_info, indent);
  func_info->write_epilogue(os);
}

void FuncTypeAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncTypeAST: int," << endl;
}

void FuncTypeAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  info->ret_type = "i32";
}

void BlockAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "BlockAST {" << endl;
  stmt->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void BlockAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  make_indent(os, indent);
  os << "%"
     << "entry:" << endl;
  stmt->Dump(os, info, indent + 1);
}

void StmtAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "StmtAST {" << endl;
  exp->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void StmtAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  exp->Dump(os, info, indent);
}

void ExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ExpAST {" << endl;

  uexp->Print(os, indent + 1);

  make_indent(os, indent);
  os << " }," << endl;
}

void ExpAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  uexp->Dump(os, info, indent);
}

void PrimaryExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "PrimaryAST {" << endl;
  make_indent(os, indent + 1);
  const char* type = enum_name();
  os << "type: " << type << endl;

  if (strcmp(type, BRACKETSEXP_NAME) == 0) {
    exp->Print(os, indent + 1);
  } else {
    number->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void PrimaryExpAST::Dump(ostream& os,
                         shared_ptr<CodeFuncInfo> info,
                         int indent) {
  switch (pex) {
    case Brackets:
      exp->Dump(os, info, indent);
      break;
    case Number:
      number->Dump(os, info, indent);
      break;
    default:
      assert(false);
  }
}

const char* PrimaryExpAST::enum_name() const {
  switch (pex) {
    case Brackets:
      return BRACKETSEXP_NAME;
    case Number:
      return NUMBER_NAME;
    default:
      assert(false);
  }
}

void NumberAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "NumberAST { " << endl;
  make_indent(os, indent + 1);
  os << "int_const: " << int_const << endl;
  make_indent(os, indent);
  os << " }," << endl;
}

void NumberAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  info->push_imm(int_const);
}

void UnaryExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "UnaryExpAST {" << endl;
  make_indent(os, indent + 1);

  const char* type = enum_name();
  os << "type: " << type << endl;

  if (strcmp(type, PRIMARY_NAME) == 0) {
    prim->Print(os, indent + 1);
  } else {
    uop->Print(os, indent + 1);
    uexp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void UnaryExpAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  switch (uex) {
    case Unary:
      uexp->Dump(os, info, indent);
      uop->Dump(os, info, indent);
      break;
    case Primary:
      prim->Dump(os, info, indent);
      break;
    default:
      assert(false);
      break;
  }
}

const char* UnaryExpAST::enum_name() const {
  switch (uex) {
    case Primary:
      return PRIMARY_NAME;
    case Unary:
      return UNARY_NAME;
    default:
      assert(false);
  }
}

void UnaryOPAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "UnaryOPAST { " << enum_name() << " } " << endl;
}

void UnaryOPAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  info->write_inst(os, *enum_name());
}

const char* UnaryOPAST::enum_name() const {
  switch (uop) {
    case uop_t::Pos:
      return POS_NAME;
    case uop_t::Neg:
      return NEG_NAME;
    case uop_t::Not:
      return NOT_NAME;
    default:
      assert(false);
  }
}

void make_indent(ostream& os, int indent) {
  string idt(INDENT_LEN, ' ');
  for (int i = 0; i < indent; i++) {
    os << idt;
  }
}
