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

// 访问alloc
void visit_inst_alloc(const koopa_raw_value_t& value);

// 访问load
void visit_inst_load(const koopa_raw_value_t& inst_load);

// 访问store
void visit_inst_store(const koopa_raw_value_t& inst);

// 访问二元运算
void visit_inst_binary(const koopa_raw_value_t& inst);

// 访问分支
void visit_inst_branch(const koopa_raw_value_t& inst);

// 访问跳转
void visit_inst_jump(const koopa_raw_value_t& inst);

// 访问函数调用
void visit_inst_call(const koopa_raw_value_t& inst);

// 访问ret
void visit_inst_ret(const koopa_raw_return_t& inst_ret);

// 其他函数

// 顺序遍历，计算函数分配所需的内存
void CalcMemoryNeeded(const koopa_raw_function_t& func);

// 获取某条指令返回值的放置位置，如果在栈上，则将其拉回寄存器内
const Reg GetValueResult(const koopa_raw_value_t& value);

// 给定参数号，输出应该存储这个参数的位置
const InstResultInfo GetParamPosition(const int& param_cnt);

}  // namespace riscv