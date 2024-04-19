#include "util.h"

using namespace std;

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
      cout << (int)id;
      assert(false);
  }
  return string(s);
}

// ne, eq, gt, lt, ge, le, add, sub, mul, div, mod, and, or, xor, shl, shr, sar.