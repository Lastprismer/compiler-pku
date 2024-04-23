#include "riscv_util.h"
#include "riscv_ir2riscv.h"

namespace riscv {

const char* parseSymbol(const char* symbol_name) {
  return symbol_name + 1;
}

const char* get_binary_op_string(koopa_raw_binary_op_t opt) {
  koopa_raw_binary_op op = (koopa_raw_binary_op)opt;
  switch (op) {
    case KOOPA_RBO_NOT_EQ:
      return "!=";
    case KOOPA_RBO_EQ:
      return "==";
    case KOOPA_RBO_GT:
      return ">";
    case KOOPA_RBO_LT:
      return "<";
    case KOOPA_RBO_GE:
      return ">=";
    case KOOPA_RBO_LE:
      return "<=";
    case KOOPA_RBO_ADD:
      return "+";
    case KOOPA_RBO_SUB:
      return "-";
    case KOOPA_RBO_MUL:
      return "*";
    case KOOPA_RBO_DIV:
      return "/";
    case KOOPA_RBO_MOD:
      return "%";
    case KOOPA_RBO_AND:
      return "&";
    case KOOPA_RBO_OR:
      return "|";
    case KOOPA_RBO_XOR:
      return "^";
    case KOOPA_RBO_SHL:
      return "<<";
    case KOOPA_RBO_SHR:
      return "<<(shr)";
    case KOOPA_RBO_SAR:
      return "<<(sar)";
  }
}

bool immInBound(int imm) {
  return imm >= -2048 && imm <= 2047;
}
Reg zeroReg() {
  return Reg::x0;
}
}  // namespace riscv