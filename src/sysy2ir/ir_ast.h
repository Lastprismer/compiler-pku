// 所有 AST 的基类
#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include "ir_gen.h"
#include "ir_sysy2ir.h"

#define INDENT_LEN 4

using namespace std;

/*

CompUnit  ::= FuncDef;

FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncType  ::= "int";

Block     ::= "{" Stmt "}";
Stmt        ::= "return" Exp ";";

Exp         ::= LOrExp;
PrimaryExp  ::= "(" Exp ")" | Number;
Number      ::= INT_CONST;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp     ::= "+" | "-" | "!";
MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp     ::= EqExp | LAndExp "&&" EqExp;
LOrExp      ::= LAndExp | LOrExp "||" LAndExp;

*/

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Print(ostream& os, int indent) const = 0;
  virtual void Dump() = 0;
};

// CompUnit  ::= FuncDef;
class CompUnitAST : public BaseAST {
 public:
  unique_ptr<BaseAST> func_def;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// FuncDef   ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST {
 public:
  unique_ptr<BaseAST> funcType;
  string funcName;
  unique_ptr<BaseAST> block;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// FuncType  ::= "int";
class FuncTypeAST : public BaseAST {
 public:
  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// Block     ::= "{" Stmt "}";
class BlockAST : public BaseAST {
 public:
  unique_ptr<BaseAST> stmt;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// Stmt        ::= "return" Exp ";";
class StmtAST : public BaseAST {
 public:
  unique_ptr<BaseAST> exp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// Exp         ::= LOrExp;
class ExpAST : public BaseAST {
 public:
  unique_ptr<BaseAST> loexp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// PrimaryExp  ::= "(" Exp ")" | Number;
class PrimaryExpAST : public BaseAST {
 public:
  enum pex_t { Brackets, Number };
  pex_t pex;
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> number;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  const char* type() const;
};

// Number      ::= INT_CONST;
class NumberAST : public BaseAST {
 public:
  int int_const;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
class UnaryExpAST : public BaseAST {
 public:
  enum uex_t { Primary, Unary };
  uex_t uex;
  unique_ptr<BaseAST> prim;
  unique_ptr<BaseAST> uop;
  unique_ptr<BaseAST> uexp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  const char* type() const;
};

// UnaryOp     ::= "+" | "-" | "!";
class UnaryOPAST : public BaseAST {
 public:
  enum uop_t { Pos, Neg, Not };
  uop_t uop;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  const char* op_name() const;
};

// MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
class MulExpAST : public BaseAST {
 public:
  enum mex_t { Unary, MulOPUnary };
  mex_t mex;
  enum mop_t { Mul, Div, Mod };
  mop_t mop;
  unique_ptr<BaseAST> mexp;
  unique_ptr<BaseAST> uexp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  const char* op_name() const;
  string type() const;
};

// AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
class AddExpAST : public BaseAST {
 public:
  enum aex_t { MulExp, AddOPMul };
  aex_t aex;
  enum aop_t { Add, Sub };
  aop_t aop;
  unique_ptr<BaseAST> mexp;
  unique_ptr<BaseAST> aexp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  const char* op_name() const;
  string type() const;
};

// RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpAST : public BaseAST {
 public:
  enum rex_t { AddExp, RelOPAdd };
  rex_t rex;
  enum rop_t { LessThan, LessEqual, GreaterThan, GreaterEqual };
  rop_t rop;
  unique_ptr<BaseAST> rexp;
  unique_ptr<BaseAST> aexp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  const char* op_name() const;
  string type() const;
};

// EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpAST : public BaseAST {
 public:
  enum eex_t { RelExp, EqOPRel };
  eex_t eex;
  enum eop_t { Equal, NotEqual };
  eop_t eop;
  unique_ptr<BaseAST> eexp;
  unique_ptr<BaseAST> rexp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  const char* op_name() const;
  string type() const;
};

// LAndExp     ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAst : public BaseAST {
 public:
  enum laex_t { EqExp, LAOPEq };
  laex_t laex;
  unique_ptr<BaseAST> laexp;
  unique_ptr<BaseAST> eexp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  string type() const;
};

// LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAst : public BaseAST {
 public:
  enum loex_t { LAndExp, LOOPLA };
  loex_t loex;
  unique_ptr<BaseAST> laexp;
  unique_ptr<BaseAST> loexp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  string type() const;
};

void make_indent(ostream& os, int indent);
// ...