%code requires {
  #include <memory>
  #include <string>
  #include <sysy2ir/ir_ast.h>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <sysy2ir/ir_ast.h>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);
extern int yylineno;

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token OPLE OPLT OPGE OPGT OPEQ OPNE OPAND OPOR
%token CONST
%token IF ELSE WHILE CONTINUE BREAK
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block
%type <ast_val> Exp PrimaryExp Number UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
// lv4-const
%type <ast_val> Decl ConstDecl BType ConstDef ConstDeclList ConstInitVal LVal BlockList BlockItem ConstExp
%type <ast_val> VarDecl VarDef InitVal VarDeclList
// lv6-if
%type <ast_val> Stmt OpenStmt ClosedStmt SimpleStmt

%define parse.error verbose

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
// CompUnit      ::= FuncDef;
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

// Decl          ::= ConstDecl | VarDecl;
Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->de = DeclAST::de_t::CONST;
    ast->decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->de = DeclAST::de_t::VAR;
    ast->decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;


// ConstDecl     ::= "const" BType ConstDef ConstDeclList ";";
ConstDecl
  : CONST BType ConstDef ConstDeclList ';' {
    auto ast = new ConstDeclAST();
    ast->btype = unique_ptr<BaseAST>($2);
    // 插入开头def
    ast->const_defs.push_back(unique_ptr<ConstDefAST>((ConstDefAST*)$3));
    auto list = unique_ptr<ConstDeclListUnit>((ConstDeclListUnit*)$4);
    // 插入剩余def
    for (auto it = list->const_defs.rbegin(); it != list->const_defs.rend(); ++it) {
      ast->const_defs.push_back(unique_ptr<ConstDefAST>(*it));
    }
    $$ = ast;
  }
  ;

// ConstDeclList  ::= "," ConstDef ConstDeclList | epsilon
ConstDeclList
  : ',' ConstDef ConstDeclList {
    ((ConstDeclListUnit*)$3)->const_defs.push_back((ConstDefAST*)$2);
    $$ = $3;
  }
  | {
    auto ast = new ConstDeclListUnit();
    $$ = ast;
  }
  ;

// BType         ::= "int";
BType
  : INT {
    auto ast = new BTypeAST();
    ast->btype = string("int");
    $$ = ast;
  }
  ;

// ConstDef      ::= IDENT "=" ConstInitVal;
ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->var_name = *unique_ptr<string>($1);
    ast->const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// ConstInitVal  ::= ConstExp;
ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;


// VarDecl     ::= BType VarDef VarDeclList ";";
VarDecl
  : BType VarDef VarDeclList ';' {
    auto ast = new VarDeclAST();
    ast->btype = unique_ptr<BaseAST>($1);
    // 插入开头def
    ast->var_defs.push_back(unique_ptr<VarDefAST>((VarDefAST*)$2));
    auto list = unique_ptr<VarDeclListUnit>((VarDeclListUnit*)$3);
    // 插入剩余def
    for (auto it = list->var_defs.rbegin(); it != list->var_defs.rend(); ++it) {
      ast->var_defs.push_back(unique_ptr<VarDefAST>(*it));
    }
    $$ = ast;
  }
  ;

// VarDeclList     ::= "," VarDef VarDeclList | epsilon
VarDeclList
  : ',' VarDef VarDeclList {
    ((VarDeclListUnit*)$3)->var_defs.push_back((VarDefAST*)$2);
    $$ = $3;
  }
  | {
    auto ast = new VarDeclListUnit();
    $$ = ast;
  }
  ;

// VarDef        ::= IDENT | IDENT "=" InitVal;
VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->var_name = *unique_ptr<string>($1);
    ast->init_with_val = false;
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->var_name = *unique_ptr<string>($1);
    ast->init_val = unique_ptr<BaseAST>($3);
    ast->init_with_val = true;
    $$ = ast;
  }
  ;

// InitVal       ::= Exp;
InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->func_name = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// FuncType  ::= "int";
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    $$ = ast;
  }
  ;

// Block     ::= "{" BlockList "}";
Block
  : '{' BlockList '}' {
    auto ast = new BlockAST();

    // 插入item
    auto list = unique_ptr<BlockListUnit>((BlockListUnit*)$2);
    for (auto it = list->block_items.rbegin(); it != list->block_items.rend(); ++it) {
      auto ptr = *it;
      ast->block_items.push_back(unique_ptr<BlockItemAST>(ptr));
    }
    $$ = ast;
  }
  ;

// BlockList  ::= BlockItem BlockList | epsilon
BlockList
  : BlockItem BlockList {
    ((BlockListUnit*)$2)->block_items.push_back((BlockItemAST*)$1);
    $$ = $2;
  }
  | {
    auto ast = new BlockListUnit();
    $$ = ast;
  }
  ;

// BlockItem     ::= Decl | Stmt;
BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->bt = BlockItemAST::blocktype_t::decl;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->bt = BlockItemAST::blocktype_t::stmt;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// Stmt            ::= OpenStmt | ClosedStmt
Stmt
  : OpenStmt {
    auto ast = new StmtAST();
    ast->type = StmtAST::stmty_t::open;
    ast->stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ClosedStmt {
    auto ast = new StmtAST();
    ast->type = StmtAST::stmty_t::closed;
    ast->stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

/*
OpenStmt        ::= "if" "(" Exp ")" OpenStmt
                  | "if" "(" Exp ")" ClosedStmt
                  | "if" "(" Exp ")" ClosedStmt "else" OpenStmt
                  | "while" "(" Exp ")" OpenStmt
*/
OpenStmt
  : IF '(' Exp ')' OpenStmt {
    auto ast = new OpenStmtAST();
    ast->type = OpenStmtAST::opty_t::io;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->open = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' ClosedStmt {
    auto ast = new OpenStmtAST();
    ast->type = OpenStmtAST::opty_t::ic;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->closed = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' ClosedStmt ELSE OpenStmt {
    auto ast = new OpenStmtAST();
    ast->type = OpenStmtAST::opty_t::iceo;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->closed = unique_ptr<BaseAST>($5);
    ast->open = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' OpenStmt {
    auto ast = new OpenStmtAST();
    ast->type = OpenStmtAST::opty_t::loop;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->open = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

/*
ClosedStmt      ::= SimpleStmt
                  | "if" "(" Exp ")" ClosedStmt "else" ClosedStmt
                  | "while" "(" Exp ")" ClosedStmt
*/
ClosedStmt
  : SimpleStmt {
    auto ast = new ClosedStmtAST();
    ast->type = ClosedStmtAST::csty_t::simp;
    ast->simple = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IF '(' Exp ')' ClosedStmt ELSE ClosedStmt {
    auto ast = new ClosedStmtAST();
    ast->type = ClosedStmtAST::csty_t::icec;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->tclosed = unique_ptr<BaseAST>($5);
    ast->fclosed = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' ClosedStmt {
    auto ast = new ClosedStmtAST();
    ast->type = ClosedStmtAST::csty_t::loop;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->tclosed = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

/*
SimpleStmt      ::= LVal "=" Exp ";"
                  | Exp ";"
                  | ";"
                  | Block
                  | "return" Exp ";"
                  | "return" ";"
                  | "continue" ";"
                  | "break" ";"
*/
SimpleStmt
  : LVal '=' Exp ';' {
    auto ast = new SimpleStmtAST();
    ast->st = SimpleStmtAST::sstmt_t::storelval;
    ast->lval = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new SimpleStmtAST();
    ast->st = SimpleStmtAST::sstmt_t::expr;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ';' {
    auto ast = new SimpleStmtAST();
    ast->st = SimpleStmtAST::sstmt_t::nullexp;
    $$ = ast;
  }
  | Block {
    auto ast = new SimpleStmtAST();
    ast->st = SimpleStmtAST::sstmt_t::block;
    ast->blk = unique_ptr<BaseAST>($1);
    $$ = ast;
  } 
  | RETURN Exp ';' {
    auto ast = new SimpleStmtAST();
    ast->st = SimpleStmtAST::sstmt_t::ret;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new SimpleStmtAST();
    ast->st = SimpleStmtAST::sstmt_t::nullret;
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new SimpleStmtAST();
    ast->st = SimpleStmtAST::sstmt_t::cont;
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new SimpleStmtAST();
    ast->st = SimpleStmtAST::sstmt_t::brk;
    $$ = ast;
  }
  ;

// Exp         ::= LOrExp;
Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->loexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// LVal          ::= IDENT;
LVal
  : IDENT {
    auto ast = new LValAST();
    ast->var_name = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

// PrimaryExp    ::= "(" Exp ")" | LVal | Number;
PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->pt = PrimaryExpAST::primary_exp_type_t::Brackets;
    ast->content = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->pt = PrimaryExpAST::primary_exp_type_t::LVal;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

  | Number {
    auto ast = new PrimaryExpAST();
    ast->pt = PrimaryExpAST::primary_exp_type_t::Number;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// Number      ::= INT_CONST;
Number
  : INT_CONST {
    auto ast = new NumberAST();
    ast->int_const = $1;
    $$ = ast;
  }
  ;

/*
UnaryExp    ::= PrimaryExp
              | "+" UnaryExp
              | "-" UnaryExp
              | "!" UnaryExp;
*/
UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->uex = UnaryExpAST::uex_t::Primary;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | '+' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->uex = UnaryExpAST::uex_t::OPUnary;
    ast->uop = UnaryExpAST::uop_t::Pos;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '-' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->uex = UnaryExpAST::uex_t::OPUnary;
    ast->uop = UnaryExpAST::uop_t::Neg;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '!' UnaryExp {
    auto ast = new UnaryExpAST();
    ast->uex = UnaryExpAST::uex_t::OPUnary;
    ast->uop = UnaryExpAST::uop_t::Not;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

// MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->mex = MulExpAST::mex_t::Unary;
    ast->uexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp '*' UnaryExp {
    auto ast = new MulExpAST();
    ast->mex = MulExpAST::mex_t::MulOPUnary;
    ast->mop = MulExpAST::mop_t::Mul;
    ast->mexp = unique_ptr<BaseAST>($1);
    ast->uexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    auto ast = new MulExpAST();
    ast->mex = MulExpAST::mex_t::MulOPUnary;
    ast->mop = MulExpAST::mop_t::Div;
    ast->mexp = unique_ptr<BaseAST>($1);
    ast->uexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '%' UnaryExp {
    auto ast = new MulExpAST();
    ast->mex = MulExpAST::mex_t::MulOPUnary;
    ast->mop = MulExpAST::mop_t::Mod;
    ast->mexp = unique_ptr<BaseAST>($1);
    ast->uexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
AddExp
  : MulExp {
    auto ast = new AddExpAST();
    ast->aex = AddExpAST::aex_t::MulExp;
    ast->mexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp '+' MulExp {
    auto ast = new AddExpAST();
    ast->aex = AddExpAST::aex_t::AddOPMul;
    ast->aop = AddExpAST::aop_t::Add;
    ast->aexp = unique_ptr<BaseAST>($1);
    ast->mexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | AddExp '-' MulExp {
    auto ast = new AddExpAST();
    ast->aex = AddExpAST::aex_t::AddOPMul;
    ast->aop = AddExpAST::aop_t::Sub;
    ast->aexp = unique_ptr<BaseAST>($1);
    ast->mexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
RelExp
  : AddExp {
    auto ast = new RelExpAST();
    ast->rex = RelExpAST::rex_t::AddExp;
    ast->aexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp OPLT AddExp {
    auto ast = new RelExpAST();
    ast->rex = RelExpAST::rex_t::RelOPAdd;
    ast->rop = RelExpAST::rop_t::LessThan;
    ast->rexp = unique_ptr<BaseAST>($1);
    ast->aexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp OPLE AddExp {
    auto ast = new RelExpAST();
    ast->rex = RelExpAST::rex_t::RelOPAdd;
    ast->rop = RelExpAST::rop_t::LessEqual;
    ast->rexp = unique_ptr<BaseAST>($1);
    ast->aexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp OPGT AddExp {
    auto ast = new RelExpAST();
    ast->rex = RelExpAST::rex_t::RelOPAdd;
    ast->rop = RelExpAST::rop_t::GreaterThan;
    ast->rexp = unique_ptr<BaseAST>($1);
    ast->aexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp OPGE AddExp {
    auto ast = new RelExpAST();
    ast->rex = RelExpAST::rex_t::RelOPAdd;
    ast->rop = RelExpAST::rop_t::GreaterEqual;
    ast->rexp = unique_ptr<BaseAST>($1);
    ast->aexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
EqExp
  : RelExp {
    auto ast = new EqExpAST();
    ast->eex = EqExpAST::eex_t::RelExp;
    ast->rexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp OPEQ RelExp {
    auto ast = new EqExpAST();
    ast->eex = EqExpAST::eex_t::EqOPRel;
    ast->eop = EqExpAST::eop_t::Equal;
    ast->eexp = unique_ptr<BaseAST>($1);
    ast->rexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | EqExp OPNE RelExp {
    auto ast = new EqExpAST();
    ast->eex = EqExpAST::eex_t::EqOPRel;
    ast->eop = EqExpAST::eop_t::NotEqual;
    ast->eexp = unique_ptr<BaseAST>($1);
    ast->rexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// LAndExp     ::= EqExp | LAndExp "&&" EqExp;
LAndExp
  : EqExp {
    auto ast = new LAndExpAST();
    ast->laex = LAndExpAST::laex_t::EqExp;
    ast->eexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp OPAND EqExp {
    auto ast = new LAndExpAST();
    ast->laex = LAndExpAST::laex_t::LAOPEq;
    ast->laexp = unique_ptr<BaseAST>($1);
    ast->eexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
LOrExp
  : LAndExp {
    auto ast = new LOrExpAST();
    ast->loex = LOrExpAST::loex_t::LAndExp;
    ast->laexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp OPOR LAndExp {
    auto ast = new LOrExpAST();
    ast->loex = LOrExpAST::loex_t::LOOPLA;
    ast->loexp = unique_ptr<BaseAST>($1);
    ast->laexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// ConstExp      ::= Exp;
ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << " at line " << yylineno << endl;
}
