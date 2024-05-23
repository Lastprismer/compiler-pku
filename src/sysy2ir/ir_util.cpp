#include "ir_util.h"

namespace ir {

string BiOp2koopa(OpID id) {
  const char* s;
  switch (id) {
    case BI_ADD:
      s = "add";
      break;
    case BI_SUB:
      s = "sub";
      break;
    case BI_MUL:
      s = "mul";
      break;
    case BI_DIV:
      s = "div";
      break;
    case BI_MOD:
      s = "mod";
      break;
    case LG_AND:
      s = "and";
      break;
    case LG_OR:
      s = "or";
      break;
    case LG_EQ:
      s = "eq";
      break;
    case LG_NEQ:
      s = "ne";
      break;
    case LG_LT:
      s = "lt";
      break;
    case LG_LE:
      s = "le";
      break;
    case LG_GT:
      s = "gt";
      break;
    case LG_GE:
      s = "ge";
      break;
    default:
      s = "";
      break;
  }
  return string(s);
}

const string GetVarType(const VarType& ty) {
  switch (ty) {
    case VarType::e_int:
      return string("i32");
    case VarType::e_void:
      return string("");
    default:
      return string("");
  }
}

}  // namespace ir