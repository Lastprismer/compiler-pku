#include "ir_gen.h"

namespace ir {

#pragma region RetInfo

RetInfo::RetInfo() : ty(ty_void) {}

RetInfo::RetInfo(int _value) : ty(ty_int), value(_value) {}

RetInfo::RetInfo(string _symbol) : ty(ty_sbl), name(_symbol) {}

const int& RetInfo::GetValue() const {
  assert(ty == ty_int);
  return value;
}

const string& RetInfo::GetSym() const {
  assert(ty == ty_sbl);
  return name;
}

const string RetInfo::GetInfo() const {
  switch (ty) {
    case retty_t::ty_int:
      return to_string(GetValue());
    case retty_t::ty_sbl:
      return GetSym();
    default:
      return "";
  }
}

#pragma endregion

#pragma region IfInfo

IfInfo::IfInfo()
    : ty(ifty_t::i), then_label(-1), else_label(-1), next_label(-1) {}
IfInfo::IfInfo(ifty_t _ty)
    : ty(_ty), then_label(-1), else_label(-1), next_label(-1) {}

IfInfo::IfInfo(int then, int next)
    : ty(ifty_t::i), then_label(then), next_label(next) {}

IfInfo::IfInfo(int then, int _else, int next)
    : ty(ifty_t::ie), then_label(then), else_label(_else), next_label(next) {}

#pragma endregion

#pragma region LoopInfo

LoopInfo::LoopInfo() {}

#pragma endregion

#pragma region ArrInfo

ArrInfo::ArrInfo() : shape(), size(0) {}

ArrInfo::ArrInfo(const vector<int>& _shape) : shape(_shape) {
  size = 1;
  for (const int& i : shape) {
    size *= i;
  }
}

const string ArrInfo::GetType() const {
  // int a[4][3]: [[i32, 3], 4]
  stringstream ss;
  int dim = Dim();
  for (int i = 0; i < dim; i++) {
    ss << "[";
  }
  ss << "i32, " << shape[dim - 1] << "]";
  for (int i = dim - 2; i > -1; i--) {
    ss << ", " << shape[i] << "]";
  }
  return ss.str();
}

const string ArrInfo::GetPtrType() const {
  assert(shape[0] == 0);
  stringstream ss;
  int dim = Dim();
  ss << '*';
  // 第0维是0，不要
  for (int i = 1; i < dim; i++) {
    ss << "[";
  }
  ss << "i32";
  for (int i = dim - 1; i > 0; i--) {
    ss << ", " << shape[i] << "]";
  }
  return ss.str();
}

const int ArrInfo::Dim() const {
  return shape.size();
}

const int ArrInfo::GetSize() const {
  return size;
}

ArrInfo ArrInfo::GetFrag(const int& begin, const int& end) const {
  vector<int> frag;
  frag.assign(shape.begin() + begin, shape.begin() + end);
  return ArrInfo(frag);
}

#pragma endregion

#pragma region arr init

ArrInitNode::ArrInitNode() : ty(e_value), value(), nodes(), parent(nullptr) {}

ArrInitNode::ArrInitNode(const RetInfo& val)
    : ty(e_value), value(val), nodes(), parent(nullptr) {}

ArrInitNode::ArrInitNode(const node_t& type)
    : ty(type), value(), nodes(), parent(nullptr) {}

#pragma endregion

#pragma region STE

SymbolTableEntry::SymbolTableEntry()
    : symbol_type(SymbolType::e_unused), var_type(VarType::e_unused), id(-1) {}

const string SymbolTableEntry::GetAllocName() const {
  return "@" + var_name + '_' + std::to_string(id);
}

const string SymbolTableEntry::GetAllocInst() const {
  assert(symbol_type == SymbolType::e_var);
  if (var_type == VarType::e_int) {
    // int
    // @x = alloc i32
    stringstream ss;
    ss << GetAllocName() << " = alloc i32";
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
    return "";
  }
}

const string SymbolTableEntry::GetLoadInst(
    const string& loadToSymbolName) const {
  assert(symbol_type == SymbolType::e_var);
  if (var_type == VarType::e_int) {
    // int
    // %0 = load @x
    stringstream ss;
    ss << loadToSymbolName << " = load " << GetAllocName();
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
    return "";
  }
}

const string SymbolTableEntry::GetStoreInst(
    const string& storeFromSymbolName) const {
  if (var_type == VarType::e_int || var_type == VarType::e_ptr) {
    // int
    // store %0, @x
    stringstream ss;
    ss << "store " << storeFromSymbolName << ", " << GetAllocName();
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
    return "";
  }
}

const string SymbolTableEntry::GetStoreInst(const int& imm) const {
  // (symbolType == SymbolType::VAR);
  if (var_type == VarType::e_int) {
    // int
    // store 0, @x
    stringstream ss;
    ss << "store " << imm << ", " << GetAllocName();
    return ss.str();
  } else {
    cerr << "[NOT SUPPORTED]" << endl;
    assert(false);
    return "";
  }
}

#pragma endregion

#pragma region Symbol Table

SymbolTable::SymbolTable() : table(), parent(nullptr) {}

bool SymbolTable::TryGetEntry(string symbol_name, SymbolTableEntry& out) const {
  if (table.find(symbol_name) != table.end()) {
    out = table.at(symbol_name);
    return true;
  }
  return false;
}

void SymbolTable::InsertEntry(const SymbolTableEntry& entry) {
  if (table.find(entry.var_name) != table.end()) {
    cerr << "Symbol Table insert error: symbol with name \"" << entry.var_name
         << "\" has already inserted in the table. It will be overwritten by "
            "default"
         << endl;
    table[entry.var_name] = entry;
    return;
  }
  // do actual insert
  table.emplace(entry.var_name, entry);
}

void SymbolTable::ClearTable() {
  table.clear();
}

#pragma endregion

#pragma region BaseProcessor

BaseProcessor::BaseProcessor() : _enable(false) {}

void BaseProcessor::Enable() {
  _enable = true;
}

void BaseProcessor::Disable() {
  _enable = false;
}

const bool& BaseProcessor::IsEnabled() {
  return _enable;
}

#pragma endregion

#pragma region DeclaimProcessor

DeclaimProcessor::DeclaimProcessor()
    : BaseProcessor(),
      current_symbol_type(SymbolType::e_unused),
      current_var_type(VarType::e_unused) {}

void DeclaimProcessor::SetSymbolType(const SymbolType& type) {
  assert(IsEnabled());
  current_symbol_type = type;
}

void DeclaimProcessor::SetVarType(const VarType& type) {
  assert(IsEnabled());
  current_var_type = type;
}

void DeclaimProcessor::Reset() {
  current_symbol_type = SymbolType::e_unused;
  current_var_type = VarType::e_unused;
}

SymbolTableEntry DeclaimProcessor::GenerateConstEntry(const string& var_name,
                                                      const int& value) {
  assert(IsEnabled() && current_symbol_type == SymbolType::e_const &&
         current_var_type != VarType::e_unused);
  SymbolTableEntry ste;
  ste.symbol_type = SymbolType::e_const;
  ste.var_type = VarType::e_int;
  ste.var_name = var_name;
  ste.const_value = value;
  ste.id = RegisterVar();
  return ste;
}

SymbolTableEntry DeclaimProcessor::GenerateArrEntry(const string& var_name,
                                                    const ArrInfo& info) {
  SymbolTableEntry ste;
  ste.symbol_type = SymbolType::e_var;
  ste.var_type = VarType::e_arr;
  ste.var_name = var_name;
  ste.arr_info = info;
  ste.id = RegisterVar();
  return ste;
}

SymbolTableEntry DeclaimProcessor::GeneratePtrEntry(const string& var_name,
                                                    const ArrInfo& ptr_info) {
  SymbolTableEntry ste;
  ste.symbol_type = SymbolType::e_var;
  ste.var_type = VarType::e_ptr;
  ste.var_name = var_name;
  ste.arr_info = ptr_info;
  ste.id = RegisterVar();
  return ste;
}

SymbolTableEntry DeclaimProcessor::GenerateVarEntry(const string& var_name,
                                                    const VarType& var_ty) {
  assert(IsEnabled() && current_symbol_type != SymbolType::e_unused &&
         current_var_type != VarType::e_unused);
  SymbolTableEntry ste;
  ste.symbol_type = SymbolType::e_var;
  ste.var_type = VarType::e_int;
  ste.var_name = var_name;
  ste.id = RegisterVar();
  return ste;
}

SymbolTableEntry DeclaimProcessor::QuickGenEntry(const SymbolType& st,
                                                 const VarType& vt,
                                                 string name) {
  SymbolTableEntry ste;
  ste.symbol_type = st;
  ste.var_type = vt;
  ste.var_name = name;
  ste.id = RegisterVar();
  return ste;
}

const SymbolType DeclaimProcessor::getCurSymType() const {
  return current_symbol_type;
}

const VarType DeclaimProcessor::getCurVarType() const {
  return current_var_type;
}

const int DeclaimProcessor::RegisterVar() {
  return var_pool++;
}

#pragma endregion

#pragma region Assignment

AssignmentProcessor::AssignmentProcessor() : BaseProcessor() {}

void AssignmentProcessor::WriteAssign(const RetInfo& value) const {
  auto& gen = IRGenerator::getInstance();
  if (current_var.var_type == VarType::e_int) {
    gen.WriteStoreInst(value, current_var);
  } else if (current_var.var_type == VarType::e_arr) {
    gen.WriteStoreArrInst(current_var, value, arr_addr);
  } else if (current_var.var_type == VarType::e_ptr) {
    gen.WriteStorePtrInst(current_var, value, arr_addr);
  }
}

#pragma endregion

SymbolManager::SymbolManager() : dproc(), aproc(), RootTable() {
  currentTable = &RootTable;
}

const SymbolTableEntry SymbolManager::getEntry(string symbol_name) {
  SymbolTableEntry entry;
  SymbolTable* search = currentTable;
  while (search != nullptr) {
    if (search->TryGetEntry(symbol_name, entry))
      return entry;
    search = search->parent;
  }
  cerr << "SymbolManager: don't find symbol with name " << symbol_name << endl;
  assert(false);
  return SymbolTableEntry();
}

void SymbolManager::InsertEntry(SymbolTableEntry entry) {
  currentTable->InsertEntry(entry);
}

void SymbolManager::PushScope() {
  SymbolTable* tmp = new SymbolTable();
  tmp->parent = currentTable;
  currentTable = tmp;
}

void SymbolManager::PopScope() {
  SymbolTable* tmp = currentTable;
  currentTable = currentTable->parent;
  delete tmp;
}

#pragma region Branch

BranchManager::BranchManager() : hasRetThisBB(false), bbPool(0), loopStack() {}

const int BranchManager::registerNewBB() {
  return bbPool++;
}

void BranchManager::PushInfo(const LoopInfo info) {
  loopStack.push_back(info);
}

void BranchManager::PopInfo() {
  loopStack.pop_back();
}

const LoopInfo& BranchManager::GetCurInfo() const {
  return loopStack.back();
}

const bool BranchManager::IsInALoop() const {
  return loopStack.size() > 0;
}

const string BranchManager::GenerateLabelFromBranchedLoop() {
  return '%' + string("unreachable_in_loop_") + to_string(registerNewBB());
}

void BranchManager::Reset() {
  assert(loopStack.size() == 0);
  bbPool = 0;
  hasRetThisBB = false;
}

#pragma endregion

#pragma region func

FuncManager::FuncManager()
    : func_name(), ret_ty(), ret_info(), symbolPool(0), func_table() {}

void FuncManager::WriteFuncPrologue() {
  // 记入函数表
  func_table.emplace(func_name, ret_ty);

  auto& setting = IRGenerator::getInstance().setting;
  auto& os = setting.getOs();

  os << "fun @" << func_name << "(";
  WriteParamsDefine();
  os << ")" << (ret_ty == VarType::e_void ? "" : ": ")  // 格式
     << GetVarType(ret_ty) << " "                       // 格式
     << "{\n"
     << setting.getIndentStr()
     << "%"
        "entry:"
     << endl;
  setting.getIndent() += 2;
  WriteAllocParams();
  return;
}

void FuncManager::WriteFuncEpilogue() {
  auto& setting = IRGenerator::getInstance().setting;
  auto& os = setting.getOs();
  os << "}\n" << endl;
  setting.getIndent() -= 2;
  return;
}

void FuncManager::WriteRetInst() {
  auto& setting = IRGenerator::getInstance().setting;
  auto& os = setting.getOs();
  os << setting.getIndentStr() << "ret " << ret_info.GetInfo() << endl;
  IRGenerator::getInstance().branchCore.hasRetThisBB = true;
}

const int FuncManager::registerNewSymbol() {
  return symbolPool++;
}

void FuncManager::InsertParam(VarType ty, string name) {
  auto& gen = IRGenerator::getInstance();
  assert(ty == VarType::e_int);
  params.push_back(
      gen.symbolCore.dproc.QuickGenEntry(SymbolType::e_var, ty, name));
}

void FuncManager::InsertParam(const SymbolTableEntry& entry) {
  params.push_back(entry);
}

void FuncManager::WriteParamsDefine() {
  auto& os = IRGenerator::getInstance().setting.getOs();
  int len = params.size();

  for (int i = 0; i < len; i++) {
    if (i != 0)
      os << ", ";
    if (params[i].var_type == VarType::e_int) {
      os << getParamVarName(params[i].var_name) << ": "
         << GetVarType(params[i].var_type);
    } else if (params[i].var_type == VarType::e_ptr) {
      os << getParamVarName(params[i].var_name) << ": "
         << params[i].arr_info.GetPtrType();
    }
  }
}

void FuncManager::WriteAllocParams() {
  auto& gen = IRGenerator::getInstance();
  for (auto it = params.begin(); it != params.end(); it++) {
    SymbolTableEntry entry(*it);
    if (entry.var_type == VarType::e_int) {
      gen.WriteAllocInst(entry);
      gen.symbolCore.InsertEntry(entry);
      RetInfo tmp(getParamVarName(it->var_name));
      gen.WriteStoreInst(tmp, entry);
    } else if (entry.var_type == VarType::e_ptr) {
      gen.WriteAllocPtrInst(entry);
      gen.symbolCore.InsertEntry(entry);
      RetInfo tmp(getParamVarName(it->var_name));
      gen.WriteStoreInst(tmp, entry);
    } else {
      assert(false);
    }
  }
}

void FuncManager::Reset() {
  func_name = string();
  ret_ty = VarType::e_int;
  ret_info = RetInfo();
  symbolPool = 0;
  params.clear();
}

void FuncManager::SetDefaultRetInfo() {
  switch (ret_ty) {
    case VarType::e_int:
      ret_info = RetInfo(0);
      break;
    case VarType::e_void:
    default:
      ret_info = RetInfo();
  }
}

const map<string, VarType>& FuncManager::GetFuncTable() const {
  return func_table;
}

void FuncManager::AddLibFuncs() {
  func_table.emplace(string("getint"), VarType::e_int);
  func_table.emplace(string("getch"), VarType::e_int);
  func_table.emplace(string("getarray"), VarType::e_int);
  func_table.emplace(string("putint"), VarType::e_void);
  func_table.emplace(string("putch"), VarType::e_void);
  func_table.emplace(string("putarray"), VarType::e_void);
  func_table.emplace(string("starttime"), VarType::e_void);
  func_table.emplace(string("stoptime"), VarType::e_void);
}

const string FuncManager::getParamVarName(const string& name) const {
  // %符号命名不会和默认@符号冲突，且变量名不可能是数字
  return "%" + name;
}
#pragma endregion

#pragma region arr init

ArrInitManager::ArrInitManager()
    : root(ArrInitNode::node_t::e_arr), current(&root), should_insert(false) {}

void ArrInitManager::PushInfo(const RetInfo& info) {
  ArrInitNode val(info);
  val.parent = current;
  current->nodes.push_back(val);
}

void ArrInitManager::PushArr() {
  ArrInitNode arr(ArrInitNode::node_t::e_arr);
  arr.parent = current;
  current->nodes.push_back(arr);
  current = &(current->nodes[current->nodes.size() - 1]);
}

void ArrInitManager::PopArr() {
  current = current->parent;
}

void ArrInitManager::Clear() {
  root = ArrInitNode(ArrInitNode::node_t::e_arr);
  current = &root;
  should_insert = false;
}

const vector<RetInfo> ArrInitManager::GetInits(const ArrInfo& shape) {
  vector<RetInfo> ret;
  RecursionGetInits(root, shape, ret);
  return ret;
}

void ArrInitManager::GetInitString(const ArrInfo& arr,
                                   const vector<RetInfo>& inits,
                                   stringstream& output) {
  int dim = arr.Dim();
  if (dim == 1) {
    // 1d
    // int a[3] = {1,2,3}
    output << '{';
    for (int i = 0; i < arr.shape[0]; i++) {
      if (i != 0)
        output << ", ";
      output << inits[i].GetInfo();
    }
    output << '}';
  } else {
    // nd
    // int a[3][2][2] = {{{0,0},{0,0}},{{0,0},{0,0}},{{0,0},{0,0}}}
    output << '{';

    ArrInfo new_info = arr.GetFrag(1, dim);
    vector<RetInfo> new_inits;
    for (int i = 0; i < arr.shape[0]; i++) {
      if (i != 0)
        output << ", ";
      // 截取数据
      auto begin = inits.begin() + i * new_info.GetSize();
      auto end = begin + new_info.GetSize();
      new_inits.assign(begin, end);
      GetInitString(new_info, new_inits, output);
    }
    output << '}';
  }
}

void ArrInitManager::RecursionGetInits(const ArrInitNode& node,
                                       const ArrInfo& arr,
                                       vector<RetInfo>& appendto) {
  // 记录这个shape中已经初始化的元素个数
  int has_inited = 0;
  // 记录数组dim
  // shape = (len 1, len 2, len 3, ..., len n)
  int dim = arr.Dim();
  // 这个数组应该初始化的元素数
  int should_init = arr.GetSize();

  // 初始化这个子数组中的所有元素
  for (auto n : node.nodes) {
    if (n.ty == ArrInitNode::e_value) {
      // 元素：视为最底层
      appendto.push_back(RetInfo(n.value));
      has_inited++;

    } else if (n.ty == ArrInitNode::e_arr) {
      // 数组

      // 计算当前对齐到的最长的边界
      // 考虑形如 {{1}, 2}：1对齐值为0，对齐到len 2处
      // 最长边界计算上界为len 2, i > 0
      int align = 1;
      int i = dim - 1;

      // 当前已经填充完毕的元素的个数必须是len n的整数倍
      assert(has_inited % arr.shape[i] == 0);
      for (; i > 0; i--) {
        align *= arr.shape[i];
        if (has_inited % align != 0) {
          break;
        }
      }

      // 恢复到对齐的最长边界; 自然退出时i = 0, 也要+1
      i++;

      // 计算这个子数组的新shape: 从i到dim
      ArrInfo fragment = arr.GetFrag(i, dim);

      // 处理这个数组
      RecursionGetInits(n, fragment, appendto);
      // 已处理元素个数累加上数组元素个数
      has_inited += fragment.GetSize();
    }
  }

  // 给数组补零
  while (has_inited < should_init) {
    appendto.push_back(RetInfo(0));
    has_inited++;
  }
}

#pragma endregion

IRGenerator::IRGenerator()
    : symbolCore(), branchCore(), funcCore(), arrinitCore() {
  setting.setOs(cout).setIndent(0);
}

IRGenerator& IRGenerator::getInstance() {
  static IRGenerator gen;
  return gen;
}

#pragma region lv3

void IRGenerator::WriteFuncPrologue() {
  funcCore.WriteFuncPrologue();
}

void IRGenerator::WriteFuncEpilogue() {
  funcCore.WriteFuncEpilogue();
}

void IRGenerator::WriteRetInst() {
  funcCore.WriteRetInst();
}

const RetInfo IRGenerator::WriteUnaryInst(const RetInfo& left, OpID op) {
  if (op == OpID::UNARY_POS) {
    return left;
  }
  // 只有两种运算
  // 1. a = -b，等效于 a = 0 - b，推出b，加入0，推入b，调用sub
  // 2. a = !b，等效于a = 0 == b，推出b，加入0，推入b，调用eq
  else if (op == OpID::UNARY_NEG) {
    return WriteBinaryInst(RetInfo(0), left, OpID::BI_SUB);
  } else {
    return WriteBinaryInst(RetInfo(0), left, OpID::LG_EQ);
  }
}

const RetInfo IRGenerator::WriteBinaryInst(const RetInfo& left,
                                           const RetInfo& right,
                                           OpID op) {
  ostream& os = setting.getOs();

  // const expr
  if (left.ty == RetInfo::retty_t::ty_int &&
      right.ty == RetInfo::retty_t::ty_int) {
    return RetInfo(calcConstExpr(left.GetValue(), right.GetValue(), op));
  }

  const string newSymbolName = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << newSymbolName << " = " << BiOp2koopa(op)
     << ' ' << left.GetInfo() << ", " << right.GetInfo() << endl;
  return RetInfo(newSymbolName);
}

const RetInfo IRGenerator::WriteLogicInst(const RetInfo& left,
                                          const RetInfo& right,
                                          OpID op) {
  // left -> bool
  const RetInfo n_left = WriteBinaryInst(RetInfo(0), left, OpID::LG_NEQ);
  const RetInfo n_right = WriteBinaryInst(RetInfo(0), right, OpID::LG_NEQ);
  return WriteBinaryInst(n_left, n_right, op);
}

#pragma endregion

#pragma region lv4

void IRGenerator::WriteAllocInst(const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();
  os << setting.getIndentStr() << entry.GetAllocInst() << endl;
}

const RetInfo IRGenerator::WriteLoadInst(const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();
  const string newSymbolName = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << entry.GetLoadInst(newSymbolName) << endl;
  return RetInfo(newSymbolName);
}

void IRGenerator::WriteStoreInst(const RetInfo& value,
                                 const SymbolTableEntry& entry) {
  ostream& os = setting.getOs();

  os << setting.getIndentStr();
  if (value.ty == RetInfo::ty_int) {
    // 常量赋值
    os << entry.GetStoreInst(value.GetValue()) << endl;
  } else if (value.ty == RetInfo::ty_sbl) {
    // 符号赋值
    os << entry.GetStoreInst(value.GetSym()) << endl;
  }
}

#pragma endregion

#pragma region lv6

void IRGenerator::WriteBasicBlockPrologue(const int& bb_id) {
  ostream& os = getInstance().setting.getOs();
  os << "\n%" << getLabelName(bb_id) << ":" << endl;
}

void IRGenerator::WriteBrInst(const RetInfo& cond, IfInfo& info) {
  ostream& os = getInstance().setting.getOs();
  switch (info.ty) {
    case IfInfo::ifty_t::i:
      info.then_label = branchCore.registerNewBB();
      info.next_label = branchCore.registerNewBB();
      os << setting.getIndentStr() << "br " << cond.GetInfo() << ", "
         << getLabelName(info.then_label) << ", "
         << getLabelName(info.next_label) << endl;
      break;
    case IfInfo::ifty_t::ie:
    default:
      info.then_label = branchCore.registerNewBB();
      info.else_label = branchCore.registerNewBB();
      info.next_label = branchCore.registerNewBB();
      os << setting.getIndentStr() << "br " << cond.GetInfo() << ", "
         << getLabelName(info.then_label) << ", "
         << getLabelName(info.else_label) << endl;
      break;
  }
  return;
}

void IRGenerator::WriteJumpInst(const int& labelID) {
  WriteJumpInst(getLabelName(labelID));
}

void IRGenerator::WriteJumpInst(const string& labelName) {
  ostream& os = getInstance().setting.getOs();
  os << setting.getIndentStr() << "jump " << labelName << endl;
}

void IRGenerator::WriteLabel(const int& BBId) {
  WriteLabel(getLabelName(BBId));
}

void IRGenerator::WriteLabel(const string& labelName) {
  ostream& os = getInstance().setting.getOs();
  os << "\n" << labelName << ":" << endl;
  // 新块的开始
  branchCore.hasRetThisBB = false;
}

const string IRGenerator::registerShortCircuitVar() {
  return string("if_") + to_string(registerNewVar());
}

#pragma endregion

#pragma region lv7

void IRGenerator::InitLoopInfo(LoopInfo& info) {
  info.cond_label = branchCore.registerNewBB();
}

void IRGenerator::WriteBrInst(const RetInfo& cond, LoopInfo& loopInfo) {
  ostream& os = getInstance().setting.getOs();
  loopInfo.body_label = branchCore.registerNewBB();
  loopInfo.next_label = branchCore.registerNewBB();
  os << setting.getIndentStr() << "br " << cond.GetInfo() << ", "
     << getLabelName(loopInfo.body_label) << ", "
     << getLabelName(loopInfo.next_label) << endl;
}

#pragma endregion

#pragma region lv8

#pragma endregion

const RetInfo IRGenerator::WriteCallInst(const string& func_name,
                                         const vector<RetInfo>& params) {
  auto& os = setting.getOs();
  os << setting.getIndentStr();

  // 查函数表，保存值
  bool hasRetValue = false;
  string newSymbolName;
  auto& funcTbl = funcCore.GetFuncTable();
  if (funcTbl.at(func_name) == VarType::e_int) {
    newSymbolName = getSymbolName(funcCore.registerNewSymbol());
    os << newSymbolName << " = ";
    hasRetValue = true;
  }

  int len = params.size();
  os << "call @" << func_name << "(";

  for (int i = 0; i < len; i++) {
    if (i != 0)
      os << ", ";
    os << params[i].GetInfo();
  }

  os << ")" << endl;

  if (hasRetValue) {
    return RetInfo(newSymbolName);
  } else {
    return RetInfo();
  }
}

void IRGenerator::WriteLibFuncDecl() {
  auto& os = setting.getOs();
  os <<
      R"EOF(decl @getint(): i32
decl @getch(): i32
decl @getarray(*i32): i32
decl @putint(i32)
decl @putch(i32)
decl @putarray(i32, *i32)
decl @starttime()
decl @stoptime())EOF"
     << endl;
  funcCore.AddLibFuncs();
}

void IRGenerator::WriteGlobalVar(const SymbolTableEntry& entry,
                                 const RetInfo& init) {
  auto& os = setting.getOs();
  os << "global " << entry.GetAllocName() << " = alloc "
     << GetVarType(entry.var_type) << ", ";
  if (init.ty == RetInfo::ty_void) {
    os << "zeroinit" << endl;
  } else {
    os << init.GetValue() << endl;
  }
}

#pragma endregion

#pragma region lv9

void IRGenerator::WriteGlobalArrVar(const SymbolTableEntry& entry) {
  assert(entry.var_type == VarType::e_arr);

  auto& os = setting.getOs();
  auto init = arrinitCore.GetInits(entry.arr_info);

  // 定义
  os << "global " << entry.GetAllocName() << " = alloc "
     << entry.arr_info.GetType() << ", ";
  // 初始化
  int init_len = init.size();
  if (init_len == 0) {
    // 使用zeroinit
    os << "zeroinit" << endl;
  } else {
    stringstream ss;
    arrinitCore.GetInitString(entry.arr_info, init, ss);
    os << ss.str() << endl;
  }

  arrinitCore.Clear();
}

void IRGenerator::WriteAllocArrInst(const SymbolTableEntry& entry,
                                    const bool& has_init) {
  assert(entry.var_type == VarType::e_arr);

  const string indent = setting.getIndentStr();
  auto& os = setting.getOs();
  auto init = arrinitCore.GetInits(entry.arr_info);
  int size = entry.arr_info.GetSize();

  // 定义
  os << indent << entry.GetAllocName() << " = alloc "
     << entry.arr_info.GetType() << endl;

  if (!has_init) {
    // 不初始化
  } else {
    // 动态地址
    int dim = entry.arr_info.Dim();
    vector<int> cur_addr(dim);

    for (int i = 0; i < size; i++) {
      const string addr_1 =
          WriteGetPtrFromArrInt(entry.GetAllocName(), cur_addr);

      os << indent << "store " << init[i].GetInfo() << ", " << addr_1 << endl;

      // 更新地址
      int j = dim - 1;
      cur_addr[j]++;
      // 不用检查第一维
      for (; j > 0; j--) {
        if (cur_addr[j] >= entry.arr_info.shape[j]) {
          cur_addr[j - 1]++;
          cur_addr[j] -= entry.arr_info.shape[j];
        } else {
          // 一次不满足就直接跳出
          break;
        }
      }
    }
  }

  arrinitCore.Clear();
}

void IRGenerator::WriteGetelemptrInst(const string& symbol,
                                      const string& arr_var,
                                      const string& addr) const {
  auto& os = setting.getOs();
  const string indent = setting.getIndentStr();
  os << indent << symbol << " = getelemptr " << arr_var << ", " << addr << endl;
}

const string IRGenerator::WriteGetPtrFromArr(const string& arr_var,
                                             const vector<RetInfo>& addr) {
  string addr_1 = getSymbolName(funcCore.registerNewSymbol());
  string addr_2 = arr_var;

  int len = addr.size();
  for (int i = 0; i < len; i++) {
    WriteGetelemptrInst(addr_1, addr_2, addr[i].GetInfo());
    addr_2 = addr_1;
    if (i != len - 1)
      addr_1 = getSymbolName(funcCore.registerNewSymbol());
  }
  return addr_1;
}

const string IRGenerator::WriteGetPtrFromArrInt(const string& arr_var,
                                                const vector<int>& addr) {
  string addr_1 = getSymbolName(funcCore.registerNewSymbol());
  string addr_2 = arr_var;
  int len = addr.size();
  for (int i = 0; i < len; i++) {
    WriteGetelemptrInst(addr_1, addr_2, to_string(addr[i]));
    addr_2 = addr_1;
    if (i != len - 1)
      addr_1 = getSymbolName(funcCore.registerNewSymbol());
  }
  return addr_1;
}

const RetInfo IRGenerator::WriteLoadArrInst(const SymbolTableEntry& entry,
                                            const vector<RetInfo>& addr) {
  auto& os = setting.getOs();

  const string addr_1 = WriteGetPtrFromArr(entry.GetAllocName(), addr);

  const string ret = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << ret << " = load " << addr_1 << endl;
  return RetInfo(ret);
}

void IRGenerator::WriteStoreArrInst(const SymbolTableEntry& entry,
                                    const RetInfo& value,
                                    const vector<RetInfo>& addr) {
  auto& os = setting.getOs();

  const string addr_1 = WriteGetPtrFromArr(entry.GetAllocName(), addr);

  const string ret = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << "store " << value.GetInfo() << ", " << addr_1
     << endl;
  return;
}

void IRGenerator::WriteGetPtrInst(const string& symbol,
                                  const string& arr_var,
                                  const string& addr) const {
  auto& os = setting.getOs();
  const string indent = setting.getIndentStr();
  os << indent << symbol << " = getptr " << arr_var << ", " << addr << endl;
}

void IRGenerator::WriteAllocPtrInst(const SymbolTableEntry& entry) {
  assert(entry.var_type == VarType::e_ptr);
  auto& os = setting.getOs();

  // 定义
  os << setting.getIndentStr() << entry.GetAllocName() << " = alloc "
     << entry.arr_info.GetPtrType() << endl;
}

const string IRGenerator::WriteGetPtrFromPtr(const string& arr_var,
                                             const vector<RetInfo>& addr) {
  // 必须有坐标
  assert(addr.size() >= 1);
  string addr_1 = getSymbolName(funcCore.registerNewSymbol());
  string addr_2 = arr_var;
  int len = addr.size();
  WriteGetPtrInst(addr_1, addr_2, addr[0].GetInfo());

  for (int i = 1; i < len; i++) {
    WriteGetelemptrInst(addr_1, addr_2, addr[i].GetInfo());
    addr_2 = addr_1;
    if (i != len - 1)
      addr_1 = getSymbolName(funcCore.registerNewSymbol());
  }
  return addr_1;
}

const RetInfo IRGenerator::WriteLoadPtrInst(const SymbolTableEntry& entry,
                                            const vector<RetInfo>& addr) {
  auto& os = setting.getOs();

  const string addr_1 = WriteGetPtrFromPtr(entry.GetAllocName(), addr);

  const string ret = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << ret << " = load " << addr_1 << endl;
  return RetInfo(ret);
}

void IRGenerator::WriteStorePtrInst(const SymbolTableEntry& entry,
                                    const RetInfo& value,
                                    const vector<RetInfo>& addr) {
  auto& os = setting.getOs();

  const string addr_1 = WriteGetPtrFromPtr(entry.GetAllocName(), addr);

  const string ret = getSymbolName(funcCore.registerNewSymbol());
  os << setting.getIndentStr() << "store " << value.GetInfo() << ", " << addr_1
     << endl;
  return;
}

const int IRGenerator::registerNewVar() {
  return symbolCore.dproc.RegisterVar();
}

#pragma endregion

#pragma region private

const string IRGenerator::getSymbolName(const int& symbol) const {
  return string("%") + to_string(symbol);
}

const string IRGenerator::getLabelName(const int& bb_id) const {
  return string("%") + string("label_") + to_string(bb_id);
}

int IRGenerator::calcConstExpr(const int& l, const int& r, OpID op) {
  switch (op) {
    case BI_ADD:
      return l + r;
    case BI_SUB:
      return l - r;
    case BI_MUL:
      return l * r;
    case BI_DIV:
      return l / r;
    case BI_MOD:
      return l % r;
    case LG_GT:
      return l > r;
    case LG_GE:
      return l >= r;
    case LG_LT:
      return l < r;
    case LG_LE:
      return l <= r;
    case LG_EQ:
      return l == r;
    case LG_NEQ:
      return l != r;
    case LG_AND:
      return l && r;
    case LG_OR:
      return l || r;
    default:
      return 0;
  }
  return 0;
}

#pragma endregion

}  // namespace ir