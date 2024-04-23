#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include "koopa.h"

namespace riscv {

typedef enum riscv_registers_enum_t {
  NONE,
  t0,
  t1,
  t2,
  t3,
  t4,
  t5,
  t6,
  a0,
  a1,
  a2,
  a3,
  a4,
  a5,
  a6,
  a7,
  x0  // 0
} Reg;

// 将具名符号或临时符号名称删去头部字符
const char* parseSymbol(const char* symbol_name);
// 获取koopa_raw_binary_op对应的符号的字符串
const char* get_binary_op_string(koopa_raw_binary_op_t opt);
// [-2048, 2047]
bool immInBound(int imm);

Reg zeroReg();
}