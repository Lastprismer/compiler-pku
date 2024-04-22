#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include "koopa.h"
#include "riscv_build.h"
#include "riscv_gen.h"
#include "riscv_read.h"

using namespace std;

namespace riscv {

/* core.cpp */
// 主功能
void ir2riscv(string ircode, const char* output);
// 生成raw program
koopa_raw_program_t get_raw_program(string ircode,
                                    koopa_raw_program_builder_t& builder);
// 释放raw program builder
void release_builder(koopa_raw_program_builder_t& builder);

}  // namespace riscv
