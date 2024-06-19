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
CompRoot        ::= CompUnitList
CompUnitList    ::= CompUnit CompUnitList | epsilon
CompUnit        ::= FuncDef | Decl

变量定义：
Decl            ::= ConstDecl | VarDecl
ConstDecl       ::= "const" BType ConstDef ConstDeclList ";"
ConstDeclList   ::= "," ConstDef ConstDeclList | epsilon

BType           ::= "int" | "void"
ConstDef        ::= IDENT "=" ConstInitVal
                  | IDENT ArrSize "=" ConstArrVal
ConstInitVal    ::= ConstExp
VarDecl         ::= BType VarDef VarDeclList ";"
VarDeclList     ::= "," VarDef VarDeclList | epsilon
VarDef          ::= IDENT
                  | IDENT "=" InitVal
                  | IDENT ArrSize
                  | IDENT ArrSize "=" ArrInitVal
InitVal         ::= Exp


数组定义：
ArrSize         ::= "[" ConstExp "]" ArrSizeList
ArrSizeList     ::= "[" ConstExp "]" ArrSizeList | epsilon

CAElement       ::= ConstExp | ConstArrVal
ConstArrVal     ::= "{" "}" | "{" CAElement CAElementList "}"
CAElementList   ::= "," CAElement CAElementList | epsilon

AIElement       ::= Exp | ArrInitVal
ArrInitVal      ::= "{" "}" | "{" AIElement AIElementList "}"
AIElementList   ::= "," AIElement AIElementList | epsilon


函数定义：
FuncDef         ::= BType IDENT "(" FuncFParams ")" Block
FuncFParams     ::= FuncFParam FuncFParamsList | epsilon
FuncFParamsList ::= "," FuncFParam FuncFParamsList | epsilon
FuncFParam      ::= INT IDENT
                  | INT IDENT "[" "]"
                  | INT IDENT "[" "]" ArrSize

语句：
Block           ::= "{" BlockItem BlockList "}"
BlockList       ::= BlockItem BlockList | epsilon
BlockItem       ::= Decl | Stmt


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
Exp             ::= LOrExp

ArrAddr         ::= "[" Exp "]" ArrAddrList
ArrAddrList     ::= "[" Exp "]" ArrAddrList | epsilon
LVal            ::= IDENT | IDENT ArrAddr

PrimaryExp      ::= "(" Exp ")" | LVal | Number
Number          ::= INT_CONST
UnaryExp        ::= PrimaryExp
                  | "+" UnaryExp
                  | "-" UnaryExp
                  | "!" UnaryExp
                  | IDENT "(" FuncRParams ")"
                  | IDENT "(" ")"


函数调用：
FuncRParams     ::= Exp FuncRParamsList
FuncRParamsList ::= "," Exp FuncRParamsList | epsilon

MulExp          ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp
AddExp          ::= MulExp | AddExp ("+" | "-") MulExp
RelExp          ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp
EqExp           ::= RelExp | EqExp ("==" | "!=") RelExp
LAndExp         ::= EqExp | LAndExp "&&" EqExp
LOrExp          ::= LAndExp | LOrExp "||" LAndExp
ConstExp        ::= Exp

*/

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Print(ostream& os, int indent) const = 0;
  virtual void Dump() = 0;
};

#pragma region CompRoot
class CompUnitAST;
// CompRoot        ::= CompUnitList
class CompRootAST : public BaseAST {
 public:
  vector<unique_ptr<CompUnitAST>> comp_units;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region CompUnitList
// CompUnitList    ::= CompUnit CompUnitList | epsilon
// 不进树
class CompUnitListUnit : public BaseAST {
 public:
  vector<CompUnitAST*> comp_units;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region CompUnit

// CompUnit        ::= FuncDef | Decl;
class CompUnitAST : public BaseAST {
 public:
  enum comp_unit_ty { e_func_def, e_decl } ty;
  unique_ptr<BaseAST> content;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region Decl
// Decl          ::= ConstDecl | VarDecl;
class DeclAST : public BaseAST {
 public:
  enum de_t { e_const, e_var };
  de_t de;
  unique_ptr<BaseAST> decl;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ConstDecl
class ConstDefAST;
// ConstDecl     ::= "const" BType ConstDef ConstDeclList ";";
class ConstDeclAST : public BaseAST {
 public:
  unique_ptr<BaseAST> btype;
  vector<unique_ptr<ConstDefAST>> const_defs;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ConstDeclList
// ConstDeclList  ::= "," ConstDef ConstDeclList | epsilon
// 不进树
class ConstDeclListUnit : public BaseAST {
 public:
  // forgive me
  vector<ConstDefAST*> const_defs;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region BType
// BType         ::= "int";
class BTypeAST : public BaseAST {
 public:
  enum btype_t { e_int, e_void } ty;
  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ConstDef
/*
ConstDef        ::= IDENT "=" ConstInitVal
                  | IDENT ArrSize "=" ConstArrVal
*/
class ConstDefAST : public BaseAST {
 public:
  enum def_t { e_int, e_arr } ty;
  string var_name;
  unique_ptr<BaseAST> arr_size;
  unique_ptr<BaseAST> const_init_val;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ConstInitVal
// ConstInitVal  ::= ConstExp;
class ConstInitValAST : public BaseAST {
 public:
  unique_ptr<BaseAST> const_exp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region VarDecl
class VarDefAST;
// VarDecl     ::= BType VarDef VarDeclList ";";
class VarDeclAST : public BaseAST {
 public:
  unique_ptr<BaseAST> btype;
  vector<unique_ptr<VarDefAST>> var_defs;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region VarDeclList
// VarDeclList  ::= "," VarDef VarDeclList | epsilon
// 不进树
class VarDeclListUnit : public BaseAST {
 public:
  // hello, world
  vector<VarDefAST*> var_defs;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region VarDef
/*
VarDef          ::= IDENT
                  | IDENT "=" InitVal
                  | IDENT ArrSize
                  | IDENT ArrSize "=" ArrInitVal
*/
class VarDefAST : public BaseAST {
 public:
  bool init_with_val;
  enum def_t { e_int, e_arr } ty;
  string var_name;
  unique_ptr<BaseAST> arr_size;
  unique_ptr<BaseAST> init_val;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region InitVal
// InitVal       ::= Exp;
class InitValAST : public BaseAST {
 public:
  unique_ptr<BaseAST> exp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ArrSize
class ConstExpAST;
// ArrSize         ::= "[" ConstExp "]" ArrSizeList
class ArrSizeAST : public BaseAST {
 public:
  vector<unique_ptr<ConstExpAST>> arr_size;
  vector<int> size_value;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ArrSizeList
// ArrSizeList     ::= "[" ConstExp "]" ArrSizeList | epsilon
class ArrSizeListUnit : public BaseAST {
 public:
  vector<ConstExpAST*> values;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region CAElement
// CAElement       ::= ConstExp | ConstArrVal
class CAElementAST : public BaseAST {
 public:
  enum caty_t { e_cexp, e_carr } ty;
  unique_ptr<BaseAST> content;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ConstArrVal
// ConstArrVal     ::= "{" "}" | "{" CAElement CAElementList "}"
class ConstArrValAST : public BaseAST {
 public:
  vector<unique_ptr<CAElementAST>> values;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region CAElementList
// CAElementList ::= "," CAElement CAElementList | epsilon
// 不进树
class CAElementListUnit : public BaseAST {
 public:
  vector<CAElementAST*> values;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region AIElement
class ExpAST;
// AIElement       ::= Exp | ArrInitVal
class AIElementAST : public BaseAST {
 public:
  enum aity_t { e_exp, e_arr } ty;
  unique_ptr<BaseAST> content;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ArrInitVal
// ArrInitVal      ::= "{" "}" | "{" AIElement AIElementList "}"
class ArrInitValAST : public BaseAST {
 public:
  vector<unique_ptr<AIElementAST>> values;
  vector<RetInfo> init_values;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region AIElementList
// AIElementList   ::= "," AIElement AIElementList | epsilon
// 不进树
class AIElementListUnit : public BaseAST {
 public:
  vector<AIElementAST*> values;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region FuncDef
// FuncDef         ::= BType IDENT "(" FuncFParams ")" Block
class FuncDefAST : public BaseAST {
 public:
  unique_ptr<BaseAST> func_type;
  unique_ptr<BaseAST> params;
  unique_ptr<BaseAST> block;
  string func_name;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region FuncFParams
// FuncFParams     ::= FuncFParam FuncFParamsList | epsilon
class FuncFParamAST;
class FuncFParamsAST : public BaseAST {
 public:
  vector<unique_ptr<FuncFParamAST>> params;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region FuncFParamsList
// FuncFParamsList       ::= "," FuncFParam FuncFParamsList | epsilon
// 不进树
class FuncFParamsListUnit : public BaseAST {
 public:
  vector<FuncFParamAST*> params;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region FuncFParam
/*
FuncFParam      ::= INT IDENT
                  | INT IDENT "[" "]"
                  | INT IDENT "[" "]" ArrSize
*/
class FuncFParamAST : public BaseAST {
 public:
  unique_ptr<BaseAST> ptr_size;
  bool is_ptr;
  string param_name;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region Block
class BlockItemAST;
// Block           ::= "{" BlockItem BlockList "}"
class BlockAST : public BaseAST {
 public:
  vector<unique_ptr<BlockItemAST>> block_items;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region BlockList
// BlockList  ::= BlockItem BlockList | epsilon
// 不进树
class BlockListUnit : public BaseAST {
 public:
  // plz forgive me
  vector<BlockItemAST*> block_items;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region BlockItem
// BlockItem     ::= Decl | Stmt
class BlockItemAST : public BaseAST {
 public:
  enum blocktype_t { decl, stmt };
  blocktype_t bt;
  unique_ptr<BaseAST> content;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region Stmt
// Stmt            ::= OpenStmt | ClosedStmt
class StmtAST : public BaseAST {
 public:
  enum stmty_t { open, closed } type;
  unique_ptr<BaseAST> stmt;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region OpenStmt
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
#pragma endregion

#pragma region ClosedStmt
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
#pragma endregion

#pragma region SimpleStmt
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
#pragma endregion

#pragma region Exp
// Exp         ::= LOrExp
class ExpAST : public BaseAST {
 public:
  unique_ptr<BaseAST> loexp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ArrAddr
class ConstExpAST;
// ArrAddr         ::= "[" Exp "]" ArrAddrList
class ArrAddrAST : public BaseAST {
 public:
  vector<unique_ptr<ExpAST>> arr_addr;
  vector<RetInfo> addr_value;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region ArrAddrList
// ArrAddrList     ::= "[" Exp "]" ArrAddrList | epsilon
class ArrAddrListUnit : public BaseAST {
 public:
  vector<ExpAST*> addrs;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region Lval
// LVal            ::= IDENT | IDENT ArrAddr
class LValAST : public BaseAST {
 public:
  enum lval_t { e_noaddr, e_withaddr } ty;
  unique_ptr<BaseAST> arr_param;
  string var_name;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region PrimaryExp
// PrimaryExp    ::= "(" Exp ")" | LVal | Number
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
#pragma endregion

#pragma region Number
// Number      ::= INT_CONST
class NumberAST : public BaseAST {
 public:
  int int_const;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region UnaryExp
/*
UnaryExp        ::= PrimaryExp
                  | "+" UnaryExp
                  | "-" UnaryExp
                  | "!" UnaryExp
                  | IDENT "(" FuncRParams ")"
                  | IDENT "(" ")"
*/
class UnaryExpAST : public BaseAST {
 public:
  enum uex_t { Primary, OPUnary, FuncWithParam, FuncNoParam } uex;
  enum uop_t { Pos, Neg, Not } uop;

  unique_ptr<BaseAST> exp;

  string func_name;
  unique_ptr<BaseAST> params;

  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region FuncRParams
// FuncRParams     ::= Exp FuncRParamsList;
class FuncRParamsAST : public BaseAST {
 public:
  vector<unique_ptr<ExpAST>> params;
  vector<RetInfo> parsed_params;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
  const vector<RetInfo>& GetParams() const;
};
#pragma endregion

#pragma region FuncRParamsList
// FuncRParamsList ::= "," Exp FuncRParamsList | epsilon
// 不进树
class FuncRParamsListUnit : public BaseAST {
 public:
  vector<ExpAST*> params;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

#pragma region MulExp
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
#pragma endregion

#pragma region AddExp
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
#pragma endregion

#pragma region RelExp
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
#pragma endregion

#pragma region EqExp
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
#pragma endregion

#pragma region LAndExp
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
#pragma endregion

#pragma region LOrExp
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
#pragma endregion

#pragma region ConstExp
// ConstExp      ::= Exp;
class ConstExpAST : public BaseAST {
 public:
  unique_ptr<BaseAST> exp;
  RetInfo thisRet;

  void Print(ostream& os, int indent) const override;
  void Dump() override;
};
#pragma endregion

void make_indent(ostream& os, int indent);
// ...