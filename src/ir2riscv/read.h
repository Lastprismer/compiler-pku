#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include "info.h"
#include "koopa.h"
#include "util.h"

using namespace std;

/* read.cpp */
// 访问 raw program
void visit_program(const koopa_raw_program_t& program,
                   ostream& os,
                   IRFuncInfo& info);
// 访问 raw slice
void visit_slice(const koopa_raw_slice_t& slice, ostream& os, IRFuncInfo& info);
// 访问函数
void visit_func(const koopa_raw_function_t& func,
                ostream& os,
                IRFuncInfo& info);
// 访问基本块
void visit_basic_block(const koopa_raw_basic_block_t& bb,
                       ostream& os,
                       IRFuncInfo& info);
// 访问指令
void visit_value(const koopa_raw_value_t& value, ostream& os, IRFuncInfo& info);
// 访问int
void visit_inst_int(const koopa_raw_integer_t& inst_int,
                    ostream& os,
                    IRFuncInfo& info);
// 访问二元运算
void visit_inst_binary(const koopa_raw_binary_t& inst_bina,
                       ostream& os,
                       IRFuncInfo& info);
// 访问ret
void visit_inst_ret(const koopa_raw_return_t& inst_ret,
                    ostream& os,
                    IRFuncInfo& info);