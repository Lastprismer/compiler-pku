#pragma once
#include <assert.h>
#include <iostream>
#include <string>

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

std::string BiOp2koopa(OpID id);