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
  DeclaimProcessor& processor = IRGenerator::getInstance().symbolman.getDProc();
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
  IRGenerator::getInstance().symbolman.dproc.SetVarType(VarType::INT);
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
  // 计算常数表达式
  const_init_val->Dump();
  auto cv = dynamic_cast<ConstInitValAST*>(const_init_val.get());

  // 取值加入符号表
  DeclaimProcessor& pcs = gen.symbolman.getDProc();
  SymbolTableEntry entry =
      pcs.GenerateConstEntry(var_name, cv->thisRet.GetValue());
  gen.symbolman.InsertEntry(entry);
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
  auto ce = dynamic_cast<ConstExpAST*>(const_exp.get());
  thisRet = ce->thisRet;
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
  DeclaimProcessor& processor = IRGenerator::getInstance().symbolman.getDProc();
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
  DeclaimProcessor& pcs = gen.symbolman.getDProc();

  SymbolTableEntry entry = pcs.GenerateVarEntry(var_name);
  gen.WriteAllocInst(entry);
  gen.symbolman.InsertEntry(entry);
  // 如果有初始化
  if (init_with_val) {
    // 类似常数的定义
    init_val->Dump();
    auto iv = dynamic_cast<InitValAST*>(init_val.get());
    // 赋值，加入符号表
    SymbolTableEntry s_entry = gen.symbolman.getEntry(entry.VarName);
    gen.WriteStoreInst(iv->thisRet, s_entry);
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
  auto ae = dynamic_cast<ExpAST*>(exp.get());
  thisRet = ae->thisRet;
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
  gen.funcName = func_name;

  gen.WriteFuncPrologue();
  block->Dump();
  gen.WriteFuncEpilogue();
}

#pragma endregion

#pragma region FuncTypeAST

void FuncTypeAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncTypeAST: int," << endl;
}

void FuncTypeAST::Dump() {
  IRGenerator::getInstance().returnType = "i32";
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
  os << "type: " << (bt == blocktype_t::decl ? "decl" : "stmt") << endl;
  content->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void BlockItemAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  if (!gen.hasRetThisBB) {
    content->Dump();
  }
}

#pragma endregion

#pragma region StmtAST

void StmtAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "StmtAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << (type == stmty_t::open ? "open" : "closed") << endl;
  stmt->Print(os, indent + 1);
  make_indent(os, indent);
  os << " }," << endl;
}

void StmtAST::Dump() {
  stmt->Dump();
}

#pragma endregion

#pragma region OpenStmtAST

void OpenStmtAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "OpenStmtAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: ";
  switch (type) {
    case opty_t::io:
      os << "if (Exp) Open" << endl;
      make_indent(os, indent + 1);
      os << "IF" << endl;
      exp->Print(os, indent + 1);
      open->Print(os, indent + 1);
      break;
    case opty_t::ic:
      os << "if (Exp) Closed" << endl;
      make_indent(os, indent + 1);
      os << "IF" << endl;
      exp->Print(os, indent + 1);
      closed->Print(os, indent + 1);
      break;
    case opty_t::iceo:
      os << "if (Exp) Closed else Open" << endl;
      make_indent(os, indent + 1);
      os << "IF" << endl;
      exp->Print(os, indent + 1);
      closed->Print(os, indent + 1);
      make_indent(os, indent + 1);
      os << "ELSE" << endl;
      open->Print(os, indent + 1);
      break;
  }
  make_indent(os, indent);
  os << " }," << endl;
}

void OpenStmtAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();

  exp->Dump();
  auto cond = dynamic_cast<ExpAST*>(exp.get());
  RetInfo ret = cond->thisRet;
  IfInfo ifin;

  switch (type) {
    case io:
    case ic:
      ifin = IfInfo(IfInfo::ifty_t::i);
      gen.WriteBrInst(ret, ifin);
      gen.WriteLabel(ifin.then_label);
      if (type == io) {
        open->Dump();
      } else {
        closed->Dump();
      }
      if (!gen.hasRetThisBB) {
        gen.WriteJumpInst(ifin.next_label);
      }
      gen.WriteLabel(ifin.next_label);
      break;

    case iceo:
    default: {
      ifin = IfInfo(IfInfo::ifty_t::ie);
      gen.WriteBrInst(ret, ifin);

      gen.WriteLabel(ifin.then_label);
      closed->Dump();
      bool retInThen = gen.hasRetThisBB;
      if (!retInThen) {
        gen.WriteJumpInst(ifin.next_label);
      }

      gen.WriteLabel(ifin.else_label);
      open->Dump();
      bool retInElse = gen.hasRetThisBB;
      if (!gen.hasRetThisBB) {
        gen.WriteJumpInst(ifin.next_label);
      }

      if (!(retInElse && retInThen)) {
        gen.WriteLabel(ifin.next_label);
      }
    } break;
  }
}

#pragma endregion

#pragma region ClosedStmtAST

void ClosedStmtAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ClosedStmtAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: ";
  switch (type) {
    case csty_t::simp:
      os << "simple" << endl;
      simple->Print(os, indent + 1);
      break;
    case csty_t::icec:
      os << "if (Exp) Closed else Closed" << endl;
      make_indent(os, indent + 1);
      os << "IF" << endl;
      exp->Print(os, indent + 1);
      tclosed->Print(os, indent + 1);
      make_indent(os, indent + 1);
      os << "ELSE" << endl;
      fclosed->Print(os, indent + 1);
      break;
  }
  make_indent(os, indent);
  os << " }," << endl;
}

void ClosedStmtAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();

  switch (type) {
    case simp:
      simple->Dump();
      break;
    case icec:
    default: {
      exp->Dump();
      auto cond = dynamic_cast<ExpAST*>(exp.get());
      RetInfo ret = cond->thisRet;
      IfInfo ifin(IfInfo::ifty_t::ie);
      gen.WriteBrInst(ret, ifin);

      gen.WriteLabel(ifin.then_label);
      tclosed->Dump();
      bool retInThen = gen.hasRetThisBB;
      if (!retInThen) {
        gen.WriteJumpInst(ifin.next_label);
      }

      gen.WriteLabel(ifin.else_label);
      fclosed->Dump();
      bool retInElse = gen.hasRetThisBB;
      if (!gen.hasRetThisBB) {
        gen.WriteJumpInst(ifin.next_label);
      }

      if (!(retInElse && retInThen)) {
        gen.WriteLabel(ifin.next_label);
      }
    } break;
  }
}

#pragma endregion

#pragma region SimpleStmtAST
void SimpleStmtAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "SimpleStmtAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: ";
  switch (st) {
    case sstmt_t::storelval:
      os << "calculate lval" << endl;
      lval->Print(os, indent + 1);
      make_indent(os, indent + 1);
      os << "=" << endl;
      exp->Print(os, indent + 1);
      break;
    case sstmt_t::ret:
      os << "return" << endl;
      exp->Print(os, indent + 1);
      break;
    case sstmt_t::block:
      os << "block" << endl;
      blk->Print(os, indent + 1);
      break;
    case sstmt_t::expr:
      os << "expr" << endl;
      exp->Print(os, indent + 1);
      break;
    case sstmt_t::nullexp:
      os << "null exp" << endl;
      break;
    case sstmt_t::nullret:
      os << "return void" << endl;
      break;
  }
  make_indent(os, indent);
  os << " }," << endl;
}

void SimpleStmtAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  if (st == sstmt_t::storelval) {
    AssignmentProcessor& aproc = gen.symbolman.getAProc();
    // 记录左值
    aproc.Enable();
    lval->Dump();
    aproc.Disable();

    // 计算表达式
    exp->Dump();
    auto ee = dynamic_cast<ExpAST*>(exp.get());

    // 赋值
    gen.WriteStoreInst(ee->thisRet,
                       gen.symbolman.getEntry(aproc.GetCurrentVar()));
  } else if (st == sstmt_t::ret) {
    exp->Dump();
    auto ee = dynamic_cast<ExpAST*>(exp.get());
    // 设置返回值
    gen.funcRetInfo = ee->thisRet;
    gen.WriteRetInst();
  } else if (st == sstmt_t::expr) {
    // 不记录值
    exp->Dump();
  } else if (st == block) {
    gen.symbolman.PushScope();
    blk->Dump();
    gen.symbolman.PopScope();
  } else if (st == sstmt_t::nullexp) {
  } else if (st == sstmt_t::nullret) {
    gen.funcRetInfo = RetInfo();
    gen.WriteRetInst();
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
  auto le = dynamic_cast<LOrExpAST*>(loexp.get());
  thisRet = le->thisRet;
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
  SymbolTableEntry entry = gen.symbolman.getEntry(var_name);

  if (entry.symbol_type == SymbolType::CONST) {
    // const只会是右值
    if (entry.var_type == VarType::INT) {
      // const int
      thisRet = RetInfo(entry.value);
    } else {
      // const arr
      assert(false);
    }
  } else {
    if (entry.var_type == VarType::INT) {
      // var int
      // 判断是左值还是右值
      if (gen.symbolman.aproc.IsEnabled()) {
        // 为左值，设置aproc处理当前符号
        AssignmentProcessor& aproc = gen.symbolman.getAProc();
        aproc.SetCurrentVar(var_name);
      } else {
        // 为右值，获取其临时符号
        thisRet = gen.WriteLoadInst(entry);
      }
    } else {
      // var arr
      // 不支持
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

  switch (pt) {
    case primary_exp_type_t::Brackets: {
      auto ee = dynamic_cast<ExpAST*>(content.get());
      thisRet = ee->thisRet;
    } break;
    case primary_exp_type_t::LVal: {
      // 右值
      auto lv = dynamic_cast<LValAST*>(content.get());
      thisRet = lv->thisRet;
    } break;
    case primary_exp_type_t::Number: {
      auto nb = dynamic_cast<NumberAST*>(content.get());
      thisRet = RetInfo(nb->int_const);
    } break;

    default:
      break;
  }
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
  // do nothing, waiting for PrimaryExpAST to actively fetch imm
}

#pragma endregion

#pragma region UnaryExpAST

void UnaryExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "UnaryExpAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: ";

  if (uex == uex_t::Primary) {
    os << "Primary" << endl;
    exp->Print(os, indent + 1);
  } else {
    os << "UnaryOp UnaryExp" << endl;
    make_indent(os, indent + 1);
    os << "op: " << (uop == uop_t::Pos ? '+' : (uop == uop_t::Neg ? '-' : '!'))
       << endl;
    exp->Print(os, indent + 1);
  }

  make_indent(os, indent);
  os << " }," << endl;
}

void UnaryExpAST::Dump() {
  exp->Dump();
  IRGenerator& gen = IRGenerator::getInstance();

  switch (uex) {
    case Primary: {
      auto pr = dynamic_cast<PrimaryExpAST*>(exp.get());
      thisRet = pr->thisRet;
    } break;
    case OPUnary: {
      auto ex = dynamic_cast<UnaryExpAST*>(exp.get());
      switch (uop) {
        case uop_t::Pos:
          thisRet = ex->thisRet;
          break;
        case uop_t::Neg:
          thisRet = gen.WriteUnaryInst(ex->thisRet, OpID::UNARY_NEG);
          break;
        case uop_t::Not:
          thisRet = gen.WriteUnaryInst(ex->thisRet, OpID::UNARY_NOT);
          break;
      }
    } break;
    default:
      break;
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

    auto me = dynamic_cast<MulExpAST*>(mexp.get());
    auto ue = dynamic_cast<UnaryExpAST*>(uexp.get());

    IRGenerator& gen = IRGenerator::getInstance();
    switch (mop) {
      case mop_t::Mul:
        thisRet = gen.WriteBinaryInst(me->thisRet, ue->thisRet, OpID::BI_MUL);
        break;
      case mop_t::Div:
        thisRet = gen.WriteBinaryInst(me->thisRet, ue->thisRet, OpID::BI_DIV);
        break;
      case mop_t::Mod:
        thisRet = gen.WriteBinaryInst(me->thisRet, ue->thisRet, OpID::BI_MOD);
        break;
      default:
        assert(false);
    }
  } else {
    uexp->Dump();
    auto ue = dynamic_cast<UnaryExpAST*>(uexp.get());
    thisRet = ue->thisRet;
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
    auto ae = dynamic_cast<AddExpAST*>(aexp.get());
    auto me = dynamic_cast<MulExpAST*>(mexp.get());

    IRGenerator& gen = IRGenerator::getInstance();
    switch (aop) {
      case aop_t::Add:
        thisRet = gen.WriteBinaryInst(ae->thisRet, me->thisRet, OpID::BI_ADD);
        break;
      case aop_t::Sub:
        thisRet = gen.WriteBinaryInst(ae->thisRet, me->thisRet, OpID::BI_SUB);
        break;
      default:
        break;
    }
  } else {
    mexp->Dump();
    auto me = dynamic_cast<MulExpAST*>(mexp.get());
    thisRet = me->thisRet;
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

    auto rel = dynamic_cast<RelExpAST*>(rexp.get());
    auto ae = dynamic_cast<AddExpAST*>(aexp.get());

    switch (rop) {
      case rop_t::LessThan:
        thisRet = gen.WriteBinaryInst(rel->thisRet, ae->thisRet, OpID::LG_LT);
        break;
      case rop_t::LessEqual:
        thisRet = gen.WriteBinaryInst(rel->thisRet, ae->thisRet, OpID::LG_LE);
        break;
      case rop_t::GreaterThan:
        thisRet = gen.WriteBinaryInst(rel->thisRet, ae->thisRet, OpID::LG_GT);
        break;
      case rop_t::GreaterEqual:
        thisRet = gen.WriteBinaryInst(rel->thisRet, ae->thisRet, OpID::LG_GE);
        break;
      default:
        assert(false);
        break;
    }
  } else {
    aexp->Dump();
    auto ae = dynamic_cast<AddExpAST*>(aexp.get());
    thisRet = ae->thisRet;
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
    auto eq = dynamic_cast<EqExpAST*>(eexp.get());
    auto rel = dynamic_cast<RelExpAST*>(rexp.get());

    IRGenerator& gen = IRGenerator::getInstance();

    switch (eop) {
      case eop_t::Equal:
        thisRet = gen.WriteBinaryInst(eq->thisRet, rel->thisRet, OpID::LG_EQ);
        break;
      case eop_t::NotEqual:
        thisRet = gen.WriteBinaryInst(eq->thisRet, rel->thisRet, OpID::LG_NEQ);
        break;
      default:
        assert(false);
        break;
    }
  } else {
    rexp->Dump();
    auto rel = dynamic_cast<RelExpAST*>(rexp.get());
    thisRet = rel->thisRet;
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
  auto& gen = IRGenerator::getInstance();
  auto& pcs = gen.symbolman.getDProc();
  if (laex == laex_t::LAOPEq) {
    auto la = dynamic_cast<LAndExpAST*>(laexp.get());
    auto eq = dynamic_cast<EqExpAST*>(eexp.get());

    if (pcs.IsEnabled() && pcs.getCurSymType() == SymbolType::CONST) {
      laexp->Dump();
      eexp->Dump();
      thisRet = IRGenerator::getInstance().WriteLogicInst(
          la->thisRet, eq->thisRet, OpID::LG_AND);
      return;
    }

    /*
      int result = lhs != 0;
      if (lhs != 0) {
        result = rhs != 0;
      }
    */

    // int result = lhs != 0;
    laexp->Dump();
    auto entry = pcs.QuickGenEntry(SymbolType::VAR, VarType::INT,
                                   gen.registerShortCircuitVar());
    gen.WriteAllocInst(entry);
    RetInfo lhsNeZero =
        gen.WriteBinaryInst(la->thisRet, RetInfo(0), OpID::LG_NEQ);
    gen.WriteStoreInst(lhsNeZero, entry);

    // if (lhs != 0) {
    IfInfo ifinfo(IfInfo::i);
    gen.WriteBrInst(lhsNeZero, ifinfo);

    // result = rhs != 0;
    gen.WriteLabel(ifinfo.then_label);
    eexp->Dump();
    RetInfo rhsNeZero =
        gen.WriteBinaryInst(eq->thisRet, RetInfo(0), OpID::LG_NEQ);
    gen.WriteStoreInst(rhsNeZero, entry);
    gen.WriteJumpInst(ifinfo.next_label);

    // load return
    gen.WriteLabel(ifinfo.next_label);
    thisRet = gen.WriteLoadInst(entry);

  } else {
    eexp->Dump();
    auto ee = dynamic_cast<EqExpAST*>(eexp.get());
    thisRet = ee->thisRet;
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
  auto& gen = IRGenerator::getInstance();
  auto& pcs = gen.symbolman.getDProc();
  if (loex == loex_t::LOOPLA) {
    auto la = dynamic_cast<LAndExpAST*>(laexp.get());
    auto lo = dynamic_cast<LOrExpAST*>(loexp.get());

    if (pcs.IsEnabled() && pcs.getCurSymType() == SymbolType::CONST) {
      loexp->Dump();
      laexp->Dump();
      thisRet = gen.WriteLogicInst(la->thisRet, lo->thisRet, OpID::LG_OR);
      return;
    }

    /*
      int result = lhs != 0;
      if (lhs == 0) {
        result = rhs != 0;
      }
    */

    // int result = lhs;
    loexp->Dump();
    auto entry = pcs.QuickGenEntry(SymbolType::VAR, VarType::INT,
                                   gen.registerShortCircuitVar());
    gen.WriteAllocInst(entry);
    RetInfo lhsNeZero =
        gen.WriteBinaryInst(lo->thisRet, RetInfo(0), OpID::LG_NEQ);
    gen.WriteStoreInst(lhsNeZero, entry);

    // if (lhs == 0) {
    IfInfo ifinfo(IfInfo::i);
    RetInfo lhsEqZero = gen.WriteUnaryInst(lhsNeZero, OpID::UNARY_NOT);
    gen.WriteBrInst(lhsEqZero, ifinfo);

    // result = rhs != 0;
    gen.WriteLabel(ifinfo.then_label);
    laexp->Dump();
    RetInfo rhsNeZero =
        gen.WriteBinaryInst(la->thisRet, RetInfo(0), OpID::LG_NEQ);
    gen.WriteStoreInst(rhsNeZero, entry);
    gen.WriteJumpInst(ifinfo.next_label);

    // load return
    gen.WriteLabel(ifinfo.next_label);
    thisRet = gen.WriteLoadInst(entry);

  } else {
    laexp->Dump();
    auto la = dynamic_cast<LAndExpAST*>(laexp.get());
    thisRet = la->thisRet;
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
  auto ptr = dynamic_cast<ExpAST*>(exp.get());
  thisRet = ptr->thisRet;
  assert(thisRet.ty == RetInfo::ty_int);
}

#pragma endregion

void make_indent(ostream& os, int indent) {
  string idt(INDENT_LEN, ' ');
  for (int i = 0; i < indent; i++) {
    os << idt;
  }
}
