// 所有 AST 的基类
#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include "info.h"
#include "sysy2ir.h"

#define INDENT_LEN 4

using namespace std;

/*

CompUnit  ::= FuncDef;

FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncType  ::= "int";

Block     ::= "{" Stmt "}";
Stmt        ::= "return" Exp ";";

Exp         ::= UnaryExp;
PrimaryExp  ::= "(" Exp ")" | Number;
Number      ::= INT_CONST;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp     ::= "+" | "-" | "!";


*/
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Print(ostream& os, int indent) const = 0;
  virtual void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) = 0;
};

// CompUnit  ::= FuncDef;
class CompUnitAST : public BaseAST {
 public:
  unique_ptr<BaseAST> func_def;

  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;
};

// FuncDef   ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST {
 public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;
  shared_ptr<FuncInfo> func_info;

  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;
};

// FuncType  ::= "int";
class FuncTypeAST : public BaseAST {
 public:
  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;
};

// Block     ::= "{" Stmt "}";
class BlockAST : public BaseAST {
 public:
  unique_ptr<BaseAST> stmt;

  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;
};

// Stmt        ::= "return" Exp ";";
class StmtAST : public BaseAST {
 public:
  unique_ptr<BaseAST> exp;

  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;
};

// Exp         ::= UnaryExp;
class ExpAST : public BaseAST {
 public:
  unique_ptr<BaseAST> uexp;

  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;
};

// PrimaryExp  ::= "(" Exp ")" | Number;
class PrimaryExpAST : public BaseAST {
 public:
  const char* BRACKETSEXP_NAME = "(Exp)";
  const char* NUMBER_NAME = "Number";

  enum pex_t { Brackets, Number };
  pex_t pex;
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> number;

  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;

 private:
  const char* enum_name() const;
};

// Number      ::= INT_CONST;
class NumberAST : public BaseAST {
 public:
  int int_const;

  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;
};

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
class UnaryExpAST : public BaseAST {
 public:
  const char* PRIMARY_NAME = "PrimaryExp";
  const char* UNARY_NAME = "UnaryOp UnaryExp";

  enum uex_t { Primary, Unary };
  uex_t uex;
  unique_ptr<BaseAST> prim;
  unique_ptr<BaseAST> uop;
  unique_ptr<BaseAST> uexp;

  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;

 private:
  const char* enum_name() const;
};

// UnaryOp     ::= "+" | "-" | "!";
class UnaryOPAST : public BaseAST {
 public:
  const char* POS_NAME = "+";
  const char* NEG_NAME = "-";
  const char* NOT_NAME = "!";

  enum uop_t { Pos, Neg, Not };
  uop_t uop;

  void Print(ostream& os, int indent) const override;
  void Dump(ostream& os, shared_ptr<FuncInfo> info, int indent) override;

 private:
  const char* enum_name() const;
};

void make_indent(ostream& os, int indent);
// ...
