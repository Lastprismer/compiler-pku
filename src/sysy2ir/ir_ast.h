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
using ir::RetInfo;

/*
CompUnit        ::= FuncDef;

变量定义：
Decl            ::= ConstDecl | VarDecl;
ConstDecl       ::= "const" BType ConstDef ConstDeclList ";";
ConstDeclList   ::= "," ConstDef ConstDeclList | epsilon

BType           ::= "int";
ConstDef        ::= IDENT "=" ConstInitVal;
ConstInitVal    ::= ConstExp;
VarDecl         ::= BType VarDef VarDeclList ";";
VarDeclList     ::= "," VarDef VarDeclList | epsilon
VarDef          ::= IDENT | IDENT "=" InitVal;
InitVal         ::= Exp;


函数定义：
FuncDef         ::= FuncType IDENT "(" ")" Block;
FuncType        ::= "int";


语句：
Block           ::= "{" BlockItem BlockList "}";
BlockList       ::= BlockItem BlockList | epsilon
BlockItem       ::= Decl | Stmt;


逻辑：
Stmt            ::= OpenStmt | ClosedStmt
OpenStmt        ::= "if" "(" Exp ")" OpenStmt
                  | "if" "(" Exp ")" ClosedStmt
                  | "if" "(" Exp ")" ClosedStmt "else" OpenStmt
                  | "while" "(" Exp ")" OpenStmt

ClosedStmt      ::= SimpleStmt
                  | "if" "(" Exp ")" ClosedStmt "else" ClosedStmt
                  | "while" "(" Exp ")" ClosedStmt

SimpleStmt      ::= LVal "=" Exp ";"
                  | Exp ";"
                  | ";"
                  | Block
                  | "return" Exp ";"
                  | "return" ";"
                  | "continue" ";"
                  | "break" ";"



运算：
Exp             ::= LOrExp;
LVal            ::= IDENT;
PrimaryExp      ::= "(" Exp ")" | LVal | Number;
Number          ::= INT_CONST;
UnaryExp        ::= PrimaryExp
                  | "+" UnaryExp
                  | "-" UnaryExp
                  | "!" UnaryExp;
MulExp          ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp          ::= MulExp | AddExp ("+" | "-") MulExp;
RelExp          ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp           ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp         ::= EqExp | LAndExp "&&" EqExp;
LOrExp          ::= LAndExp | LOrExp "||" LAndExp;
ConstExp        ::= Exp;

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

// Decl          ::= ConstDecl | VarDecl;
class DeclAST : public BaseAST {
 public:
  enum de_t { CONST, VAR };
  de_t de;
  unique_ptr<BaseAST> decl;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

class ConstDefAST;
// ConstDecl     ::= "const" BType ConstDef ConstDeclList ";";
class ConstDeclAST : public BaseAST {
 public:
  unique_ptr<BaseAST> btype;
  vector<unique_ptr<ConstDefAST>> const_defs;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// ConstDeclList  ::= "," ConstDef ConstDeclList | epsilon
// 不进树
class ConstDeclListUnit : public BaseAST {
 public:
  // forgive me
  vector<ConstDefAST*> const_defs;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// BType         ::= "int";
class BTypeAST : public BaseAST {
 public:
  string btype;
  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// ConstDef      ::= IDENT "=" ConstInitVal;
class ConstDefAST : public BaseAST {
 public:
  string var_name;
  unique_ptr<BaseAST> const_init_val;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// ConstInitVal  ::= ConstExp;
class ConstInitValAST : public BaseAST {
 public:
  unique_ptr<BaseAST> const_exp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

class VarDefAST;
// VarDecl     ::= BType VarDef VarDeclList ";";
class VarDeclAST : public BaseAST {
 public:
  unique_ptr<BaseAST> btype;
  vector<unique_ptr<VarDefAST>> var_defs;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// VarDeclList  ::= "," VarDef VarDeclList | epsilon
// 不进树
class VarDeclListUnit : public BaseAST {
 public:
  // hello, world
  vector<VarDefAST*> var_defs;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// VarDef        ::= IDENT | IDENT "=" InitVal;
class VarDefAST : public BaseAST {
 public:
  bool init_with_val;
  string var_name;
  unique_ptr<BaseAST> init_val;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// InitVal       ::= Exp;
class InitValAST : public BaseAST {
 public:
  unique_ptr<BaseAST> exp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// FuncDef   ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST {
 public:
  unique_ptr<BaseAST> func_type;
  string func_name;
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

class BlockItemAST;
// Block           ::= "{" BlockItem BlockList "}";
class BlockAST : public BaseAST {
 public:
  vector<unique_ptr<BlockItemAST>> block_items;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// BlockList  ::= BlockItem BlockList | epsilon
// 不进树
class BlockListUnit : public BaseAST {
 public:
  // plz forgive me
  vector<BlockItemAST*> block_items;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// BlockItem     ::= Decl | Stmt;
class BlockItemAST : public BaseAST {
 public:
  enum blocktype_t { decl, stmt };
  blocktype_t bt;
  unique_ptr<BaseAST> content;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// Stmt            ::= OpenStmt | ClosedStmt
class StmtAST : public BaseAST {
 public:
  enum stmty_t { open, closed } type;
  unique_ptr<BaseAST> stmt;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

/*
OpenStmt        ::= "if" "(" Exp ")" OpenStmt
                  | "if" "(" Exp ")" ClosedStmt
                  | "if" "(" Exp ")" ClosedStmt "else" OpenStmt
                  | "while" "(" Exp ")" OpenStmt
*/
class OpenStmtAST : public BaseAST {
 public:
  enum opty_t { io, ic, iceo, loop } type;
  unique_ptr<BaseAST> open, closed, exp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

/*
ClosedStmt      ::= SimpleStmt
                  | "if" "(" Exp ")" ClosedStmt "else" ClosedStmt
                  | "while" "(" Exp ")" ClosedStmt
*/
class ClosedStmtAST : public BaseAST {
 public:
  enum csty_t { simp, icec, loop } type;
  unique_ptr<BaseAST> simple, tclosed, fclosed, exp;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

/*
SimpleStmt      ::= LVal "=" Exp ";"
                  | Exp
                  | ";"
                  | Block
                  | "return" Exp ";"
                  | "return" ";"
                  | "continue" ";"
                  | "break" ";"
*/
class SimpleStmtAST : public BaseAST {
 public:
  enum sstmt_t { storelval, ret, expr, block, nullexp, nullret, cont, brk };
  sstmt_t st;
  unique_ptr<BaseAST> lval;
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> blk;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// Exp         ::= LOrExp;
class ExpAST : public BaseAST {
 public:
  unique_ptr<BaseAST> loexp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// LVal          ::= IDENT;
class LValAST : public BaseAST {
 public:
  string var_name;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

// PrimaryExp    ::= "(" Exp ")" | LVal | Number;
class PrimaryExpAST : public BaseAST {
 public:
  enum primary_exp_type_t { Brackets, LVal, Number };
  primary_exp_type_t pt;
  unique_ptr<BaseAST> content;
  RetInfo thisRet;

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

/*
UnaryExp    ::= PrimaryExp
              | "+" UnaryExp
              | "-" UnaryExp
              | "!" UnaryExp;
*/
class UnaryExpAST : public BaseAST {
 public:
  enum uex_t { Primary, OPUnary };
  uex_t uex;
  enum uop_t { Pos, Neg, Not };
  uop_t uop;

  unique_ptr<BaseAST> exp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
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
  RetInfo thisRet;

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
  RetInfo thisRet;

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
  RetInfo thisRet;

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
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  const char* op_name() const;
  string type() const;
};

// LAndExp     ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST {
 public:
  enum laex_t { EqExp, LAOPEq };
  laex_t laex;
  unique_ptr<BaseAST> laexp;
  unique_ptr<BaseAST> eexp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  string type() const;
};

// LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST {
 public:
  enum loex_t { LAndExp, LOOPLA };
  loex_t loex;
  unique_ptr<BaseAST> laexp;
  unique_ptr<BaseAST> loexp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;

 private:
  string type() const;
};

// ConstExp      ::= Exp;
class ConstExpAST : public BaseAST {
 public:
  unique_ptr<BaseAST> exp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};

void make_indent(ostream& os, int indent);
// ...