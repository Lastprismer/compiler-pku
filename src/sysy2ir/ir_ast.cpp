#include "ir_ast.h"
#include "ir_util.h"
using namespace ir;

#pragma region CompUnitAST

void CompUnitAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "CompUnitAST {" << endl;
  func_def->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void CompUnitAST::Dump() {
  func_def->Dump();
}

#pragma endregion

#pragma region FuncDefAST

void FuncDefAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncDefAST {" << endl;
  funcType->Print(os, indent + 1);
  make_indent(os, indent + 1);
  os << "IDENT: \"" << funcName << "\"," << endl;
  block->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void FuncDefAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  funcType->Dump();
  gen.function_name = funcName;

  gen.writeFuncPrologue();
  block->Dump();
  gen.writeFuncEpilogue();
}

#pragma endregion

#pragma region FuncTypeAST

void FuncTypeAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncTypeAST: int," << endl;
}

void FuncTypeAST::Dump() {
  IRGenerator::getInstance().return_type = "i32";
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

void BlockAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  gen.writeBlockPrologue();
  stmt->Dump();
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

void StmtAST::Dump() {
  exp->Dump();
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

void ExpAST::Dump() {
  loexp->Dump();
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

void PrimaryExpAST::Dump() {
  switch (pex) {
    case Brackets:
      exp->Dump();
      break;
    case Number:
      number->Dump();
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

void NumberAST::Dump() {
  IRGenerator::getInstance().pushImm(int_const);
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

void UnaryExpAST::Dump() {
  switch (uex) {
    case Unary:
      uexp->Dump();
      uop->Dump();
      break;
    case Primary:
      prim->Dump();
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

void UnaryOPAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  switch (uop) {
    case uop_t::Pos:
      gen.writeUnaryInst(OpID::UNARY_POS);
      break;
    case uop_t::Neg:
      gen.writeUnaryInst(OpID::UNARY_NEG);
      break;
    case uop_t::Not:
      gen.writeUnaryInst(OpID::UNARY_NOT);
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

void MulExpAST::Dump() {
  if (mex == mex_t::MulOPUnary) {
    mexp->Dump();
    uexp->Dump();
    IRGenerator& gen = IRGenerator::getInstance();
    switch (mop) {
      case mop_t::Mul:
        gen.writeBinaryInst(OpID::BI_MUL);
        break;
      case mop_t::Div:
        gen.writeBinaryInst(OpID::BI_DIV);
        break;
      case mop_t::Mod:
        gen.writeBinaryInst(OpID::BI_MOD);
        break;
      default:
        assert(false);
    }
  } else {
    uexp->Dump();
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

void AddExpAST::Dump() {
  if (aex == aex_t::AddOPMul) {
    aexp->Dump();
    mexp->Dump();
    IRGenerator& gen = IRGenerator::getInstance();
    switch (aop) {
      case aop_t::Add:
        gen.writeBinaryInst(OpID::BI_ADD);
        break;
      case aop_t::Sub:
        gen.writeBinaryInst(OpID::BI_SUB);
        break;
      default:
        break;
    }
  } else {
    mexp->Dump();
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

void RelExpAST::Dump() {
  if (rex == rex_t::RelOPAdd) {
    rexp->Dump();
    aexp->Dump();
    IRGenerator& gen = IRGenerator::getInstance();
    switch (rop) {
      case rop_t::LessThan:
        gen.writeBinaryInst(OpID::LG_LT);
        break;
      case rop_t::LessEqual:
        gen.writeBinaryInst(OpID::LG_LE);
        break;
      case rop_t::GreaterThan:
        gen.writeBinaryInst(OpID::LG_GT);
        break;
      case rop_t::GreaterEqual:
        gen.writeBinaryInst(OpID::LG_GE);
        break;
      default:
        assert(false);
        break;
    }
  } else {
    aexp->Dump();
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

void EqExpAST::Dump() {
  if (eex == eex_t::EqOPRel) {
    eexp->Dump();
    rexp->Dump();
    IRGenerator& gen = IRGenerator::getInstance();
    switch (eop) {
      case eop_t::Equal:
        gen.writeBinaryInst(OpID::LG_EQ);
        break;
      case eop_t::NotEqual:
        gen.writeBinaryInst(OpID::LG_NEQ);
        break;
      default:
        break;
    }
  } else {
    rexp->Dump();
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

void LAndExpAst::Dump() {
  if (laex == laex_t::LAOPEq) {
    laexp->Dump();
    eexp->Dump();
    IRGenerator::getInstance().writeLogicInst(OpID::LG_AND);
  } else {
    eexp->Dump();
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

void LOrExpAst::Dump() {
  if (loex == loex_t::LOOPLA) {
    loexp->Dump();
    laexp->Dump();
    IRGenerator::getInstance().writeLogicInst(OpID::LG_OR);
  } else {
    laexp->Dump();
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