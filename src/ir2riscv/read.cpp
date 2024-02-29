#include "ir2riscv.h"

void visit_program(const koopa_raw_program_t& program, ostream& os) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  visit_slice(program.values, os);
  // 访问所有函数
  visit_slice(program.funcs, os);
}

void visit_slice(const koopa_raw_slice_t& slice, ostream& os) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        visit_func(reinterpret_cast<koopa_raw_function_t>(ptr), os);
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        visit_basic_block(reinterpret_cast<koopa_raw_basic_block_t>(ptr), os);
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        visit_value(reinterpret_cast<koopa_raw_value_t>(ptr), os);
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

void visit_func(const koopa_raw_function_t& func, ostream& os) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有基本块
  os << "  .text" << endl;
  os << "  .globl " << parse_symbol(func->name) << endl;
  os << parse_symbol(func->name) << ":" << endl;

  visit_slice(func->bbs, os);
}

void visit_basic_block(const koopa_raw_basic_block_t& bb, ostream& os) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  // os << "basic block: name: " << bb->name << endl;
  visit_slice(bb->insts, os);
}

void visit_value(const koopa_raw_value_t& value, ostream& os) {
  // os << "value: " << value->name << endl;
  //  根据指令类型判断后续需要如何访问
  const auto& kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      visit_inst_ret(kind.data.ret, os);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      visit_inst_int(kind.data.integer, os);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

void visit_inst_ret(const koopa_raw_return_t& inst_ret, ostream& os) {
  visit_value(inst_ret.value, os);
  os << "  ret" << endl;
}

void visit_inst_int(const koopa_raw_integer_t& inst_int, ostream& os) {
  li(os, "a0", inst_int.value);
}
