#include "util.h"
#include "ir2riscv.h"

const char* parse_symbol(const char* symbol_name) {
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