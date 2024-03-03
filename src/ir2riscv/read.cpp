#include "read.h"

void visit_program(const koopa_raw_program_t& program,
                   ostream& os,
                   IRFuncInfo& info) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  visit_slice(program.values, os, info);
  // 访问所有函数=
  visit_slice(program.funcs, os, info);
}

void visit_slice(const koopa_raw_slice_t& slice,
                 ostream& os,
                 IRFuncInfo& info) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        visit_func(reinterpret_cast<koopa_raw_function_t>(ptr), os, info);
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        visit_basic_block(reinterpret_cast<koopa_raw_basic_block_t>(ptr), os,
                          info);
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        visit_value(reinterpret_cast<koopa_raw_value_t>(ptr), os, info);
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

void visit_func(const koopa_raw_function_t& func,
                ostream& os,
                IRFuncInfo& info) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有基本块
  info.func_name = parse_symbol(func->name);
  info.write_prologue(os);
  visit_slice(func->bbs, os, info);
}

void visit_basic_block(const koopa_raw_basic_block_t& bb,
                       ostream& os,
                       IRFuncInfo& info) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  // os << "basic block: name: " << bb->name << endl;
  visit_slice(bb->insts, os, info);
}

void visit_value(const koopa_raw_value_t& value,
                 ostream& os,
                 IRFuncInfo& info) {
  // os << "value: " << value->name << endl;
  //  根据指令类型判断后续需要如何访问
  const auto& kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_INTEGER:
      visit_inst_int(kind.data.integer, os, info);
      break;
    case KOOPA_RVT_BINARY:
      visit_inst_binary(kind.data.binary, os, info);
      break;
    case KOOPA_RVT_RETURN:
      visit_inst_ret(kind.data.ret, os, info);
      break;
    default:
      assert(false);
      break;
      // 其他类型暂时遇不到
  }
}

void visit_inst_int(const koopa_raw_integer_t& inst_int,
                    ostream& os,
                    IRFuncInfo& info) {
  info.push_imm(inst_int.value);
}

void visit_inst_binary(const koopa_raw_binary_t& inst_bina,
                       ostream& os,
                       IRFuncInfo& info) {
  visit_value(inst_bina.lhs, os, info);
  visit_value(inst_bina.rhs, os, info);
  info.write_inst(os, inst_bina.op);
}

void visit_inst_ret(const koopa_raw_return_t& inst_ret,
                    ostream& os,
                    IRFuncInfo& info) {
  info.start_writing = true;
  visit_value(inst_ret.value, os, info);
  info.write_epilogue(os);
}