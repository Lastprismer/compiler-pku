#include "ast.h"
#include "util.h"

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

  loexp->Print(os, indent + 1);

  make_indent(os, indent);
  os << " }," << endl;
}

void ExpAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  loexp->Dump(os, info, indent);
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
      return "(Exp)";
    case Number:
      return "Number";
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
      return "PrimaryExp";
    case Unary:
      return "UnaryOp UnaryExp";
    default:
      assert(false);
  }
}

#pragma endregion

#pragma region UnaryOPAST

void UnaryOPAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "UnaryOPAST { " << op_name() << " } " << endl;
}

void UnaryOPAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  switch (uop) {
    case uop_t::Pos:
      info->write_unary_inst(os, OpID::UNARY_POS);
      break;
    case uop_t::Neg:
      info->write_unary_inst(os, OpID::UNARY_NEG);
      break;
    case uop_t::Not:
      info->write_unary_inst(os, OpID::UNARY_NOT);
      break;
    default:
      assert(false);
  }
}

const char* UnaryOPAST::op_name() const {
  switch (uop) {
    case uop_t::Pos:
      return "+";
    case uop_t::Neg:
      return "-";
    case uop_t::Not:
      return "!";
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
    os << "op: " << op_name() << endl;
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
    switch (mop) {
      case mop_t::Mul:
        info->write_binary_inst(os, OpID::BI_MUL);
        break;
      case mop_t::Div:
        info->write_binary_inst(os, OpID::BI_DIV);
        break;
      case mop_t::Mod:
        info->write_binary_inst(os, OpID::BI_MOD);
        break;
      default:
        assert(false);
    }
  } else {
    uexp->Dump(os, info, indent);
  }
}

const char* MulExpAST::op_name() const {
  switch (mop) {
    case mop_t::Mul:
      return "*";
    case mop_t::Div:
      return "/";
    case mop_t::Mod:
      return "%";
    default:
      assert(false);
  }
}

string MulExpAST::type() const {
  if (mex == mex_t::Unary) {
    return string("Unary");
  }
  stringstream ss;
  ss << "MulExp " << op_name() << " UnaryExp";
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
    os << "op: " << op_name() << endl;
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
    switch (aop) {
      case aop_t::Add:
        info->write_binary_inst(os, OpID::BI_ADD);
        break;
      case aop_t::Sub:
        info->write_binary_inst(os, OpID::BI_SUB);
        break;
      default:
        break;
    }
  } else {
    mexp->Dump(os, info, indent);
  }
}

const char* AddExpAST::op_name() const {
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
  ss << "AddExp " << op_name() << " MulExp";
  return ss.str();
}

#pragma endregion

#pragma region RelExpAST

void RelExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "RelExpAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << type() << endl;
  if (rex == rex_t::RelOPAdd) {
    rexp->Print(os, indent + 1);
    make_indent(os, indent);
    os << "op: " << op_name() << endl;
    aexp->Print(os, indent + 1);
  } else {
    aexp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void RelExpAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  if (rex == rex_t::RelOPAdd) {
    rexp->Dump(os, info, indent);
    aexp->Dump(os, info, indent);
    switch (rop) {
      case rop_t::LessThan:
        info->write_binary_inst(os, OpID::LG_LT);
        break;
      case rop_t::LessEqual:
        info->write_binary_inst(os, OpID::LG_LE);
        break;
      case rop_t::GreaterThan:
        info->write_binary_inst(os, OpID::LG_GT);
        break;
      case rop_t::GreaterEqual:
        info->write_binary_inst(os, OpID::LG_GE);
        break;
      default:
        assert(false);
        break;
    }
  } else {
    aexp->Dump(os, info, indent);
  }
}

const char* RelExpAST::op_name() const {
  switch (rop) {
    case rop_t::GreaterThan:
      return ">";
    case rop_t::GreaterEqual:
      return ">=";
    case rop_t::LessThan:
      return "<";
    case rop_t::LessEqual:
      return "<=";
    default:
      assert(false);
  }
}

string RelExpAST::type() const {
  if (rex == rex_t::AddExp) {
    return string("AddExp");
  }
  stringstream ss;
  ss << "RelExp " << op_name() << " AddExp";
  return ss.str();
}

#pragma endregion

#pragma region EqExpAst

void EqExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "EqExpAST {" << endl;
  make_indent(os, indent + 1);

  os << "type: " << type() << endl;

  if (eex == eex_t::EqOPRel) {
    eexp->Print(os, indent + 1);
    make_indent(os, indent);
    os << op_name() << endl;
    rexp->Print(os, indent + 1);
  } else {
    rexp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void EqExpAST::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  if (eex == eex_t::EqOPRel) {
    eexp->Dump(os, info, indent);
    rexp->Dump(os, info, indent);
    switch (eop) {
      case eop_t::Equal:
        info->write_binary_inst(os, OpID::LG_EQ);
        break;
      case eop_t::NotEqual:
        info->write_binary_inst(os, OpID::LG_NEQ);
        break;
      default:
        break;
    }
  } else {
    rexp->Dump(os, info, indent);
  }
}

const char* EqExpAST::op_name() const {
  switch (eop) {
    case eop_t::Equal:
      return "==";
    case eop_t::NotEqual:
      return "!=";
    default:
      assert(false);
  }
}

string EqExpAST::type() const {
  if (eex == eex_t::RelExp) {
    return string("RelExp");
  }
  stringstream ss;
  ss << "EqExp " << op_name() << " RelExp";
  return ss.str();
}

#pragma endregion

#pragma region LAndExpAst

void LAndExpAst::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "LAndExpAst {" << endl;
  make_indent(os, indent + 1);

  os << "type: " << type() << endl;

  if (laex == laex_t::LAOPEq) {
    laexp->Print(os, indent + 1);
    make_indent(os, indent);
    os << "&&" << endl;
    eexp->Print(os, indent + 1);
  } else {
    eexp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void LAndExpAst::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  if (laex == laex_t::LAOPEq) {
    laexp->Dump(os, info, indent);
    eexp->Dump(os, info, indent);
    info->write_logic_inst(os, OpID::LG_AND);
  } else {
    eexp->Dump(os, info, indent);
  }
}

string LAndExpAst::type() const {
  switch (laex) {
    case laex_t::EqExp:
      return string("EqEXP");

    case laex_t::LAOPEq:
    default:
      return string("LAndExp && EqExp");
  }
}

#pragma endregion

#pragma region LOrExpAst

void LOrExpAst::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "LOrExpAst {" << endl;
  make_indent(os, indent + 1);

  os << "type: " << type() << endl;

  if (loex == loex_t::LOOPLA) {
    loexp->Print(os, indent + 1);
    make_indent(os, indent);
    os << "||" << endl;
    laexp->Print(os, indent + 1);
  } else {
    laexp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void LOrExpAst::Dump(ostream& os, shared_ptr<CodeFuncInfo> info, int indent) {
  if (loex == loex_t::LOOPLA) {
    loexp->Dump(os, info, indent);
    laexp->Dump(os, info, indent);
    info->write_logic_inst(os, OpID::LG_OR);
  } else {
    laexp->Dump(os, info, indent);
  }
}

string LOrExpAst::type() const {
  switch (loex) {
    case loex_t::LAndExp:
      return string("LAndExp");

    case loex_t::LOOPLA:
    default:
      return string("LOrExp || LAndExp");
  }
}

#pragma endregion

void make_indent(ostream& os, int indent) {
  string idt(INDENT_LEN, ' ');
  for (int i = 0; i < indent; i++) {
    os << idt;
  }
}