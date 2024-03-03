#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include "build.h"
#include "info.h"
#include "koopa.h"
#include "read.h"

using namespace std;

/* core.cpp */
// 主功能
void ir2riscv(string ircode, const char* output);
// 生成raw program
koopa_raw_program_t get_raw_program(string ircode,
                                    koopa_raw_program_builder_t& builder);
// 释放raw program builder
void release_builder(koopa_raw_program_builder_t& builder);
