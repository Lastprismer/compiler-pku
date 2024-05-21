#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "koopa.h"
#include "riscv_util.h"

using namespace std;

namespace riscv {

// 语法：ret
// 行为：将寄存器a0中的值作为返回值并返回
void ret(ostream& os);

// 语法：li {dest}, {imm}
// 行为：将imm加载到dest寄存器
void li(ostream& os, const Reg& dest, int imm);

// 语法：lw {rd}, {imm12}({rs})
// 行为：rd = M[imm12 + rs]
void lw(ostream& os, const Reg& rd, const Reg& rs, int addr);

// 语法：sw {rs}, {imm12}({rd})
// 行为：M[imm12 + rd] = rs
void sw(ostream& os, const Reg& rd, const Reg& rs, int addr);

// 语法：mv {rd}, {rs}
// 行为：rd = rs
void mv(ostream& os, const Reg& rd, const Reg& rs);

// 语法：add {rd}, {rs1}, {rs2}
// 行为：rd = rs1 + rs2
void add(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：addi {rd}, {rs1}, {imm}
// 行为：rd = rs1 + imm
void addi(ostream& os, const Reg& rd, const Reg& rs1, int imm);

// 语法：sub {rd}, {rs1}, {rs2}
// 行为：rd = rs1 - rs2
void sub(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：mul {rd}, {rs1}, {rs2}
// 行为：rd = rs1 * rs2
void mul(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：div {rd}, {rs1}, {rs2}
// 行为：rd = rs1 / rs2
void div(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：rem {rd}, {rs1}, {rs2}
// 行为：rd = rs1 % rs2
void rem(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：seqz {rd}, {rs}
// 行为：rs == 0 ? rd = 1 : rd = 0
void seqz(ostream& os, const Reg& rd, const Reg& rs);

// 语法：snez {rd}, {rs}
// 行为：rs != 0 ? rd = 1 : rd = 0
void snez(ostream& os, const Reg& rd, const Reg& rs);

// 语法：slt {rd}, {rs1}, {rs2}
// 行为：rd = rs1 < rs2
void slt(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：sgt {rd}, {rs1}, {rs2}
// 行为：rd = rs1 > rs2
void sgt(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：xor {rd}, {rs1}, {rs2}
// 行为：rd = rs1 ^ rs2
void xorr(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：xori {rd}, {rs1}, {imm}
// 行为：rd = rs1 ^ imm
void xori(ostream& os, const Reg& rd, const Reg& rs1, int imm);

// 语法：and {rd}, {rs1}, {rs2}
// 行为：rd = rs1 && rs2
void andr(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：or {rd}, {rs1}, {rs2}
// 行为：rd = rs1 || rs2
void orr(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 语法：j {label}
// 行为：无条件跳转到label
void j(ostream& os, const string& label);

// 语法：bnez {reg}, {label}
// 行为：判断reg的值，如果不为0则跳转到目标，否则继续执行下一条指令
void bnez(ostream& os, const Reg& reg, const string& label);

// 语法：beqz {reg}, {label}
// 行为：判断reg的值，如果为0则跳转到目标，否则继续执行下一条指令
void beqz(ostream& os, const Reg& reg, const string& label);

// 语法：call {name}
// 行为：调用函数，从一系列寄存器中取出变量，返回值存入ra
void call(ostream& os, const string& name);

/* --- 辅助函数 ---*/

const char* regstr(Reg reg);

// 最终行为：rd = rs1 <= rs2
void sle(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 最终行为：rd = rs1 >= rs2
void sge(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 最终行为：rd = rs1 == rs2
void eq(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 最终行为：rd = rs1 != rs2
void neq(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2);

// 打印label和前一个空行
void wlabel(ostream& os, const string& label);

}  // namespace riscv
