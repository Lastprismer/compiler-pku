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

#pragma region DeclAST
void DeclAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "DeclAST {" << endl;
  make_indent(os, indent + 1);
  if (de == de_t::CONST) {
    os << "type: const" << endl;
  } else {
    os << "type: var" << endl;
  }
  decl->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void DeclAST::Dump() {
  decl->Dump();
}

#pragma endregion

#pragma region ConstDeclAST

void ConstDeclAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ConstDeclAST {" << endl;
  btype->Print(os, indent + 1);
  for (auto it = const_defs.begin(); it != const_defs.end(); it++) {
    (*it)->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << " }," << endl;
}

void ConstDeclAST::Dump() {
  DeclaimProcessor& processor =
      IRGenerator::getInstance().symbol_table.getDProc();
  processor.Enable();
  processor.SetSymbolType(SymbolType::CONST);
  btype->Dump();
  for (auto it = const_defs.begin(); it != const_defs.end(); it++) {
    (*it)->Dump();
  }
  processor.Disable();
}

#pragma endregion

#pragma region ConstDeclListUnit

void ConstDeclListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD OUTPUT THIS]" << endl;
}

void ConstDeclListUnit::Dump() {
  cerr << "[SHOULD OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region BTypeAST

void BTypeAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "BTypeAST: int" << endl;
}

void BTypeAST::Dump() {
  IRGenerator::getInstance().symbol_table.dproc.SetVarType(VarType::INT);
}

#pragma endregion

#pragma region ConstDefAST

void ConstDefAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ConstDefAST: { var_name: " << var_name << endl;
  const_init_val->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void ConstDefAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  const_init_val->Dump();
  // 此时栈顶元素应为表达式的值，弹出并检查
  assert(gen.node_stack.size() >= 1);
  const Node node = gen.getFrontNode();
  assert(node.tag == NodeTag::IMM);
  // 取值加入符号表
  DeclaimProcessor& pcs = gen.symbol_table.getDProc();
  SymbolTableEntry entry = pcs.GenerateConstEntry(var_name, node.imm);
  gen.symbol_table.insertEntry(entry);
}

#pragma endregion

#pragma region ConstInitValAST

void ConstInitValAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ConstInitValAST: {" << endl;
  const_exp->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void ConstInitValAST::Dump() {
  const_exp->Dump();
}

#pragma endregion

#pragma region VarDeclAST
void VarDeclAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "VarDeclAST {" << endl;
  btype->Print(os, indent + 1);
  for (auto it = var_defs.begin(); it != var_defs.end(); it++) {
    (*it)->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << " }," << endl;
}

void VarDeclAST::Dump() {
  DeclaimProcessor& processor =
      IRGenerator::getInstance().symbol_table.getDProc();
  processor.Enable();
  processor.SetSymbolType(SymbolType::VAR);
  btype->Dump();
  for (auto it = var_defs.begin(); it != var_defs.end(); it++) {
    (*it)->Dump();
  }
  processor.Disable();
}

#pragma endregion

#pragma region VarDeclListUnit

void VarDeclListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD OUTPUT THIS]" << endl;
}

void VarDeclListUnit::Dump() {
  cerr << "[SHOULD OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region VarDefAST

void VarDefAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "VarDefAST: { var_name: " << var_name << endl;
  make_indent(os, indent + 1);
  os << "declaim with value: " << (init_with_val ? "true" : "false") << endl;
  if (init_with_val) {
    init_val->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << " }," << endl;
}

void VarDefAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  DeclaimProcessor& pcs = gen.symbol_table.getDProc();

  SymbolTableEntry entry = pcs.GenerateVarEntry(var_name);
  gen.writeAllocInst(entry);
  gen.symbol_table.insertEntry(entry);
  // 如果有初始化
  if (init_with_val) {
    // 类似常数的定义
    init_val->Dump();
    // 赋值，但栈顶元素不用手动弹出，在write里
    SymbolTableEntry s_entry = gen.symbol_table.getEntry(entry.var_name);
    gen.writeStoreInst(s_entry);
  }
}

#pragma endregion

#pragma region InitValAST

void InitValAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "InitValAST: {" << endl;
  exp->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void InitValAST::Dump() {
  exp->Dump();
}

#pragma endregion

#pragma region FuncDefAST

void FuncDefAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncDefAST {" << endl;
  func_type->Print(os, indent + 1);
  make_indent(os, indent + 1);
  os << "func name: \"" << func_name << "\"," << endl;
  block->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void FuncDefAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  func_type->Dump();
  gen.function_name = func_name;

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
  for (auto it = block_items.begin(); it != block_items.end(); it++) {
    (*it)->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << " }," << endl;
}

void BlockAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  gen.writeBlockPrologue();
  for (auto it = block_items.begin(); it != block_items.end(); it++) {
    (*it)->Dump();
  }
}

#pragma endregion

#pragma region BlockListUnit
void BlockListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD OUTPUT THIS]" << endl;
}

void BlockListUnit::Dump() {
  cerr << "[SHOULD OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region BlockItem
void BlockItemAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "BlockItemAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << (bt == blocktype_t::DECL ? "decl" : "stmt") << endl;
  content->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void BlockItemAST::Dump() {
  content->Dump();
}

#pragma endregion

#pragma region StmtAST
void StmtAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "StmtAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << (st == stmttype_t::CALC_LVAL ? "calculate lval" : "return")
     << endl;
  if (st == CALC_LVAL) {
    lval->Print(os, indent + 1);
    make_indent(os, indent + 1);
    os << "=" << endl;
    exp->Print(os, indent + 1);
  } else {
    exp->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << " }," << endl;
}

void StmtAST::Dump() {
  if (st == stmttype_t::CALC_LVAL) {
    IRGenerator& gen = IRGenerator::getInstance();
    AssignmentProcessor& aproc = gen.symbol_table.getAProc();
    aproc.Enable();
    lval->Dump();
    aproc.Disable();
    exp->Dump();
    // 此时栈顶为需要的值，赋值
    gen.writeStoreInst(gen.symbol_table.getEntry(aproc.GetCurrentVar()));
  } else {
    exp->Dump();
  }
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

#pragma region LValAST
void LValAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "LValAST { var name: \"" << var_name << "\" }," << endl;
}

void LValAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  // 判断变量类型
  SymbolTableEntry& entry = gen.symbol_table.getEntry(var_name);
  if (entry.symbol_type == SymbolType::CONST) {
    if (entry.var_type == VarType::INT) {
      // const int
      gen.pushImm(entry.value);
    } else {
      // const arr
      assert(false);
    }
  } else {
    if (entry.var_type == VarType::INT) {
      // var int
      // 判断是左值还是右值
      if (gen.symbol_table.aproc.IsEnabled()) {
        // 为左值，设置aproc处理当前符号
        AssignmentProcessor& aproc = gen.symbol_table.getAProc();
        aproc.SetCurrentVar(var_name);
      } else {
        // 为右值，将其加载并入栈
        gen.writeLoadInst(entry);
      }

    } else {
      // var arr
      assert(false);
    }
  }
}

#pragma endregion

#pragma region PrimaryExpAST

void PrimaryExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "PrimaryAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << type() << endl;
  content->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void PrimaryExpAST::Dump() {
  content->Dump();
}

const char* PrimaryExpAST::type() const {
  switch (pt) {
    case Brackets:
      return "(Exp)";
    case Number:
      return "Number";
    case LVal:
      return "LVal";
    default:
      assert(false);
  }
}

#pragma endregion

#pragma region NumberAST

void NumberAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "NumberAST { int_const: " << int_const << " }," << endl;
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
    make_indent(os, indent + 1);
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
    make_indent(os, indent + 1);
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
    make_indent(os, indent + 1);
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
    make_indent(os, indent + 1);
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

#pragma region LAndExpAST

void LAndExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "LAndExpAST {" << endl;
  make_indent(os, indent + 1);

  os << "type: " << type() << endl;

  if (laex == laex_t::LAOPEq) {
    laexp->Print(os, indent + 1);
    make_indent(os, indent + 1);
    os << "&&" << endl;
    eexp->Print(os, indent + 1);
  } else {
    eexp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void LAndExpAST::Dump() {
  if (laex == laex_t::LAOPEq) {
    laexp->Dump();
    eexp->Dump();
    IRGenerator::getInstance().writeLogicInst(OpID::LG_AND);
  } else {
    eexp->Dump();
  }
}

string LAndExpAST::type() const {
  switch (laex) {
    case laex_t::EqExp:
      return string("EqEXP");

    case laex_t::LAOPEq:
    default:
      return string("LAndExp && EqExp");
  }
}

#pragma endregion

#pragma region LOrExpAST

void LOrExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "LOrExpAST {" << endl;
  make_indent(os, indent + 1);

  os << "type: " << type() << endl;

  if (loex == loex_t::LOOPLA) {
    loexp->Print(os, indent + 1);
    make_indent(os, indent + 1);
    os << "||" << endl;
    laexp->Print(os, indent + 1);
  } else {
    laexp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void LOrExpAST::Dump() {
  if (loex == loex_t::LOOPLA) {
    loexp->Dump();
    laexp->Dump();
    IRGenerator::getInstance().writeLogicInst(OpID::LG_OR);
  } else {
    laexp->Dump();
  }
}

string LOrExpAST::type() const {
  switch (loex) {
    case loex_t::LAndExp:
      return string("LAndExp");

    case loex_t::LOOPLA:
    default:
      return string("LOrExp || LAndExp");
  }
}

#pragma endregion

#pragma region ConstExpAST

void ConstExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ConstExpAST: {" << endl;
  exp->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void ConstExpAST::Dump() {
  exp->Dump();
}

#pragma endregion

void make_indent(ostream& os, int indent) {
  string idt(INDENT_LEN, ' ');
  for (int i = 0; i < indent; i++) {
    os << idt;
  }
}