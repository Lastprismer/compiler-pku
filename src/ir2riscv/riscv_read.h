#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include "koopa.h"
#include "riscv_gen.h"
#include "riscv_util.h"

using namespace std;

namespace riscv {

/* read.cpp */
// 访问 raw program
void visit_program(const koopa_raw_program_t& program);
// 访问 raw slice
void visit_slice(const koopa_raw_slice_t& slice);
// 访问函数
void visit_func(const koopa_raw_function_t& func);
// 访问基本块
void visit_basic_block(const koopa_raw_basic_block_t& bb);
// 访问指令
void visit_value(const koopa_raw_value_t& value);
// 访问int
void visit_inst_int(const koopa_raw_integer_t& inst_int);
// 访问二元运算
void visit_inst_binary(const koopa_raw_binary_t& inst_bina);
// 访问ret
void visit_inst_ret(const koopa_raw_return_t& inst_ret);

}