#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace ir {

using std::string, std::vector, std::stringstream;

enum class SymbolType { e_unused, e_const, e_var };
enum class VarType { e_unused, e_int, e_void, e_arr };

enum OpID {
  UNARY_POS,
  UNARY_NEG,
  UNARY_NOT,

  BI_ADD,
  BI_SUB,
  BI_MUL,
  BI_DIV,
  BI_MOD,

  LG_GT,
  LG_GE,
  LG_LT,
  LG_LE,
  LG_EQ,
  LG_NEQ,
  LG_AND,
  LG_OR,
};

string BiOp2koopa(OpID id);

const string GetVarType(const VarType& ty);
}  // namespace ir