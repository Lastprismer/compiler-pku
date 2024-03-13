#include "ast.h"

#pragma region CompUnitAST

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

#pragma endregion

#pragma region FuncDefAST

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

#pragma endregion

#pragma region FuncTypeAST

void FuncTypeAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncTypeAST: int," << endl;
}

void FuncTypeAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  info->ret_type = "i32";
}

#pragma endregion

#pragma region BlockAST

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

#pragma endregion

#pragma region StmtAST

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

#pragma endregion

#pragma region ExpAST

void ExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ExpAST {" << endl;

  aexp->Print(os, indent + 1);

  make_indent(os, indent);
  os << " }," << endl;
}

void ExpAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  aexp->Dump(os, info, indent);
}

#pragma endregion

#pragma region PrimaryExpAST

void PrimaryExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "PrimaryAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << type() << endl;

  if (pex == pex_t::Brackets) {
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

const char* PrimaryExpAST::type() const {
  switch (pex) {
    case Brackets:
      return BRACKETSEXP_NAME;
    case Number:
      return NUMBER_NAME;
    default:
      assert(false);
  }
}

#pragma endregion

#pragma region NumberAST

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

#pragma endregion

#pragma region UnaryExpAST

void UnaryExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "UnaryExpAST {" << endl;
  make_indent(os, indent + 1);

  os << "type: " << type() << endl;

  if (uex == uex_t::Primary) {
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

const char* UnaryExpAST::type() const {
  switch (uex) {
    case Primary:
      return PRIMARY_NAME;
    case Unary:
      return UNARY_NAME;
    default:
      assert(false);
  }
}

#pragma endregion

#pragma region UnaryOPAST

void UnaryOPAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "UnaryOPAST { " << enum_name() << " } " << endl;
}

void UnaryOPAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  info->write_unary_inst(os, *enum_name());
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

#pragma endregion

#pragma region MulExpAST

void MulExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "MulExpAST {" << endl;
  make_indent(os, indent + 1);

  os << "type: " << type() << endl;

  if (mex == mex_t::MulOPUnary) {
    mexp->Print(os, indent + 1);
    make_indent(os, indent);
    os << "op: " << enum_name() << endl;
    uexp->Print(os, indent + 1);
  } else {
    uexp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void MulExpAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  if (mex == mex_t::MulOPUnary) {
    mexp->Dump(os, info, indent);
    uexp->Dump(os, info, indent);
    info->write_binary_inst(os, *enum_name());
  } else {
    uexp->Dump(os, info, indent);
  }
}

const char* MulExpAST::enum_name() const {
  switch (mop) {
    case mop_t::Mul:
      return MUL_NAME;
    case mop_t::Div:
      return DIV_NAME;
    case mop_t::Mod:
      return MOD_NAME;
    default:
      assert(false);
  }
}

string MulExpAST::type() const {
  if (mex == mex_t::Unary) {
    return string("Unary");
  }
  stringstream ss;
  ss << "MulExp " << enum_name() << " UnaryExp";
  return ss.str();
}

#pragma endregion

#pragma region AddExpAST

void AddExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "AddExpAST {" << endl;
  make_indent(os, indent + 1);

  os << "type: " << type() << endl;

  if (aex == aex_t::AddOPMul) {
    aexp->Print(os, indent + 1);
    make_indent(os, indent);
    os << "op: " << enum_name() << endl;
    mexp->Print(os, indent + 1);
  } else {
    mexp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void AddExpAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  if (aex == aex_t::AddOPMul) {
    aexp->Dump(os, info, indent);
    mexp->Dump(os, info, indent);
    info->write_binary_inst(os, *enum_name());
  } else {
    mexp->Dump(os, info, indent);
  }
}

const char* AddExpAST::enum_name() const {
  switch (aop) {
    case aop_t::Add:
      return "+";
    case aop_t::Sub:
      return "-";
    default:
      assert(false);
  }
}

string AddExpAST::type() const {
  if (aex == aex_t::MulExp) {
    return string("MulExp");
  }
  stringstream ss;
  ss << "AddExp " << enum_name() << " MulExp";
  return ss.str();
}

#pragma endregion

void make_indent(ostream& os, int indent) {
  string idt(INDENT_LEN, ' ');
  for (int i = 0; i < indent; i++) {
    os << idt;
  }
}
