// 所有 AST 的基类
#pragma once

#include <iostream>
#include <memory>
#include <string>

#define INDENT_LEN 4

/*

CompUnit  ::= FuncDef;

FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncType  ::= "int";

Block     ::= "{" Stmt "}";
Stmt      ::= "return" Number ";";
Number    ::= INT_CONST;


*/
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Print(int indent) const = 0;
  virtual void Dump(std::ostream& os, int indent) const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Print(int indent) const override;

  void Dump(std::ostream& os, int indent) const override;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Print(int indent) const override;

  void Dump(std::ostream& os, int indent) const override;
};

class FuncTypeAST : public BaseAST {
 public:
  void Print(int indent) const override;

  void Dump(std::ostream& os, int indent) const override;
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;

  void Print(int indent) const override;

  void Dump(std::ostream& os, int indent) const override;
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> number;

  void Print(int indent) const override;

  void Dump(std::ostream& os, int indent) const override;
};

class NumberAST : public BaseAST {
 public:
  int int_const;

  void Print(int indent) const override;

  void Dump(std::ostream& os, int indent) const override;
};

void make_indent(int indent);
// ...
