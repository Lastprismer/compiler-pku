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
    }
  }
}

void visit_func(const koopa_raw_function_t& func) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有基本块
  RiscvGenerator& gen = RiscvGenerator::getInstance();
  gen.function_name = parseSymbol(func->name);
  gen.writePrologue();
  visit_slice(func->bbs);
}

void visit_basic_block(const koopa_raw_basic_block_t& bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  // os << "basic block: name: " << bb->name << endl;
  visit_slice(bb->insts);
}

void visit_value(const koopa_raw_value_t& value) {
  // os << "value: " << value->name << endl;
  //  根据指令类型判断后续需要如何访问
  const auto& kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_INTEGER:
      visit_inst_int(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      visit_inst_binary(kind.data.binary);
      break;
    case KOOPA_RVT_RETURN:
      visit_inst_ret(kind.data.ret);
      break;
    default:
      assert(false);
      break;
      // 其他类型暂时遇不到
  }
}

void visit_inst_int(const koopa_raw_integer_t& inst_int) {
  RiscvGenerator::getInstance().pushImm(inst_int.value);
}

void visit_inst_binary(const koopa_raw_binary_t& inst_bina) {
  visit_value(inst_bina.lhs);
  visit_value(inst_bina.rhs);
  RiscvGenerator::getInstance().writeInst(inst_bina.op);
}

void visit_inst_ret(const koopa_raw_return_t& inst_ret) {
  RiscvGenerator& gen = RiscvGenerator::getInstance();
  gen.setting.start_writing = true;
  visit_value(inst_ret.value);
  gen.writeEpilogue();
}

}  // namespace riscv