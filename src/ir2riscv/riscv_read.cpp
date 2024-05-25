#include "riscv_read.h"

namespace riscv {

void visit_program(const koopa_raw_program_t& program) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  visit_slice(program.values);
  // 访问所有函数
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
  auto& gen = RiscvGenerator::getInstance();

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
  // cout << GetTypeString(value) << ' ';
  // cout << kind.tag << endl;
  switch (kind.tag) {
    case KOOPA_RVT_INTEGER:
      // cout << "  int" << endl;
      // visit_inst_int(kind.data.integer);
      break;

    case KOOPA_RVT_ZERO_INIT:
      break;

    case KOOPA_RVT_UNDEF:
      break;

    case KOOPA_RVT_AGGREGATE:
      break;

    case KOOPA_RVT_FUNC_ARG_REF:
      break;

    case KOOPA_RVT_BLOCK_ARG_REF:
      break;

    case KOOPA_RVT_ALLOC:
      // cout << "  alloc" << endl;
      visit_inst_alloc(value);
      break;

    case KOOPA_RVT_GLOBAL_ALLOC:
      visit_inst_globalalloc(value);
      break;

    case KOOPA_RVT_LOAD:
      // cout << "  load" << endl;
      visit_inst_load(value);
      break;

    case KOOPA_RVT_STORE:
      // cout << "  store" << endl;
      visit_inst_store(value);
      break;

    case KOOPA_RVT_GET_PTR:
      visit_inst_getptr(value);
      break;

    case KOOPA_RVT_GET_ELEM_PTR:
      visit_inst_getelemptr(value);
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
      break;

    case KOOPA_RVT_RETURN:
      // cout << "  ret" << endl;
      visit_inst_ret(kind.data.ret);
      break;

    default:
      cout << "  what?" << endl;
      break;
      // 其他类型暂时遇不到
  }
}

#pragma region visit inst

void visit_inst_int(const koopa_raw_integer_t& inst_int) {
  cerr << "NOT EXPECTED" << endl;
}

void visit_inst_alloc(const koopa_raw_value_t& value) {
  auto& gen = RiscvGenerator::getInstance();
  if (value->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY) {
    ArrInfo info;
    auto base = value->ty->data.pointer.base;
    while (base->tag == KOOPA_RTT_ARRAY) {
      info.shape.insert(info.shape.begin(), base->data.array.len);
      base = base->data.array.base;
    }
    // 保存地址
    gen.stackCore.stack_used += info.GetSize();
    // 记录低地址
    info.stack_addr = gen.stackCore.stack_memory - gen.stackCore.stack_used;

    gen.arrCore.arrinfos.emplace(value, info);
  } else if (value->ty->data.pointer.base->tag == KOOPA_RTT_POINTER) {
    // 指针：当成第一维为1的数组，方便计算地址
    ArrInfo info;
    info.shape.insert(info.shape.begin(), 1);
    // 指针指向的指针的解引用，数组或int
    auto base = value->ty->data.pointer.base->data.pointer.base;
    while (base->tag == KOOPA_RTT_ARRAY) {
      info.shape.insert(info.shape.begin(), base->data.array.len);
      base = base->data.array.base;
    }
    info.stack_addr = gen.stackCore.IncreaseStackUsed();
    gen.arrCore.arrinfos.emplace(value, info);
    InstResultInfo retinfo(ValueType::e_stack, info.stack_addr);
    gen.stackCore.InstResult.emplace(value, retinfo);
  } else if (value->ty->data.pointer.base->tag == KOOPA_RTT_INT32) {
    InstResultInfo retinfo(ValueType::e_stack,
                           gen.stackCore.IncreaseStackUsed());
    gen.stackCore.InstResult.emplace(value, retinfo);
  }
}

void visit_inst_globalalloc(const koopa_raw_value_t& inst) {
  auto& gen = RiscvGenerator::getInstance();
  // 保存相关信息，声明全局变量
  const auto& inst_init = inst->kind.data.global_alloc.init;

  if (inst_init->kind.tag == KOOPA_RVT_INTEGER) {
    InitInfo info;
    info.ty = InitType::e_int;
    info.value = inst_init->kind.data.integer.value;
    gen.globalCore.WriteGlobalVarDecl(ParseSymbol(inst->name), info);

  } else if (inst_init->kind.tag == KOOPA_RVT_ZERO_INIT) {
    // zeroinit必是int
    InitInfo info;
    info.ty = InitType::e_zeroinit;
    gen.globalCore.WriteGlobalVarDecl(ParseSymbol(inst->name), info);

  } else if (inst_init->kind.tag == KOOPA_RVT_AGGREGATE) {
    // 是一个一个数组
    ArrInfo info;
    // 递归解析
    SolveArrAggInit(inst_init, info, true);
    gen.globalCore.WriteGlobalArrDecl(ParseSymbol(inst->name), info);

    // 将相关信息存进全局数组表
    // 新建一个arrinfo以消除初始化数据
    ArrInfo new_info(info.shape);
    gen.arrCore.arrinfos.emplace(inst, new_info);
  }
}

void visit_inst_load(const koopa_raw_value_t& inst) {
  auto& gen = RiscvGenerator::getInstance();
  auto& stack_core = gen.stackCore;
  auto& arr_core = gen.arrCore;

  if (inst->kind.data.load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    // 加载全局变量
    int addr = gen.globalCore.WriteLoadGlobalVar(
        ParseSymbol(inst->kind.data.load.src->name));
    // 存储其位置
    stack_core.InstResult.emplace(inst,
                                  InstResultInfo(ValueType::e_stack, addr));

    return;
  }

  else if (inst->kind.data.load.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
           inst->kind.data.load.src->kind.tag == KOOPA_RVT_GET_PTR) {
    // 手动读出来，再存到栈里
    auto& value = inst->kind.data.load.src;
    auto& os = gen.setting.getOs();

    Reg val = gen.regCore.GetAvailableReg();
    Reg realaddr = gen.regCore.GetAvailableReg();

    const auto& info = gen.stackCore.InstResult.at(value);

    gen.stackCore.WriteLW(realaddr, info.content.addr);
    // 将值load进val中
    lw(os, val, realaddr, 0);

    // 将val存到这个指令的输出中
    InstResultInfo destinfo(ValueType::e_stack, stack_core.IncreaseStackUsed());
    InstResultInfo srcinfo(val);
    stack_core.WriteDataTranfer(srcinfo, destinfo);
    stack_core.InstResult.emplace(inst, destinfo);

    gen.regCore.ReleaseReg(realaddr);
    gen.regCore.ReleaseReg(val);
    return;
  }

  else if (arr_core.arrinfos.find(inst->kind.data.load.src) !=
           arr_core.arrinfos.end()) {
    arr_core.arrinfos.emplace(inst,
                              arr_core.arrinfos.at(inst->kind.data.load.src));
    return;
  }

  else if (stack_core.InstResult.find(inst->kind.data.load.src) !=
           stack_core.InstResult.end()) {
    // 从普通的值里load

    stack_core.InstResult[inst] =
        stack_core.InstResult[inst->kind.data.load.src];
    return;
  }
  gen.setting.getOs() << "src type: " << inst->kind.data.load.src->kind.tag
                      << endl;
  assert(false);
}

void visit_inst_store(const koopa_raw_value_t& inst) {
  const auto& inst_store = inst->kind.data.store;
  auto& gen = RiscvGenerator::getInstance();
  auto& stack_core = gen.stackCore;

  InstResultInfo src, dest;

  // 选择源
  if (inst_store.value->kind.tag == KOOPA_RVT_INTEGER) {
    // 加载立即数
    src.ty = ValueType::e_imm;
    src.content.imm = inst_store.value->kind.data.integer.value;

  } else if (inst_store.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
    // 加载函数参数
    int argid = inst_store.value->kind.data.func_arg_ref.index;
    src = GetParamPosition(argid);
    if (argid >= 8) {
      // 到上一个函数的栈帧里找，也就是将地址加上自己分配的空间
      src.content.addr += stack_core.stack_memory;
    }

  } else {
    auto srcResult = stack_core.InstResult.at(inst_store.value);
    src.ty = srcResult.ty;
    if (src.ty == ValueType::e_reg) {
      src.content.reg = srcResult.content.reg;
    } else {
      src.content.addr = srcResult.content.addr;
    }
  }

  // 选择目标
  if (inst_store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    // 存储进全局变量
    // 截胡
    gen.globalCore.WriteStoreGlobalVar(ParseSymbol(inst_store.dest->name), src);
    return;
  }

  else if (stack_core.InstResult.find(inst_store.dest) !=
           stack_core.InstResult.end()) {
    dest = stack_core.InstResult.at(inst_store.dest);
    if (inst_store.dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
        inst_store.dest->kind.tag == KOOPA_RVT_GET_PTR) {
      // 截胡
      // 先处理src
      auto& os = gen.setting.getOs();
      Reg srcreg = gen.regCore.GetAvailableReg();
      switch (src.ty) {
        case ValueType::e_imm:
          li(os, srcreg, src.content.imm);
          break;
        case ValueType::e_reg:
          addi(os, srcreg, src.content.reg, 0);
          break;
        case ValueType::e_stack:
          gen.stackCore.WriteLW(srcreg, src.content.addr);
          break;
        default:
          break;
      }
      // srcreg内存储要加载进去的值
      // 加载

      Reg realaddr = gen.regCore.GetAvailableReg();
      gen.stackCore.WriteLW(realaddr, dest.content.addr);
      sw(os, realaddr, srcreg, 0);

      gen.regCore.ReleaseReg(realaddr);
      gen.regCore.ReleaseReg(srcreg);
      return;
    }
  } else {
    // 存进栈里
    dest.ty = ValueType::e_stack;
    dest.content.addr = stack_core.IncreaseStackUsed();
    stack_core.InstResult[inst_store.dest] = dest;
  }

  // 复杂
  stack_core.WriteDataTranfer(src, dest);

  // 如果源是寄存器，此处目标只会是栈，将源释放
  if (src.ty == ValueType::e_reg) {
    gen.regCore.ReleaseReg(src.content.reg);
  }
}

void visit_inst_getptr(const koopa_raw_value_t& inst) {
  const auto& inst_gep = inst->kind.data.get_elem_ptr;

  auto& gen = RiscvGenerator::getInstance();
  auto& arr_core = gen.arrCore;
  auto& stack_core = gen.stackCore;
  auto& reg_core = gen.regCore;
  auto& os = gen.setting.getOs();

  Reg regsrc = reg_core.GetAvailableReg();
  Reg addr = reg_core.GetAvailableReg();
  int sizejump = 0;

  ArrInfo& info = arr_core.arrinfos[inst_gep.src];
  arr_core.current_arr = info;
  arr_core.current_dim = 1;
  sizejump = arr_core.GetCurArrSize();

  // src加载到reg src中
  Reg tmp = reg_core.GetAvailableReg();
  assert(info.stack_addr != -1);
  if (IsImmInBound(info.stack_addr)) {
    addi(os, tmp, Reg::sp, info.stack_addr);
  } else {
    li(os, tmp, info.stack_addr);
    add(os, tmp, Reg::sp, tmp);
  }
  // getptr要多进行一次lw
  lw(os, regsrc, tmp, 0);
  reg_core.ReleaseReg(tmp);

  // index加载到addr中
  if (inst_gep.index->kind.tag == KOOPA_RVT_INTEGER) {
    li(os, addr, inst_gep.index->kind.data.integer.value);
  } else {
    stack_core.WriteDataTranfer(stack_core.InstResult.at(inst_gep.index),
                                InstResultInfo(addr));
  }

  // 计算地址偏移
  Reg tmp1 = reg_core.GetAvailableReg();
  li(os, tmp1, sizejump);
  mul(os, addr, addr, tmp1);
  reg_core.ReleaseReg(tmp1);

  // 计算地址
  // 最后存在src中
  add(os, regsrc, regsrc, addr);

  // 存回内存，返回值应该是地址
  InstResultInfo destinfo(ValueType::e_stack, stack_core.IncreaseStackUsed());
  InstResultInfo srcinfo(regsrc);
  // 走reg -> stack
  stack_core.WriteDataTranfer(srcinfo, destinfo);
  // 保存指令返回值
  stack_core.InstResult.emplace(inst, destinfo);

  reg_core.ReleaseReg(addr);
  reg_core.ReleaseReg(regsrc);
}

void visit_inst_getelemptr(const koopa_raw_value_t& inst) {
  const auto& inst_gep = inst->kind.data.get_elem_ptr;

  auto& gen = RiscvGenerator::getInstance();
  auto& arr_core = gen.arrCore;
  auto& stack_core = gen.stackCore;
  auto& reg_core = gen.regCore;
  auto& os = gen.setting.getOs();

  Reg src = reg_core.GetAvailableReg();
  Reg addr = reg_core.GetAvailableReg();
  int sizejump = 0;

  if (inst_gep.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    ArrInfo& info = arr_core.arrinfos[inst_gep.src];
    arr_core.current_arr = info;
    arr_core.current_dim = 1;
    sizejump = arr_core.GetCurArrSize();

    // src加载到reg src中
    la(os, src, ParseSymbol(inst_gep.src->name));

    // index加载到addr中
    if (inst_gep.index->kind.tag == KOOPA_RVT_INTEGER) {
      li(os, addr, inst_gep.index->kind.data.integer.value);
    } else {
      stack_core.WriteDataTranfer(stack_core.InstResult.at(inst_gep.index),
                                  InstResultInfo(addr));
    }

  } else if (inst_gep.src->kind.tag == KOOPA_RVT_ALLOC) {
    ArrInfo& info = arr_core.arrinfos[inst_gep.src];
    arr_core.current_arr = info;
    arr_core.current_dim = 1;
    sizejump = arr_core.GetCurArrSize();

    // src加载到reg src中
    assert(info.stack_addr != -1);
    if (IsImmInBound(info.stack_addr)) {
      addi(os, src, Reg::sp, info.stack_addr);
    } else {
      Reg tmp = reg_core.GetAvailableReg();
      li(os, tmp, info.stack_addr);
      add(os, src, Reg::sp, tmp);
      reg_core.ReleaseReg(tmp);
    }

    // index加载到addr中
    if (inst_gep.index->kind.tag == KOOPA_RVT_INTEGER) {
      li(os, addr, inst_gep.index->kind.data.integer.value);
    } else {
      stack_core.WriteDataTranfer(stack_core.InstResult.at(inst_gep.index),
                                  InstResultInfo(addr));
    }

  } else if (inst_gep.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
             inst_gep.src->kind.tag == KOOPA_RVT_GET_PTR) {
    // 直接利用arr_core中的信息
    // 因为生成koopaIR中，对一个数组的下标解算是连续的gep / gp指令

    // 累加current_dim
    arr_core.current_dim++;
    sizejump = arr_core.GetCurArrSize();

    // src: 从栈中获取
    stack_core.WriteDataTranfer(stack_core.InstResult.at(inst_gep.src),
                                InstResultInfo(src));

    // index加载到addr中
    if (inst_gep.index->kind.tag == KOOPA_RVT_INTEGER) {
      li(os, addr, inst_gep.index->kind.data.integer.value);
    } else {
      stack_core.WriteDataTranfer(stack_core.InstResult.at(inst_gep.index),
                                  InstResultInfo(addr));
    }
  }

  // 计算地址偏移
  Reg tmp1 = reg_core.GetAvailableReg();
  li(os, tmp1, sizejump);
  mul(os, addr, addr, tmp1);
  reg_core.ReleaseReg(tmp1);

  // 计算地址
  // 最后存在src中
  add(os, src, src, addr);

  // 存回内存，返回值应该是地址
  InstResultInfo destinfo(ValueType::e_stack, stack_core.IncreaseStackUsed());
  InstResultInfo srcinfo(src);
  // 走reg -> stack
  stack_core.WriteDataTranfer(srcinfo, destinfo);
  // 保存指令返回值
  stack_core.InstResult.emplace(inst, destinfo);

  reg_core.ReleaseReg(addr);
  reg_core.ReleaseReg(src);
}

void visit_inst_binary(const koopa_raw_value_t& inst) {
  const koopa_raw_binary_t& inst_bina = inst->kind.data.binary;
  auto& gen = RiscvGenerator::getInstance();
  auto& stack_core = gen.stackCore;
  Reg r1 = GetValueResult(inst_bina.lhs);
  Reg r2 = GetValueResult(inst_bina.rhs);
  RiscvGenerator::getInstance().WriteBinaInst(inst_bina.op, r1, r2);

  // 把r1的内容存回内存，返回地址?
  InstResultInfo dest(ValueType::e_stack, stack_core.IncreaseStackUsed()),
      src(r1);
  // 走reg -> stack
  stack_core.WriteDataTranfer(src, dest);

  gen.regCore.ReleaseReg(r1);
  gen.regCore.ReleaseReg(r2);
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

    InstResultInfo src;

    // 参数没有保存：立即数传参
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
      src = InstResultInfo(ValueType::e_imm, value->kind.data.integer.value);
    } else if (stack_core.InstResult.find(value) !=
               stack_core.InstResult.end()) {
      src = stack_core.InstResult.at(value);
    } else {
      // 还是没有存在栈里？
      assert(false);
    }

    auto dest = GetParamPosition(i);
    if (dest.ty == ValueType::e_reg) {
      gen.regCore.GetReg(dest.content.reg);
      reg_used.insert(dest.content.reg);
    }
    // imm -> reg 或 stack -> reg
    stack_core.WriteDataTranfer(src, dest);
  }
  gen.funcCore.WriteCallInst(ParseSymbol(inst_call.callee->name));
  for (auto r : reg_used) {
    gen.regCore.ReleaseReg(r);
  }

  // 处理函数返回值，计入栈
  if (inst_call.callee->ty->data.function.ret->tag == KOOPA_RTT_UNIT) {
    // 函数返回void
    return;
  }
  gen.regCore.GetReg(Reg::a0);
  InstResultInfo src(Reg::a0),
      dest(ValueType::e_stack, stack_core.IncreaseStackUsed());

  // reg -> stack
  stack_core.WriteDataTranfer(src, dest);
  gen.regCore.ReleaseReg(Reg::a0);
  stack_core.InstResult.emplace(inst, dest);
}

void visit_inst_ret(const koopa_raw_return_t& inst_ret) {
  auto& gen = RiscvGenerator::getInstance();
  auto& instResult = gen.stackCore.InstResult;
  InstResultInfo info;

  if (inst_ret.value == nullptr) {
    // 返回值为void
    info.ty = ValueType::e_unused;
  } else if (instResult.find(inst_ret.value) != instResult.end()) {
    // 返回值在变量中
    info = instResult[inst_ret.value];
  } else if (inst_ret.value->kind.tag == KOOPA_RVT_INTEGER) {
    // 返回值为立即数
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

      // 非void语句
      // 根据返回值计算

      auto base = value->ty;
      // alloc，计算指向的数据大小
      if (value->kind.tag == KOOPA_RVT_ALLOC) {
        base = base->data.pointer.base;
        int arr_len = 1;
        // 递归计算数组
        while (base->tag == KOOPA_RTT_ARRAY) {
          arr_len *= base->data.array.len;
          base = base->data.array.base;
        }
        alloc_size += 4 * arr_len;
      } else if (base->tag != KOOPA_RTT_UNIT) {
        alloc_size += 4;
      }

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
  std::cerr << "alloc size: " << alloc_size << std::endl;
  alloc_size = (alloc_size + 15) / 16 * 16;

  auto& gen = RiscvGenerator::getInstance();
  gen.stackCore.SetStackMem(alloc_size);
  gen.funcCore.is_leaf_func = has_call_inst;
  return;
}

const Reg GetValueResult(const koopa_raw_value_t& value) {
  auto& gen = RiscvGenerator::getInstance();
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

void SolveArrAggInit(const koopa_raw_value_t& value,
                     ArrInfo& info,
                     const bool& should_push_dim) {
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    info.PushNum(value->kind.data.integer.value);
  } else if (value->kind.tag == KOOPA_RVT_AGGREGATE) {
    int len = value->kind.data.aggregate.elems.len;
    if (should_push_dim) {
      info.shape.insert(info.shape.begin(), len);
    }

    // 只有每个维度的第一个才改变维度
    bool spd = true;
    for (int i = 0; i < len; i++) {
      auto val = reinterpret_cast<koopa_raw_value_t>(
          value->kind.data.aggregate.elems.buffer[i]);
      SolveArrAggInit(val, info, spd && should_push_dim);
      spd = false;
    }
  }
}

#pragma endregion

}  // namespace riscv