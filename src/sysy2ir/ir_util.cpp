#include "ir_util.h"

namespace ir {

ArrInfo::ArrInfo() : dimension(0), shape() {}

ArrInfo::ArrInfo(const int& dim, const vector<int>& _shape)
    : dimension(dim), shape(_shape) {}

const string ArrInfo::GetType() const {
  // TODO：多维扩展
  stringstream ss;
  ss << "[i32, " << shape[0] << "]";
  return ss.str();
}

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
    default:
      return string();
  }
}

}  // namespace ir