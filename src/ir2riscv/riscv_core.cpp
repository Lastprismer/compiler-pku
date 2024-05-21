#include "riscv_ir2riscv.h"

namespace riscv {

void ir2riscv(string ircode, const char* output) {
  koopa_raw_program_builder_t builder;
  koopa_raw_program_t program = get_raw_program(ircode, builder);
  stringstream ss;
  RiscvGenerator::getInstance().setting.setOs(ss);
  visit_program(program);

  ofstream outfile(output);
  if (outfile.is_open()) {
    outfile << ss.str();
    outfile.close();
  } else {
    cerr << "无法打开文件：" << output << endl;
  }
  release_builder(builder);
}

koopa_raw_program_t get_raw_program(string ircode,
                                    koopa_raw_program_builder_t& builder) {
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(ircode.c_str(), &program);
  // 确保解析时没有出错
  assert(ret == KOOPA_EC_SUCCESS);
  // 创建一个 raw program builder, 用来构建 raw program
  builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);
  return raw;
}

void release_builder(koopa_raw_program_builder_t& builder) {
  koopa_delete_raw_program_builder(builder);
}

}  // namespace riscv