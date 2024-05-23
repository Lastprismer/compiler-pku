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
%token VOID
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef BType Block
%type <ast_val> Exp PrimaryExp Number UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
// lv4 - const
%type <ast_val> Decl ConstDecl ConstDef ConstDeclList ConstInitVal LVal BlockList BlockItem ConstExp
%type <ast_val> VarDecl VarDef InitVal VarDeclList
// lv6 - if
%type <ast_val> Stmt OpenStmt ClosedStmt SimpleStmt
// lv8 - func
%type <ast_val> CompUnit CompUnitList FuncFParams FuncFParamsList FuncFParam FuncRParams FuncRParamsList
// lv9 - arr
%type <ast_val> ArrSize ConstArrVal CAElementList ArrInitVal AIElementList
%type <ast_val> ArrSizeList CAElement AIElement ArrAddr ArrAddrList

%define parse.error verbose

%%

// CompRoot        ::= CompUnitList
CompRoot
  : CompUnitList {
    auto comp_root = make_unique<CompRootAST>();
    auto list = unique_ptr<CompUnitListUnit>(dynamic_cast<CompUnitListUnit*>($1));
    for (auto it = list->comp_units.rbegin(); it != list->comp_units.rend(); ++it) {
      comp_root->comp_units.push_back(unique_ptr<CompUnitAST>(*it));
    }
    ast = move(comp_root);
  }
  ;

// CompUnitList    ::= CompUnit CompUnitList | epsilon
CompUnitList
  : CompUnit CompUnitList {
    (dynamic_cast<CompUnitListUnit*>($2))->comp_units.push_back(dynamic_cast<CompUnitAST*>($1));
    $$ = $2;
  }
  | {
    auto ast = new CompUnitListUnit();
    $$ = ast;
  }
  ;

// CompUnit        ::= FuncDef | Decl
CompUnit
  : FuncDef {
    auto ast = new CompUnitAST();
    ast->ty = CompUnitAST::comp_unit_ty::e_func_def;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Decl {
    auto ast = new CompUnitAST();
    ast->ty = CompUnitAST::comp_unit_ty::e_decl;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// Decl          ::= ConstDecl | VarDecl
Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->de = DeclAST::de_t::e_const;
    ast->decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->de = DeclAST::de_t::e_var;
    ast->decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;


// ConstDecl     ::= "const" BType ConstDef ConstDeclList ";"
ConstDecl
  : CONST BType ConstDef ConstDeclList ';' {
    auto ast = new ConstDeclAST();
    ast->btype = unique_ptr<BaseAST>($2);
    // 插入开头def
    ast->const_defs.push_back(unique_ptr<ConstDefAST>(dynamic_cast<ConstDefAST*>($3)));
    auto list = unique_ptr<ConstDeclListUnit>(dynamic_cast<ConstDeclListUnit*>($4));
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
    (dynamic_cast<ConstDeclListUnit*>($3))->const_defs.push_back(dynamic_cast<ConstDefAST*>($2));
    $$ = $3;
  }
  | {
    auto ast = new ConstDeclListUnit();
    $$ = ast;
  }
  ;

// BType         ::= "int" | "void"
BType
  : INT {
    auto ast = new BTypeAST();
    ast->ty = BTypeAST::btype_t::e_int;
    $$ = ast;
  }
  | VOID {
    auto ast = new BTypeAST();
    ast->ty = BTypeAST::btype_t::e_void;
    $$ = ast;
  }
  ;

/*
ConstDef        ::= IDENT "=" ConstInitVal
                  | IDENT ArrSize "=" ConstArrVal
*/
ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ty = ConstDefAST::def_t::e_int;
    ast->var_name = *unique_ptr<string>($1);
    ast->const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT ArrSize '=' ConstArrVal {
    auto ast = new ConstDefAST();
    ast->ty = ConstDefAST::def_t::e_arr;
    ast->var_name = *unique_ptr<string>($1);
    ast->arr_size = unique_ptr<BaseAST>($2);
    ast->const_init_val = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  ;

// ConstInitVal  ::= ConstExp
ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;


// VarDecl     ::= BType VarDef VarDeclList ";"
VarDecl
  : BType VarDef VarDeclList ';' {
    auto ast = new VarDeclAST();
    ast->btype = unique_ptr<BaseAST>($1);
    // 插入开头def
    ast->var_defs.push_back(unique_ptr<VarDefAST>(dynamic_cast<VarDefAST*>($2)));
    auto list = unique_ptr<VarDeclListUnit>(dynamic_cast<VarDeclListUnit*>($3));
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
    (dynamic_cast<VarDeclListUnit*>($3))->var_defs.push_back(dynamic_cast<VarDefAST*>($2));
    $$ = $3;
  }
  | {
    auto ast = new VarDeclListUnit();
    $$ = ast;
  }
  ;

/*
VarDef          ::= IDENT
                  | IDENT "=" InitVal
                  | IDENT ArrSize
                  | IDENT ArrSize "=" ArrInitVal
*/
VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->init_with_val = false;
    ast->ty = VarDefAST::def_t::e_int;
    ast->var_name = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->init_with_val = true;
    ast->ty = VarDefAST::def_t::e_int;
    ast->var_name = *unique_ptr<string>($1);
    ast->init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT ArrSize {
    auto ast = new VarDefAST();
    ast->init_with_val = false;
    ast->ty = VarDefAST::def_t::e_arr;
    ast->var_name = *unique_ptr<string>($1);
    ast->arr_size = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | IDENT ArrSize '=' ArrInitVal {
    auto ast = new VarDefAST();
    ast->init_with_val = true;
    ast->ty = VarDefAST::def_t::e_arr;
    ast->var_name = *unique_ptr<string>($1);
    ast->arr_size = unique_ptr<BaseAST>($2);
    ast->init_val = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  ;

// InitVal       ::= Exp
InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// ArrSize         ::= "[" ConstExp "]" ArrSizeList
ArrSize
  : '[' ConstExp ']' ArrSizeList {
    auto ast = new ArrSizeAST();
    ast->arr_size.push_back(unique_ptr<ConstExpAST>(dynamic_cast<ConstExpAST*>($2)));
    auto list = unique_ptr<ArrSizeListUnit>(dynamic_cast<ArrSizeListUnit*>($4));
    // 插入剩余
    for (auto it = list->values.rbegin(); it != list->values.rend(); ++it) {
      ast->arr_size.push_back(unique_ptr<ConstExpAST>(*it));
    }
    $$ = ast;
  }
  ;

// ArrSizeList     ::= "[" ConstExp "]" ArrSizeList | epsilon
ArrSizeList
  : '[' ConstExp ']' ArrSizeList {
    (dynamic_cast<ArrSizeListUnit*>($4))->values.push_back(dynamic_cast<ConstExpAST*>($2));
    $$ = $4;
  }
  | {
    auto ast = new ArrSizeListUnit();
    $$ = ast;
  }
  ;

// CAElement       ::= ConstExp | ConstArrVal
CAElement
  : ConstExp {
    auto ast = new CAElementAST();
    ast->ty = CAElementAST::caty_t::e_cexp;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ConstArrVal {
    auto ast = new CAElementAST();
    ast->ty = CAElementAST::caty_t::e_carr;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// ConstArrVal     ::= "{" "}" | "{" CAElement CAElementList "}"
ConstArrVal
  : '{' '}' {
    auto ast = new ConstArrValAST();
    $$ = ast;
  }
  | '{' CAElement CAElementList '}' {
    auto ast = new ConstArrValAST();
    // 插入开头
    ast->values.push_back(unique_ptr<CAElementAST>(dynamic_cast<CAElementAST*>($2)));
    auto list = unique_ptr<CAElementListUnit>(dynamic_cast<CAElementListUnit*>($3));
    // 插入剩余
    for (auto it = list->values.rbegin(); it != list->values.rend(); ++it) {
      ast->values.push_back(unique_ptr<CAElementAST>(*it));
    }
    $$ = ast;
  }
  ;

// CAElementList ::= "," CAElement CAElementList | epsilon
CAElementList
  : ',' CAElement CAElementList {
    (dynamic_cast<CAElementListUnit*>($3))->values.push_back(dynamic_cast<CAElementAST*>($2));
    $$ = $3;
  }
  | {
    auto ast = new CAElementListUnit();
    $$ = ast;
  }
  ;

// AIElement       ::= Exp | ArrInitVal
AIElement
  : Exp {
    auto ast = new AIElementAST();
    ast->ty = AIElementAST::aity_t::e_exp;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ArrInitVal {
    auto ast = new AIElementAST();
    ast->ty = AIElementAST::aity_t::e_arr;
    ast->content = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

// ArrInitVal      ::= "{" "}" | "{" AIElement AIElementList "}"
ArrInitVal
  : '{' '}' {
    auto ast = new ArrInitValAST();
    $$ = ast;
  }
  | '{' AIElement AIElementList '}' {
    auto ast = new ArrInitValAST();
    // 插入开头
    ast->values.push_back(unique_ptr<AIElementAST>(dynamic_cast<AIElementAST*>($2)));
    auto list = unique_ptr<AIElementListUnit>(dynamic_cast<AIElementListUnit*>($3));
    // 插入剩余
    for (auto it = list->values.rbegin(); it != list->values.rend(); ++it) {
      ast->values.push_back(unique_ptr<AIElementAST>(*it));
    }
    $$ = ast;
  }
  ;

// AIElementList    ::= "," AIElement AIElementList | epsilon
AIElementList
  : ',' AIElement AIElementList {
    (dynamic_cast<AIElementListUnit*>($3))->values.push_back(dynamic_cast<AIElementAST*>($2));
    $$ = $3;
  }
  | {
    auto ast = new AIElementListUnit();
    $$ = ast;
  }
  ;

// FuncDef         ::= FuncType IDENT "(" FuncFParams ")" Block
FuncDef
  : BType IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->func_name = *unique_ptr<string>($2);
    ast->params = unique_ptr<BaseAST>($4);
    ast->block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  ;

// FuncFParams     ::= FuncFParam FuncFParamsList | epsilon
FuncFParams
  : FuncFParam FuncFParamsList {
    auto ast = new FuncFParamsAST();
    ast->params.push_back(unique_ptr<FuncFParamAST>(dynamic_cast<FuncFParamAST*>($1)));
    auto list = unique_ptr<FuncFParamsListUnit>(dynamic_cast<FuncFParamsListUnit*>($2));
    for (auto it = list->params.rbegin(); it != list->params.rend(); ++it) {
      ast->params.push_back(unique_ptr<FuncFParamAST>(*it));
    }
    $$ = ast;
  }
  | {
    auto ast = new FuncFParamsAST();
    $$ = ast;
  }
  ;

// FuncFParamsList       ::= "," FuncFParam FuncFParamsList | epsilon
FuncFParamsList
  : ',' FuncFParam FuncFParamsList {
    (dynamic_cast<FuncFParamsListUnit*>($3))->params.push_back(dynamic_cast<FuncFParamAST*>($2));
    $$ = $3;
  }
  | {
    auto ast = new FuncFParamsListUnit();
    $$ = ast;
  }
  ;

/*
FuncFParam      ::= BType IDENT
                  | BType IDENT "[" "]"
                  | BType IDENT "[" "]" ArrSize
*/
FuncFParam
  : INT IDENT {
    auto ast = new FuncFParamAST();
    ast->is_ptr = false;
    ast->param_name = *unique_ptr<string>($2);
    $$ = ast;
  }
  | INT IDENT '[' ']' {
    auto ast = new FuncFParamAST();
    ast->param_name = *unique_ptr<string>($2);
    ast->ptr_size = unique_ptr<BaseAST>(nullptr);
    ast->is_ptr = true;
    $$ = ast;
  }
  | INT IDENT '[' ']' ArrSize {
    auto ast = new FuncFParamAST();
    ast->param_name = *unique_ptr<string>($2);
    ast->ptr_size = unique_ptr<BaseAST>($5);
    ast->is_ptr = true;
    $$ = ast;
  }
  ;

// Block     ::= "{" BlockList "}"
Block
  : '{' BlockList '}' {
    auto ast = new BlockAST();

    // 插入item
    auto list = unique_ptr<BlockListUnit>(dynamic_cast<BlockListUnit*>($2));
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
    (dynamic_cast<BlockListUnit*>($2))->block_items.push_back(dynamic_cast<BlockItemAST*>($1));
    $$ = $2;
  }
  | {
    auto ast = new BlockListUnit();
    $$ = ast;
  }
  ;

// BlockItem     ::= Decl | Stmt
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


// ArrAddr         ::= "[" Exp "]" ArrAddrList
ArrAddr
  : '[' Exp ']' ArrAddrList {
    auto ast = new ArrAddrAST();
    ast->arr_addr.push_back(unique_ptr<ExpAST>(dynamic_cast<ExpAST*>($2)));
    auto list = unique_ptr<ArrAddrListUnit>(dynamic_cast<ArrAddrListUnit*>($4));
    // 插入剩余
    for (auto it = list->addrs.rbegin(); it != list->addrs.rend(); ++it) {
      ast->arr_addr.push_back(unique_ptr<ExpAST>(*it));
    }
    $$ = ast;
  }
  ;

// ArrAddrList     ::= "[" Exp "]" ArrAddrList | epsilon
ArrAddrList
  : '[' Exp ']' ArrAddrList {
    (dynamic_cast<ArrAddrListUnit*>($4))->addrs.push_back(dynamic_cast<ExpAST*>($2));
    $$ = $4;
  }
  | {
    auto ast = new ArrAddrListUnit();
    $$ = ast;
  }
  ;

// LVal            ::= IDENT | IDENT ArrAddr
LVal
  : IDENT {
    auto ast = new LValAST();
    ast->ty = LValAST::lval_t::e_int;
    ast->var_name = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT ArrAddr {
    auto ast = new LValAST();
    ast->ty = LValAST::lval_t::e_arr;
    ast->var_name = *unique_ptr<string>($1);
    ast->arr_param = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

// PrimaryExp    ::= "(" Exp ")" | LVal | Number
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

// Number      ::= INT_CONST
Number
  : INT_CONST {
    auto ast = new NumberAST();
    ast->int_const = $1;
    $$ = ast;
  }
  ;

/*
UnaryExp        ::= PrimaryExp
                  | "+" UnaryExp
                  | "-" UnaryExp
                  | "!" UnaryExp
                  | IDENT "(" FuncRParams ")"
                  | IDENT "(" ")"
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
  | IDENT '(' FuncRParams ')' {
    auto ast = new UnaryExpAST();
    ast->uex = UnaryExpAST::uex_t::FuncWithParam;
    ast->func_name = *unique_ptr<string>($1);
    ast->params = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT '(' ')' {
    auto ast = new UnaryExpAST();
    ast->uex = UnaryExpAST::uex_t::FuncNoParam;
    ast->func_name = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

// FuncRParams     ::= Exp FuncRParamsList
FuncRParams
  : Exp FuncRParamsList {
    auto ast = new FuncRParamsAST();
    ast->params.push_back(unique_ptr<ExpAST>(dynamic_cast<ExpAST*>($1)));
    auto list = unique_ptr<FuncRParamsListUnit>(dynamic_cast<FuncRParamsListUnit*>($2));
    for (auto it = list->params.rbegin(); it != list->params.rend(); ++it) {
      ast->params.push_back(unique_ptr<ExpAST>(*it));
    }
    $$ = ast;
  }
  ;

// FuncRParamsList ::= "," Exp FuncRParamsList | epsilon
FuncRParamsList
  : ',' Exp FuncRParamsList {
    (dynamic_cast<FuncRParamsListUnit*>($3))->params.push_back(dynamic_cast<ExpAST*>($2));
    $$ = $3;
  }
  | {
    auto ast = new FuncRParamsListUnit();
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
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << " at line " << yylineno << endl;
}
