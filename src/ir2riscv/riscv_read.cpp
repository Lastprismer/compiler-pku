#include "riscv_read.h"

namespace riscv {

void visit_program(const koopa_raw_program_t& program) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  visit_slice(program.values);
  // 访问所有函数=
  visit_slice(program.funcs);
}

void visit_slice(const koopa_raw_slice_t& slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        visit_func(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        visit_basic_block(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        visit_value(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
        break;
    }
  }
}

void visit_func(const koopa_raw_function_t& func) {
  // 处理函数定义
  if (func->bbs.len == 0) {
    return;
  }
  // 访问所有基本块
  RiscvGenerator& gen = RiscvGenerator::getInstance();

  gen.Clear();

  gen.funcCore.func_name = ParseSymbol(func->name);
  CalcMemoryNeeded(func);
  gen.funcCore.WritePrologue();

  visit_slice(func->bbs);
}

void visit_basic_block(const koopa_raw_basic_block_t& bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  // os << "basic block: name: " << bb->name << endl;
  auto& gen = RiscvGenerator::getInstance();
  gen.bbCore.WriteBBName(bb->name);
  visit_slice(bb->insts);
}

void visit_value(const koopa_raw_value_t& value) {
  //  根据指令类型判断后续需要如何访问
  const auto& kind = value->kind;
  // cout << GetTypeString(value);
  // cout << kind.tag;
  switch (kind.tag) {
    case KOOPA_RVT_INTEGER:
      // cout << "  int" << endl;
      // visit_inst_int(kind.data.integer);
      break;
    case KOOPA_RVT_ALLOC:
      // cout << "  alloc" << endl;
      // visit_inst_alloc(value);
      break;
    case KOOPA_RVT_LOAD:
      // cout << "  load" << endl;
      visit_inst_load(value);
      break;
    case KOOPA_RVT_STORE:
      // cout << "  store" << endl;
      visit_inst_store(value);
      break;
    case KOOPA_RVT_BINARY:
      // cout << "  binary" << endl;
      visit_inst_binary(value);
      break;
    case KOOPA_RVT_BRANCH:
      // cout << "  branch" << endl;
      visit_inst_branch(value);
      break;
    case KOOPA_RVT_JUMP:
      // cout << "  jump" << endl;
      visit_inst_jump(value);
      break;
    case KOOPA_RVT_CALL:
      // cout << "  call" << endl;
      visit_inst_call(value);
    case KOOPA_RVT_RETURN:
      // cout << "  ret" << endl;
      visit_inst_ret(kind.data.ret);
      break;
    default:
      // cout << "  what?" << endl;
      break;
      // 其他类型暂时遇不到
  }
}

#pragma region visit inst

void visit_inst_int(const koopa_raw_integer_t& inst_int) {
  cerr << "NOT EXPECTED" << endl;
}

void visit_inst_alloc(const koopa_raw_value_t& value) {
  cerr << "NOT EXPECTED" << endl;
}

void visit_inst_load(const koopa_raw_value_t& inst_load) {
  RiscvGenerator& gen = RiscvGenerator::getInstance();
  StackMemoryModule& stack_core = gen.stackCore;
  if (stack_core.InstResult.find(inst_load->kind.data.load.src) !=
      stack_core.InstResult.end()) {
    stack_core.InstResult[inst_load] =
        stack_core.InstResult[inst_load->kind.data.load.src];
    return;
  }
  assert(false);
}

void visit_inst_store(const koopa_raw_value_t& inst) {
  const auto& inst_store = inst->kind.data.store;
  auto& gen = RiscvGenerator::getInstance();
  auto& stack_core = gen.stackCore;

  InstResultInfo dest, src;
  if (stack_core.InstResult.find(inst_store.dest) !=
      stack_core.InstResult.end()) {
    dest = stack_core.InstResult.at(inst_store.dest);
  } else {
    // 存进栈里
    dest.ty = ValueType::e_stack;
    dest.content.addr = stack_core.IncreaseStackUsed();
    stack_core.InstResult[inst_store.dest] = dest;
  }

  if (inst_store.value->kind.tag == KOOPA_RVT_INTEGER) {
    // 加载立即数
    src.ty = ValueType::e_imm;
    src.content.imm = inst_store.value->kind.data.integer.value;

  } else if (inst_store.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
    // 加载函数参数
    int argid = inst_store.value->kind.data.func_arg_ref.index;
    InstResultInfo src = GetParamPosition(argid);
    if (argid >= 8) {
      // 到上一个函数的栈帧里找，也就是将地址加上自己分配的空间
      src.content.addr += stack_core.stack_memory;
    }

  } else {
    // src不应该是stack
    auto srcResult = stack_core.InstResult.at(inst_store.value);
    src.ty = srcResult.ty;
    if (src.ty == ValueType::e_reg)
      src.content.reg = srcResult.content.reg;
    else
      src.content.addr = srcResult.content.addr;
  }

  stack_core.WriteStoreInst(src, dest);
}

void visit_inst_binary(const koopa_raw_value_t& inst) {
  const koopa_raw_binary_t& inst_bina = inst->kind.data.binary;
  RiscvGenerator& gen = RiscvGenerator::getInstance();
  auto& stack_core = gen.stackCore;
  Reg r1 = GetValueResult(inst_bina.lhs);
  Reg r2 = GetValueResult(inst_bina.rhs);
  RiscvGenerator::getInstance().WriteBinaInst(inst_bina.op, r1, r2);

  // 把r1的内容存回内存，返回地址?
  InstResultInfo dest(ValueType::e_stack, stack_core.IncreaseStackUsed()),
      src(r1);
  stack_core.WriteStoreInst(src, dest);
  if (stack_core.InstResult.find(inst_bina.rhs) !=
          stack_core.InstResult.end() ||
      inst_bina.rhs->kind.tag == KOOPA_RVT_INTEGER) {  // 保存过或是立即数
    // r2是临时分配的
    gen.regCore.ReleaseReg(r2);
  }
  stack_core.InstResult.emplace(inst, dest);
}

void visit_inst_branch(const koopa_raw_value_t& inst) {
  auto& gen = RiscvGenerator::getInstance();
  auto& stack_core = gen.stackCore;
  auto& branch = inst->kind.data.branch;
  Reg cond = GetValueResult(branch.cond);
  gen.bbCore.WriteBranch(cond, string(branch.true_bb->name),
                         string(branch.false_bb->name));

  if (stack_core.InstResult.find(branch.cond) != stack_core.InstResult.end() ||
      branch.cond->kind.tag == KOOPA_RVT_INTEGER) {  // 保存过或是立即数
    // cond是临时分配的
    gen.regCore.ReleaseReg(cond);
  }
}

void visit_inst_jump(const koopa_raw_value_t& inst) {
  auto& gen = RiscvGenerator::getInstance();
  gen.bbCore.WriteJumpInst(inst->kind.data.jump.target->name);
}

void visit_inst_call(const koopa_raw_value_t& inst) {
  auto& gen = RiscvGenerator::getInstance();
  auto& stack_core = gen.stackCore;
  const auto& inst_call = inst->kind.data.call;
  int len = inst_call.args.len;
  set<Reg> reg_used;

  for (int i = 0; i < len; i++) {
    auto value = reinterpret_cast<koopa_raw_value_t>(inst_call.args.buffer[i]);

    // 参数一定是已经计算过并存在栈里的了
    assert(stack_core.InstResult.find(value) != stack_core.InstResult.end());

    auto dest = GetParamPosition(i);
    if (dest.ty == ValueType::e_reg) {
      gen.regCore.GetReg(dest.content.reg);
      reg_used.insert(dest.content.reg);
    }
    stack_core.WriteStoreInst(stack_core.InstResult.at(value), dest);
  }
  gen.funcCore.WriteCallInst(ParseSymbol(inst_call.callee->name));
  for (auto r : reg_used) {
    gen.regCore.ReleaseReg(r);
  }

  // 处理函数返回值，计入栈
  gen.regCore.GetReg(Reg::a0);
  InstResultInfo src(Reg::a0),
      dest(ValueType::e_stack, stack_core.IncreaseStackUsed());
  stack_core.WriteStoreInst(src, dest);
  gen.regCore.ReleaseReg(Reg::a0);
  stack_core.InstResult.emplace(inst, dest);
}

void visit_inst_ret(const koopa_raw_return_t& inst_ret) {
  RiscvGenerator& gen = RiscvGenerator::getInstance();
  auto& instResult = gen.stackCore.InstResult;
  InstResultInfo info;

  if (instResult.find(inst_ret.value) != instResult.end())
    info = instResult[inst_ret.value];
  else if (inst_ret.value->kind.tag == KOOPA_RVT_INTEGER) {
    info.ty = ValueType::e_imm;
    info.content.imm = inst_ret.value->kind.data.integer.value;
  }
  gen.funcCore.WriteEpilogue(info);
}

#pragma endregion

#pragma region util

void CalcMemoryNeeded(const koopa_raw_function_t& func) {
  int alloc_size = 0;
  // 遍历函数所有的bb中的所有指令
  const koopa_raw_slice_t& func_bbs = func->bbs;
  // 计算ra保存
  bool has_call_inst = false;
  // 计算为其他函数调用参数分配的空间
  int max_called_func_params = 0;

  assert(func_bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
  for (size_t i = 0; i < func_bbs.len; ++i) {
    const koopa_raw_basic_block_t& bb =
        reinterpret_cast<koopa_raw_basic_block_t>(func_bbs.buffer[i]);

    assert(bb->insts.kind == KOOPA_RSIK_VALUE);
    for (size_t j = 0; j < bb->insts.len; ++j) {
      const koopa_raw_value_t& value =
          reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
      // alloc和返回值为int32的非load语句
      if ((value->ty->tag == KOOPA_RTT_INT32 &&
           value->kind.tag != KOOPA_RVT_LOAD) ||
          value->kind.tag == KOOPA_RVT_ALLOC)
        alloc_size += 4;

      if (value->kind.tag == KOOPA_RVT_CALL) {
        has_call_inst = true;
        max_called_func_params =
            max(max_called_func_params, (int)value->kind.data.call.args.len);
      }
    }
  }

  if (has_call_inst) {
    alloc_size += 4;
  }

  if (max_called_func_params > 8) {
    alloc_size += 4 * (max_called_func_params - 8);
  }

  // 向上进位到16
  std::cout << "alloc size: " << alloc_size << std::endl;
  alloc_size = (alloc_size + 15) / 16 * 16;

  auto& gen = RiscvGenerator::getInstance();
  gen.stackCore.SetStackMem(alloc_size);
  gen.funcCore.is_leaf_func = has_call_inst;
  return;
}

const Reg GetValueResult(const koopa_raw_value_t& value) {
  RiscvGenerator& gen = RiscvGenerator::getInstance();
  auto& stack_core = gen.stackCore;
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    // 处理常数
    Reg rs = gen.regCore.GetAvailableReg();
    stack_core.WriteLI(rs, value->kind.data.integer.value);
    return rs;
  }
  assert(stack_core.InstResult.find(value) != stack_core.InstResult.end());

  InstResultInfo info;
  info = stack_core.InstResult.at(value);
  if (info.ty == ValueType::e_reg) {
    cout << "will this actually run?" << endl;
    // 还没做分配策略的
    return info.content.reg;
  } else if (info.ty == ValueType::e_stack) {
    // 先读出
    Reg rs = gen.regCore.GetAvailableReg();
    stack_core.WriteLW(rs, info.content.addr);
    return rs;
  }
  assert(false);
}

const InstResultInfo GetParamPosition(const int& param_cnt) {
  switch (param_cnt) {
    case 0:
      return InstResultInfo(Reg::a0);
    case 1:
      return InstResultInfo(Reg::a1);
    case 2:
      return InstResultInfo(Reg::a2);
    case 3:
      return InstResultInfo(Reg::a3);
    case 4:
      return InstResultInfo(Reg::a4);
    case 5:
      return InstResultInfo(Reg::a5);
    case 6:
      return InstResultInfo(Reg::a6);
    case 7:
      return InstResultInfo(Reg::a7);
    default:
      return InstResultInfo(ValueType::e_stack, (param_cnt - 8) * 4);
  }
}

#pragma endregion

}  // namespace riscv