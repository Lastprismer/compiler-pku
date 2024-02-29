#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "koopa.h"

using namespace std;

/* core.cpp */
// 主功能
void ir2riscv(string ircode, const char* output);
// 生成raw program
koopa_raw_program_t get_raw_program(string ircode,
                                    koopa_raw_program_builder_t& builder);
// 释放raw program builder
void release_builder(koopa_raw_program_builder_t& builder);

/* funcdata.cpp */
typedef struct {
  koopa_raw_function_t& raw_function;
  koopa_raw_value_t& return_value;
} function_data_t;

/* build.cpp */
// 生成li指令
// 语法：li {dest} {imm}
// 行为：将imm加载到dest寄存器
void li(ostream& os, const string& dest, int imm);

/* read.cpp */
// 访问 raw program
void visit_program(const koopa_raw_program_t& program, ostream& os);
// 访问 raw slice
void visit_slice(const koopa_raw_slice_t& slice, ostream& os);
// 访问函数
void visit_func(const koopa_raw_function_t& func, ostream& os);
// 访问基本块
void visit_basic_block(const koopa_raw_basic_block_t& bb, ostream& os);
// 访问指令
void visit_value(const koopa_raw_value_t& value, ostream& os);
// 访问ret
void visit_inst_ret(const koopa_raw_return_t& inst_ret, ostream& os);
// 访问int
void visit_inst_int(const koopa_raw_integer_t& inst_int, ostream& os);

/* util.cpp */
// 将具名符号或临时符号名称删去头部字符
const char* parse_symbol(const char* symbol_name);
