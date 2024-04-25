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
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt
%type <ast_val> Exp PrimaryExp Number UnaryExp UnaryOp MulExp AddExp RelExp EqExp LAndExp LOrExp
// lv4
%type <ast_val> Decl ConstDecl BType ConstDef ConstDeclList ConstInitVal LVal BlockList BlockItem ConstExp

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

// Decl          ::= ConstDecl;
Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->de = DeclAST::de_t::cnst;
    ast->decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
// |
// ConstDecl     ::= "const" BType ConstDef ConstDeclList ";";
// ConstDeclList  ::= "," ConstDef ConstDeclList | epsilon

ConstDecl
  : CONST BType ConstDef ConstDeclList ';' {
    auto ast = new ConstDeclAST();
    ast->btype = unique_ptr<BaseAST>($2);
    // 插入开头def
    ast->const_defs.push_back(unique_ptr<ConstDefAST>((ConstDefAST*)$3));
    auto list = unique_ptr<ConstDeclListUnit>((ConstDeclListUnit*)$4);
    // 插入剩余def
    for (auto it = list->const_defs.end() - 1; it >= list->const_defs.begin(); it--) {
      ast->const_defs.push_back(unique_ptr<ConstDefAST>(*it));
    }
    $$ = ast;
  }
  ;

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
    $$ = ast;
  }

// ConstDef      ::= IDENT "=" ConstInitVal;
ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->var_name = *unique_ptr<string>($1);
    ast->const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

// ConstInitVal  ::= ConstExp;
ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
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

// 同上, 不再解释
// FuncType  ::= "int";
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    $$ = ast;
  }
  ;

// Block         ::= "{" {BlockItem} "}";
// |
// Block     ::= "{" BlockItem BlockList "}";
// BlockList  ::= BlockItem BlockList | epsilon
Block
  : '{' BlockItem BlockList '}' {
    auto ast = new BlockAST();
    // 插入开头item
    ast->block_items.push_back(unique_ptr<BlockItemAST>((BlockItemAST*)$2));
    auto list = unique_ptr<BlockListUnit>((BlockListUnit*)$3);
    // 插入剩余item
    for (auto it = list->block_items.end() - 1; it >= list->block_items.begin(); it--) {
      ast->block_items.push_back(unique_ptr<BlockItemAST>(*it));
    }
    $$ = ast;
  }
  ;

BlockList
  : BlockItem BlockList {
    ((BlockListUnit*)$2)->block_items.push_back((BlockItemAST*)$1);
    $$ = $2;
  }
  | {
    auto ast = new BlockListUnit();
    $$ = ast;
  };

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


/*
Stmt          ::= LVal "=" Exp ";"
                | "return" Exp ";";
*/
Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->st = StmtAST::stmttype_t::retn;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->st = StmtAST::stmttype_t::decl;
    ast->lval = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
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

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->uex = UnaryExpAST::uex_t::Primary;
    ast->prim = unique_ptr<BaseAST>($1);
    ast->uop = nullptr;
    ast->uexp = nullptr;
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST();
    ast->uex = UnaryExpAST::uex_t::Unary;
    ast->prim = nullptr;
    ast->uop = unique_ptr<BaseAST>($1);
    ast->uexp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

// UnaryOp     ::= "+" | "-" | "!";
UnaryOp
  : '+' {
    auto ast = new UnaryOPAST();
    ast->uop = UnaryOPAST::uop_t::Pos;
    $$ = ast;
  }
  | '-' {
    auto ast = new UnaryOPAST();
    ast->uop = UnaryOPAST::uop_t::Neg;
    $$ = ast;
  }
  | '!' {
    auto ast = new UnaryOPAST();
    ast->uop = UnaryOPAST::uop_t::Not;
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
  cerr << "error: " << s << endl;
}
