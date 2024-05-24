#include "ir_ast.h"
#include "ir_util.h"
using namespace ir;

#pragma region CompRoot

void CompRootAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "CompRootAST {" << endl;
  for (auto it = comp_units.begin(); it != comp_units.end(); it++) {
    (*it)->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void CompRootAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  gen.WriteLibFuncDecl();

  // 先处理全局变量
  gen.symbolCore.dproc.global = true;
  for (auto it = comp_units.begin(); it != comp_units.end(); it++) {
    if ((*it)->ty == CompUnitAST::e_decl) {
      (*it)->Dump();
    }
  }
  gen.symbolCore.dproc.global = false;
  gen.setting.getOs() << endl;

  for (auto it = comp_units.begin(); it != comp_units.end(); it++) {
    if ((*it)->ty == CompUnitAST::e_func_def) {
      (*it)->Dump();
      gen.funcCore.Reset();
      gen.branchCore.Reset();
    }
  }
}

#pragma endregion

#pragma region CompUnitList

void CompUnitListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void CompUnitListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region Comp

void CompUnitAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "CompUnitAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << (ty == e_func_def ? "FuncDef" : "Decl") << endl;
  content->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void CompUnitAST::Dump() {
  content->Dump();
}

#pragma endregion

#pragma region Decl
void DeclAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "DeclAST {" << endl;
  make_indent(os, indent + 1);
  if (de == de_t::e_const) {
    os << "type: const" << endl;
  } else {
    os << "type: var" << endl;
  }
  decl->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void DeclAST::Dump() {
  decl->Dump();
  IRGenerator::getInstance().symbolCore.dproc.Reset();
}

#pragma endregion

#pragma region ConstDecl

void ConstDeclAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ConstDeclAST {" << endl;
  btype->Print(os, indent + 1);
  for (auto it = const_defs.begin(); it != const_defs.end(); it++) {
    (*it)->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void ConstDeclAST::Dump() {
  DeclaimProcessor& processor = IRGenerator::getInstance().symbolCore.dproc;
  processor.Enable();
  processor.SetSymbolType(SymbolType::e_const);
  btype->Dump();
  for (auto it = const_defs.begin(); it != const_defs.end(); it++) {
    (*it)->Dump();
  }
  processor.Disable();
}

#pragma endregion

#pragma region ConstDeclList

void ConstDeclListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void ConstDeclListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region BType

void BTypeAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "BTypeAST: " << (ty == e_int ? "int" : "void") << endl;
}

void BTypeAST::Dump() {
  IRGenerator::getInstance().symbolCore.dproc.SetVarType(
      (ty == e_int ? VarType::e_int : VarType::e_void));
}

#pragma endregion

#pragma region ConstDef

void ConstDefAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ConstDefAST: {" << endl;
  make_indent(os, indent + 1);
  os << "var_name: " << var_name << endl;
  make_indent(os, indent + 1);
  os << "ty: " << (ty == e_int ? "int" : "array") << endl;
  if (ty == e_arr) {
    arr_size->Print(os, indent + 1);
  }
  const_init_val->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void ConstDefAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  auto& pcs = gen.symbolCore.dproc;

  if (ty == e_int) {
    // 计算常数表达式
    const_init_val->Dump();
    auto ptr = dynamic_cast<ConstInitValAST*>(const_init_val.get());

    // 取值加入符号表
    auto entry = pcs.GenerateConstEntry(var_name, ptr->thisRet.GetValue());
    gen.symbolCore.InsertEntry(entry);
  }

  else if (ty == e_arr) {
    // arr
    arr_size->Dump();
    auto& size = dynamic_cast<ArrSizeAST*>(arr_size.get())->size_value;

    ArrInfo info(size);

    // 取值加入符号表
    auto entry = pcs.GenerateArrEntry(var_name, info);
    gen.symbolCore.InsertEntry(entry);

    // 解析常数数组表达式
    const_init_val->Dump();

    // 是全局变量
    if (pcs.global) {
      gen.WriteGlobalArrVar(entry);
    } else {
      // 是局部变量
      gen.WriteAllocArrInst(entry, true);
    }
  }
}

#pragma endregion

#pragma region ConstInitVal

void ConstInitValAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ConstInitValAST: {" << endl;
  const_exp->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void ConstInitValAST::Dump() {
  const_exp->Dump();
  auto ce = dynamic_cast<ConstExpAST*>(const_exp.get());
  thisRet = ce->thisRet;
}

#pragma endregion

#pragma region VarDecl
void VarDeclAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "VarDeclAST {" << endl;
  btype->Print(os, indent + 1);
  for (auto it = var_defs.begin(); it != var_defs.end(); it++) {
    (*it)->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void VarDeclAST::Dump() {
  DeclaimProcessor& processor = IRGenerator::getInstance().symbolCore.dproc;
  processor.Enable();
  processor.SetSymbolType(SymbolType::e_var);
  btype->Dump();
  for (auto it = var_defs.begin(); it != var_defs.end(); it++) {
    (*it)->Dump();
  }
  processor.Disable();
}

#pragma endregion

#pragma region VarDeclList

void VarDeclListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void VarDeclListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region VarDef

void VarDefAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "VarDefAST: {" << endl;
  make_indent(os, indent + 1);
  os << "var_name: " << var_name << endl;
  make_indent(os, indent + 1);
  os << "ty: " << (ty == e_int ? "int" : "array") << endl;
  if (ty == e_arr) {
    arr_size->Print(os, indent + 1);
  }
  make_indent(os, indent + 1);
  os << "declaim with value: " << (init_with_val ? "true" : "false") << endl;
  if (init_with_val) {
    init_val->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void VarDefAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  auto& pcs = gen.symbolCore.dproc;
  if (ty == e_int) {
    // int
    SymbolTableEntry entry = pcs.GenerateVarEntry(var_name, VarType::e_int);
    gen.symbolCore.InsertEntry(entry);

    // 初始化信息
    RetInfo init;
    if (init_with_val) {
      init_val->Dump();
      auto iv = dynamic_cast<InitValAST*>(init_val.get());
      init = iv->thisRet;
    }
    if (pcs.global) {
      // 声明全局变量，无初始化则为零初始化
      gen.WriteGlobalVar(entry, init);
    } else {
      // 局部变量
      gen.WriteAllocInst(entry);
      if (init_with_val) {
        gen.WriteStoreInst(init, entry);
      }
    }

  } else if (ty == e_arr) {
    // arr
    arr_size->Dump();
    auto& size = dynamic_cast<ArrSizeAST*>(arr_size.get())->size_value;
    ArrInfo info(size);

    // 取值加入符号表
    auto entry = pcs.GenerateArrEntry(var_name, info);
    gen.symbolCore.InsertEntry(entry);

    // 处理可能的初始化
    // 空的初始化，在全局中被视为零，局部中视为不初始化
    // {}视为零初始化
    bool has_init = false;

    if (init_with_val) {
      // 解析常数数组表达式
      init_val->Dump();
      has_init = true;
    }

    // 是全局变量
    if (pcs.global) {
      gen.WriteGlobalArrVar(entry);
    } else {
      // 是局部变量
      gen.WriteAllocArrInst(entry, has_init);
    }
  }
}

#pragma endregion

#pragma region InitVal

void InitValAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "InitValAST: {" << endl;
  exp->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void InitValAST::Dump() {
  exp->Dump();
  auto ae = dynamic_cast<ExpAST*>(exp.get());
  thisRet = ae->thisRet;
}

#pragma endregion

#pragma region ArrSize

void ArrSizeAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ArrSizeAST: {" << endl;
  make_indent(os, indent + 1);
  os << "shape: " << endl;
  for (int i = 0; i < arr_size.size(); i++) {
    arr_size[i]->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void ArrSizeAST::Dump() {
  for (int i = 0; i < arr_size.size(); i++) {
    arr_size[i]->Dump();
    auto ptr = dynamic_cast<ConstExpAST*>(arr_size[i].get());
    size_value.push_back(ptr->thisRet.GetValue());
  }
}

#pragma endregion

#pragma region ArrSizeList

void ArrSizeListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void ArrSizeListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region CAElement

void CAElementAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "CAElementAST: {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << (ty == e_cexp ? "ConstExp" : "ConstArrVal") << endl;
  content->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void CAElementAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  auto& arrinit = gen.arrinitCore;
  if (ty == e_cexp) {
    content->Dump();
    auto ptr = dynamic_cast<ConstExpAST*>(content.get());
    arrinit.PushInfo(ptr->thisRet);
  } else if (ty == e_carr) {
    content->Dump();
  }
}

#pragma endregion

#pragma region ConstArrVal

void ConstArrValAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ConstArrValAST: {" << endl;
  int len = values.size();
  if (len == 0) {
    make_indent(os, indent + 1);
    os << "no init" << endl;
  }
  for (int i = 0; i < len; i++) {
    values[i]->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void ConstArrValAST::Dump() {
  int len = values.size();
  // 如果只有一个元素且元素类型为arr，则不增加大括号
  if (len == 1 && values[0]->ty == CAElementAST::e_carr) {
    values[0]->Dump();
  } else {
    auto& gen = IRGenerator::getInstance();
    auto& arrinit = gen.arrinitCore;

    // 添加大括号
    // 若为根则不添加大括号
    bool insert = arrinit.should_insert;
    arrinit.should_insert = true;

    if (insert)
      arrinit.PushArr();
    // 遍历子项
    for (int i = 0; i < len; i++) {
      values[i]->Dump();
    }
    // 推出
    if (insert)
      arrinit.PopArr();
  }
}

#pragma endregion

#pragma region CAElementList

void CAElementListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void CAElementListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region AIElement

void AIElementAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "AIElementAST: {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << (ty == e_exp ? "Exp" : "ArrInitVal") << endl;
  content->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void AIElementAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  auto& arrinit = gen.arrinitCore;
  if (ty == e_exp) {
    content->Dump();
    auto ptr = dynamic_cast<ExpAST*>(content.get());
    arrinit.PushInfo(ptr->thisRet);
  } else if (ty == e_arr) {
    content->Dump();
  }
}

#pragma endregion

#pragma region ArrInitVal

void ArrInitValAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ArrInitValAST: {" << endl;
  int len = values.size();
  if (len == 0) {
    make_indent(os, indent + 1);
    os << "no init" << endl;
  }
  for (int i = 0; i < len; i++) {
    values[i]->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void ArrInitValAST::Dump() {
  int len = values.size();
  // 如果只有一个元素且元素类型为arr，则不增加大括号
  if (len == 1 && values[0]->ty == AIElementAST::e_arr) {
    values[0]->Dump();
  } else {
    auto& gen = IRGenerator::getInstance();
    auto& arrinit = gen.arrinitCore;

    // 添加大括号
    // 若为根则不添加大括号
    bool insert = arrinit.should_insert;
    arrinit.should_insert = true;

    if (insert)
      arrinit.PushArr();
    // 遍历子项
    for (int i = 0; i < len; i++) {
      values[i]->Dump();
    }
    // 推出
    if (insert)
      arrinit.PopArr();
  }
}

#pragma endregion

#pragma region AIElementList

void AIElementListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void AIElementListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region FuncDef

void FuncDefAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncDefAST {" << endl;
  func_type->Print(os, indent + 1);
  make_indent(os, indent + 1);
  os << "func name: \"" << func_name << "\"," << endl;
  params->Print(os, indent + 1);
  block->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void FuncDefAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  // 记录函数类型和参数
  gen.symbolCore.dproc.Enable();
  func_type->Dump();
  gen.funcCore.ret_ty = gen.symbolCore.dproc.getCurVarType();
  params->Dump();
  gen.symbolCore.dproc.Disable();

  gen.funcCore.func_name = func_name;

  gen.symbolCore.PushScope();
  gen.WriteFuncPrologue();
  block->Dump();
  gen.symbolCore.PopScope();
  // 如果函数结束没有return，就按照函数返回值类型补一个return;
  if (!gen.branchCore.hasRetThisBB) {
    gen.funcCore.SetDefaultRetInfo();
    gen.WriteRetInst();
  }
  gen.WriteFuncEpilogue();
}

#pragma endregion

#pragma region FuncFParams

void FuncFParamsAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncFParamsAST {" << endl;
  for (auto it = params.begin(); it != params.end(); it++) {
    (*it)->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void FuncFParamsAST::Dump() {
  for (auto it = params.begin(); it != params.end(); it++) {
    (*it)->Dump();
  }
}

#pragma endregion

#pragma region FuncFParamsList

void FuncFParamsListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void FuncFParamsListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region FuncFParam

void FuncFParamAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncFParamAST {" << endl;
  make_indent(os, indent + 1);
  os << "param name: " << param_name << endl;
  make_indent(os, indent + 1);
  os << "is pointer: " << (is_ptr ? "true" : "false") << endl;
  if (is_ptr) {
    make_indent(os, indent + 1);
    os << "pointer size: []" << endl;
    ptr_size->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void FuncFParamAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  auto& pcs = gen.symbolCore.dproc;
  if (is_ptr) {
    ptr_size->Dump();
    auto size(dynamic_cast<ArrSizeAST*>(ptr_size.get())->size_value);

    // 数组开头先拍一个1
    size.insert(size.begin(), 0);
    ArrInfo info(size);

    // 取值，不加入符号表，符号表在之后才加
    auto entry = pcs.GeneratePtrEntry(param_name, info);
    gen.funcCore.InsertParam(entry);
  } else {
    gen.funcCore.InsertParam(VarType::e_int, param_name);
  }
}

#pragma endregion

#pragma region Block

void BlockAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "BlockAST {" << endl;
  for (auto it = block_items.begin(); it != block_items.end(); it++) {
    (*it)->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void BlockAST::Dump() {
  for (auto it = block_items.begin(); it != block_items.end(); it++) {
    (*it)->Dump();
  }
}

#pragma endregion

#pragma region BlockList
void BlockListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void BlockListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
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
  os << "}," << endl;
}

void BlockItemAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  if (!gen.branchCore.hasRetThisBB) {
    content->Dump();
  }
}

#pragma endregion

#pragma region Stmt

void StmtAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "StmtAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << (type == stmty_t::open ? "open" : "closed") << endl;
  stmt->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void StmtAST::Dump() {
  stmt->Dump();
}

#pragma endregion

#pragma region OpenStmt

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
    case opty_t::loop:
      os << "Loop" << endl;
      make_indent(os, indent + 1);
      os << "WHILE" << endl;
      exp->Print(os, indent + 1);
      open->Print(os, indent + 1);
      break;
  }
  make_indent(os, indent);
  os << "}," << endl;
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
      if (!gen.branchCore.hasRetThisBB) {
        gen.WriteJumpInst(ifin.next_label);
      }
      gen.WriteLabel(ifin.next_label);
      break;

    case iceo: {
      ifin = IfInfo(IfInfo::ifty_t::ie);
      gen.WriteBrInst(ret, ifin);

      gen.WriteLabel(ifin.then_label);
      closed->Dump();
      bool retInThen = gen.branchCore.hasRetThisBB;
      if (!retInThen) {
        gen.WriteJumpInst(ifin.next_label);
      }

      gen.WriteLabel(ifin.else_label);
      open->Dump();
      bool retInElse = gen.branchCore.hasRetThisBB;
      if (!gen.branchCore.hasRetThisBB) {
        gen.WriteJumpInst(ifin.next_label);
      }

      if (!(retInElse && retInThen)) {
        gen.WriteLabel(ifin.next_label);
      }
    } break;

    case loop:
    default: {
      LoopInfo loopInfo;
      gen.InitLoopInfo(loopInfo);
      gen.WriteJumpInst(loopInfo.cond_label);

      gen.WriteLabel(loopInfo.cond_label);
      exp->Dump();
      auto cond = dynamic_cast<ExpAST*>(exp.get());
      RetInfo ret = cond->thisRet;
      gen.WriteBrInst(ret, loopInfo);
      gen.branchCore.PushInfo(loopInfo);

      gen.WriteLabel(loopInfo.body_label);
      open->Dump();
      if (!gen.branchCore.hasRetThisBB) {
        gen.WriteJumpInst(loopInfo.cond_label);
      }

      gen.WriteLabel(loopInfo.next_label);
      gen.branchCore.PopInfo();
    } break;
  }
}

#pragma endregion

#pragma region ClosedStmt

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
    case csty_t::loop:
      os << "Loop" << endl;
      make_indent(os, indent + 1);
      os << "WHILE" << endl;
      exp->Print(os, indent + 1);
      tclosed->Print(os, indent + 1);
      break;
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void ClosedStmtAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();

  switch (type) {
    case simp:
      simple->Dump();
      break;

    case icec: {
      exp->Dump();
      auto cond = dynamic_cast<ExpAST*>(exp.get());
      RetInfo ret = cond->thisRet;
      IfInfo ifin(IfInfo::ifty_t::ie);
      gen.WriteBrInst(ret, ifin);

      gen.WriteLabel(ifin.then_label);
      tclosed->Dump();
      bool retInThen = gen.branchCore.hasRetThisBB;
      if (!retInThen) {
        gen.WriteJumpInst(ifin.next_label);
      }

      gen.WriteLabel(ifin.else_label);
      fclosed->Dump();
      bool retInElse = gen.branchCore.hasRetThisBB;
      if (!gen.branchCore.hasRetThisBB) {
        gen.WriteJumpInst(ifin.next_label);
      }

      if (!(retInElse && retInThen)) {
        gen.WriteLabel(ifin.next_label);
      }
    } break;

    case loop:
    default: {
      LoopInfo loopInfo;
      gen.InitLoopInfo(loopInfo);
      gen.WriteJumpInst(loopInfo.cond_label);

      gen.WriteLabel(loopInfo.cond_label);
      exp->Dump();
      auto cond = dynamic_cast<ExpAST*>(exp.get());
      RetInfo ret = cond->thisRet;
      gen.WriteBrInst(ret, loopInfo);
      gen.branchCore.PushInfo(loopInfo);

      gen.WriteLabel(loopInfo.body_label);
      tclosed->Dump();
      if (!gen.branchCore.hasRetThisBB) {
        gen.WriteJumpInst(loopInfo.cond_label);
      }

      gen.WriteLabel(loopInfo.next_label);
      gen.branchCore.PopInfo();
    } break;
  }
}

#pragma endregion

#pragma region SimpleStmt
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
    case sstmt_t::cont:
      os << "continue" << endl;
      break;
    case sstmt_t::brk:
      os << "break" << endl;
      break;
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void SimpleStmtAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();
  switch (st) {
    case sstmt_t::storelval: {
      AssignmentProcessor& aproc = gen.symbolCore.aproc;
      // 记录左值
      aproc.Enable();
      lval->Dump();
      aproc.Disable();

      // 计算表达式
      exp->Dump();
      auto ee = dynamic_cast<ExpAST*>(exp.get());

      // 赋值
      aproc.WriteAssign(ee->thisRet);
    } break;

    case sstmt_t::ret: {
      exp->Dump();
      auto ee = dynamic_cast<ExpAST*>(exp.get());
      // 设置返回值
      gen.funcCore.ret_info = ee->thisRet;
      gen.WriteRetInst();
    } break;

    case sstmt_t::expr: {
      exp->Dump();
    } break;

    case sstmt_t::block: {
      gen.symbolCore.PushScope();
      blk->Dump();
      gen.symbolCore.PopScope();
    } break;

    case sstmt_t::nullexp: {
    } break;

    case sstmt_t::nullret: {
      gen.funcCore.ret_info = RetInfo();
      gen.WriteRetInst();
    } break;

    case sstmt_t::brk: {
      auto& brCore = gen.branchCore;
      if (!brCore.IsInALoop())
        return;
      auto& loopInfo = brCore.GetCurInfo();
      gen.WriteJumpInst(loopInfo.next_label);
      gen.WriteLabel(brCore.GenerateLabelFromBranchedLoop());
    } break;

    case sstmt_t::cont: {
      auto& brCore = gen.branchCore;
      if (!brCore.IsInALoop())
        return;
      auto& loopInfo = brCore.GetCurInfo();
      gen.WriteJumpInst(loopInfo.cond_label);
      gen.WriteLabel(brCore.GenerateLabelFromBranchedLoop());
    } break;

    default:
      break;
  }
}

#pragma endregion

#pragma region Exp

void ExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ExpAST {" << endl;
  loexp->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
}

void ExpAST::Dump() {
  loexp->Dump();
  auto le = dynamic_cast<LOrExpAST*>(loexp.get());
  thisRet = le->thisRet;
}

#pragma endregion

#pragma region ArrAddr

void ArrAddrAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ArrAddrAST: {" << endl;
  make_indent(os, indent + 1);
  os << "addr: " << endl;
  for (int i = 0; i < arr_addr.size(); i++) {
    arr_addr[i]->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void ArrAddrAST::Dump() {
  for (int i = 0; i < arr_addr.size(); i++) {
    arr_addr[i]->Dump();
    auto ptr = dynamic_cast<ExpAST*>(arr_addr[i].get());
    addr_value.push_back(ptr->thisRet.GetInfo());
  }
}

#pragma endregion

#pragma region ArrAddrList

void ArrAddrListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void ArrAddrListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region LVal

void LValAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "LValAST {" << endl;
  make_indent(os, indent + 1);
  os << "var name: " << var_name << endl;
  make_indent(os, indent + 1);
  os << "ty: " << (ty == e_noaddr ? "no addr" : "with addr") << endl;
  if (ty == e_withaddr) {
    make_indent(os, indent + 1);
    os << "array param:" << endl;
    arr_param->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void LValAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  auto& aproc = gen.symbolCore.aproc;
  SymbolTableEntry entry = gen.symbolCore.getEntry(var_name);
  // 判断变量类型

  // 是否将被赋值，是否是左值。
  // 在SimpleStmt中，仅当Lval = Exp语句时（解析Lval时）启用。
  bool is_assigning = aproc.IsEnabled();

  if (entry.var_type == VarType::e_int) {
    // int
    if (entry.symbol_type == SymbolType::e_const) {
      // const int，const只会是右值
      thisRet = RetInfo(entry.const_value);
    } else if (entry.symbol_type == SymbolType::e_var) {
      // var int
      if (is_assigning) {
        // 左值，设置aproc处理当前符号
        aproc.current_var = entry;
        aproc.Disable();
      } else {
        // 右值，获取其临时符号
        thisRet = gen.WriteLoadInst(entry);
      }
    }

  } else if (entry.var_type == VarType::e_arr) {
    // arr
    // 蛤蛤
    // 我爱lv9
    // 如果是数组且没有地址或地址维度不够长，退化成指针
    // 否则退化成int

    if (is_assigning) {
      // 左值，设置aproc处理当前符号
      // 地址一定够长
      aproc.current_var = entry;
      aproc.Disable();

      // 解析数组参数
      arr_param->Dump();
      auto ptr = dynamic_cast<ArrAddrAST*>(arr_param.get());
      const auto addr = ptr->addr_value;
      aproc.arr_addr = addr;
    } else {
      // 右值，获取其临时符号
      // const arr也在此处处理

      // 解析数组参数
      vector<RetInfo> addr;
      if (ty == e_withaddr) {
        arr_param->Dump();
        auto ptr = dynamic_cast<ArrAddrAST*>(arr_param.get());
        addr = ptr->addr_value;

        // 数组，地址足够长，就从数组中load值作为thisRet
        if (addr.size() == entry.arr_info.Dim()) {
          thisRet = gen.WriteLoadArrInst(entry, addr);
          return;
        }
        // 不够长，退化成指针
        addr.push_back(RetInfo(0));
        thisRet = gen.WriteGetPtrFromArr(entry.GetAllocName(), addr);
      } else
        // 没有地址，退化成指针
        addr.push_back(RetInfo(0));
      thisRet = gen.WriteGetPtrFromArr(entry.GetAllocName(), addr);
    }

  } else if (entry.var_type == VarType::e_ptr) {
    // ptr

    auto ptr = dynamic_cast<ArrAddrAST*>(arr_param.get());

    if (is_assigning) {
      // 左值，设置aproc处理当前符号
      aproc.current_var = entry;
      aproc.Disable();

      // 解析指针参数
      arr_param->Dump();
      const auto addr = ptr->addr_value;
      aproc.arr_addr = addr;
    } else {
      // 右值，获取其临时符号

      // 解析数组参数
      vector<RetInfo> addr;
      if (ty == e_withaddr) {
        arr_param->Dump();
        auto ptr = dynamic_cast<ArrAddrAST*>(arr_param.get());
        addr = ptr->addr_value;

        // 数组，地址足够长，就从数组中load值作为thisRet
        if (addr.size() == entry.arr_info.Dim()) {
          thisRet = gen.WriteLoadPtrInst(entry, addr);
          return;
        }
        // 不够长，退化成指针
        addr.push_back(RetInfo(0));
        thisRet = gen.WriteGetPtrFromPtr(entry.GetAllocName(), addr);
      } else
        // 没有地址，退化成指针
        addr.push_back(RetInfo(0));
      thisRet = gen.WriteGetPtrFromPtr(entry.GetAllocName(), addr);
    }
  }
}

#pragma endregion

#pragma region PrimaryExp

void PrimaryExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "PrimaryAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: " << type() << endl;
  content->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
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

#pragma region Number

void NumberAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "NumberAST { int_const: " << int_const << " }," << endl;
}

void NumberAST::Dump() {
  // do nothing, waiting for PrimaryExpAST to actively fetch imm
}

#pragma endregion

#pragma region UnaryExp

void UnaryExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "UnaryExpAST {" << endl;
  make_indent(os, indent + 1);
  os << "type: ";

  if (uex == uex_t::Primary) {
    os << "Primary" << endl;
    exp->Print(os, indent + 1);
  } else if (uex == uex_t::OPUnary) {
    os << "UnaryOp UnaryExp" << endl;
    make_indent(os, indent + 1);
    os << "op: " << (uop == uop_t::Pos ? '+' : (uop == uop_t::Neg ? '-' : '!'))
       << endl;
    exp->Print(os, indent + 1);
  } else if (uex == uex_t::FuncWithParam) {
    os << "Func" << endl;
    make_indent(os, indent + 1);
    os << "Func name: " << func_name << endl;
    params->Print(os, indent + 1);
  } else if (uex == uex_t::FuncNoParam) {
    os << "Func" << endl;
    make_indent(os, indent + 1);
    os << "Func name: " << func_name << endl;
  }

  make_indent(os, indent);
  os << "}," << endl;
}

void UnaryExpAST::Dump() {
  IRGenerator& gen = IRGenerator::getInstance();

  switch (uex) {
    case uex_t::Primary: {
      exp->Dump();
      auto pr = dynamic_cast<PrimaryExpAST*>(exp.get());
      thisRet = pr->thisRet;
    } break;
    case uex_t::OPUnary: {
      exp->Dump();
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
    case uex_t::FuncWithParam: {
      params->Dump();
      auto ptr = dynamic_cast<FuncRParamsAST*>(params.get());
      thisRet = gen.WriteCallInst(func_name, ptr->GetParams());
    } break;
    case uex_t::FuncNoParam:
    default:
      thisRet = gen.WriteCallInst(func_name, vector<RetInfo>());
      break;
      ;
  }
}

#pragma endregion

#pragma region FuncRParams

void FuncRParamsAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "FuncRParamsAST {" << endl;
  for (auto it = params.begin(); it != params.end(); it++) {
    (*it)->Print(os, indent + 1);
  }
  make_indent(os, indent);
  os << "}," << endl;
}

void FuncRParamsAST::Dump() {
  for (auto it = params.begin(); it != params.end(); it++) {
    (*it)->Dump();
    parsed_params.push_back((*it)->thisRet);
  }
}

const vector<RetInfo>& FuncRParamsAST::GetParams() const {
  return parsed_params;
}

#pragma endregion

#pragma region FuncRParamsList

void FuncRParamsListUnit::Print(ostream& os, int indent) const {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

void FuncRParamsListUnit::Dump() {
  cerr << "[SHOULD NOT OUTPUT THIS]" << endl;
}

#pragma endregion

#pragma region MulExp

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
  os << "}," << endl;
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

#pragma region AddExp

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
  os << "}," << endl;
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

#pragma region RelExp

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
  os << "}," << endl;
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

#pragma region EqExp

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
  os << "}," << endl;
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

#pragma region LAndExp

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
  os << "}," << endl;
}

void LAndExpAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  auto& pcs = gen.symbolCore.dproc;
  if (laex == laex_t::LAOPEq) {
    auto la = dynamic_cast<LAndExpAST*>(laexp.get());
    auto eq = dynamic_cast<EqExpAST*>(eexp.get());

    if (pcs.IsEnabled() && pcs.getCurSymType() == SymbolType::e_const) {
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
    auto entry = pcs.QuickGenEntry(SymbolType::e_var, VarType::e_int,
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

#pragma region LOrExp

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
  os << "}," << endl;
}

void LOrExpAST::Dump() {
  auto& gen = IRGenerator::getInstance();
  auto& pcs = gen.symbolCore.dproc;
  if (loex == loex_t::LOOPLA) {
    auto la = dynamic_cast<LAndExpAST*>(laexp.get());
    auto lo = dynamic_cast<LOrExpAST*>(loexp.get());

    if (pcs.IsEnabled() && pcs.getCurSymType() == SymbolType::e_const) {
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
    auto entry = pcs.QuickGenEntry(SymbolType::e_var, VarType::e_int,
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

#pragma region ConstExp

void ConstExpAST::Print(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ConstExpAST: {" << endl;
  exp->Print(os, indent + 1);
  make_indent(os, indent);
  os << "}," << endl;
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