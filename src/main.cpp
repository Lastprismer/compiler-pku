#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include "ast.h"
#include "ir2riscv/ir2riscv.h"
#include "sysy2ir/sysy2ir.h"

void supreme_compile(int argc, const char* argv[]);

enum CompilerMode { koopa, riscv, perf };

int main(int argc, const char* argv[]) {
  supreme_compile(argc, argv);
  return 0;
}

void supreme_compile(int argc, const char* argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto input = argv[2];
  auto output = argv[4];
  CompilerMode mode;
  if (strcmp(argv[1], "-koopa") == 0) {
    mode = CompilerMode::koopa;
  } else if (strcmp(argv[1], "-riscv") == 0) {
    mode = CompilerMode::riscv;
  } else if (strcmp(argv[1], "-perf") == 0) {
    mode = CompilerMode::perf;
  } else {
    assert(false);
  }

  string ir = sysy2ir(input, output, mode == CompilerMode::koopa, true);
  if (mode == CompilerMode::koopa) {
    return;
  }
  ir2riscv(ir, output);
}
