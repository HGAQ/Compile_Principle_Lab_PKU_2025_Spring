%code requires {
  #include <memory>
  #include <string>
  #include <vector>
  #include "include/ast.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "include/ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

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
//%union {
//  std::string *str_val;
//  int int_val;
//}

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  std::vector<std::unique_ptr<BaseAST>> *vect_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT 
%token <str_val> UOPERATOR AOPERATOR MOPERATOR LOOPERATOR LAOPERATOR EOPERATOR ROPERATOR 
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt 
%type <ast_val> Exp PrimaryExp UnaryExp MulExp AddExp LOrExp LAndExp EqExp RelExp
%type <ast_val> Decl ConstDecl ConstDef ConstInitVal ConstExp BlockItem LVal VarDecl VarDef InitVal 
%type <ast_val> FuncFParam FuncRParam DFuncDef  
%type <ast_val> StmtIF
%type <vect_val> VBlockItem VConstDef VVarDef VFuncFParam VFuncRParam VFuncDef 
%type <int_val> Number
%type <str_val> UnaryOp MulOp AddOp LOrOp LAndOp EqOp RelOp

%%

CompUnit 
  : VFuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->vfunc_def = unique_ptr<vector<unique_ptr<BaseAST>>>($1);
    ast = move(comp_unit);
  }

VFuncDef
  : DFuncDef {
    auto vect = new vector<unique_ptr<BaseAST>>();
    vect -> push_back(unique_ptr<BaseAST>($1));
    $$ = vect;
  }
  | VFuncDef DFuncDef {
    auto vect = $1;
    vect -> push_back(unique_ptr<BaseAST>($2));
    $$ = vect;
  }
  ;

DFuncDef :
  FuncDef {
    auto ast = new DFuncDefAST();
    ast->func_def = unique_ptr<BaseAST>($1);
    ast->type = 0;
    $$ = ast;
  }
  | Decl{
    auto ast = new DFuncDefAST();
    ast->decl = unique_ptr<BaseAST>($1);
    ast->type = 1;
    $$ = ast;
  }

FuncDef 
  :  FuncType IDENT '(' VFuncFParam ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->vfunc_param = unique_ptr<vector<unique_ptr<BaseAST>>>($4);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($6);
    ast->type = 0;
    $$ = ast;
  }
  | FuncType IDENT '('  ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->vfunc_param = make_unique<vector<unique_ptr<BaseAST>>>();
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->type = 1;
    $$ = ast;
  }
  ;

VFuncFParam
  : FuncFParam{
    auto vect = new vector<unique_ptr<BaseAST>>();
    vect-> push_back(unique_ptr<BaseAST>($1));
    $$ = vect;
  }
  | VFuncFParam ',' FuncFParam{
    auto vect = $1;
    vect -> push_back(unique_ptr<BaseAST>($3));
    $$ = vect;
  }
;

FuncFParam 
  : FuncType IDENT{
    auto ast = new FuncFParamAST();
    ast->btype = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    $$ = ast;
  }

// ...
// 同上, 不再解释
FuncType : INT {
    auto ast = new FuncTypeAST();
    ast->type = "int";
    $$ = ast;
  }
  | VOID {
    auto ast = new FuncTypeAST();
    ast->type = "void";
    $$ = ast;
  }
  ;

Block
  : '{' VBlockItem '}' {
    //auto stmt = unique_ptr<string>($2);
    //$$ = new string("{ " + *stmt + " }");
    auto ast = new BlockAST();
    ast->vblock_item = unique_ptr<vector<unique_ptr<BaseAST>>>($2);
    $$ = ast;
  }
  //|'{' '}'{
  //  auto ast = new BlockAST();
  //  $$ = ast;
  //}
  ;

VBlockItem
  : {
    auto vect = new vector<unique_ptr<BaseAST>>();
    $$ = vect;
  }
  | VBlockItem BlockItem{
    auto vect = $1;
    vect -> push_back(unique_ptr<BaseAST>($2));
    $$ = vect;
  };

BlockItem
  : Stmt{
  auto ast = new BlockItemAST();
  ast -> stmt = unique_ptr<BaseAST>($1);
  ast -> type = 0;
  $$ = ast;
}
| Decl{
  auto ast = new BlockItemAST();
  ast -> decl = unique_ptr<BaseAST>($1);
  ast -> type = 1;
  $$ = ast;
}
;

Decl
  : ConstDecl{
    auto ast = new DeclAST();
    ast -> constdecl = unique_ptr<BaseAST>($1);
    ast -> type = 0;
    $$ = ast;
  }
  | VarDecl{
    auto ast = new DeclAST();
    ast -> vardecl = unique_ptr<BaseAST>($1);
    ast -> type = 1;
    $$ = ast;
  }
;

ConstDecl
  : CONST FuncType VConstDef ';'{
    auto ast = new ConstDeclAST();
    ast->btype = unique_ptr<BaseAST>($2);
    ast->vconst_def = unique_ptr<vector<unique_ptr<BaseAST>>>($3);
    $$ = ast;
  }
;

VarDecl
  : FuncType VVarDef ';'{
    auto ast = new VarDeclAST();
    ast->btype = unique_ptr<BaseAST>($1);
    ast->vvar_def = unique_ptr<vector<unique_ptr<BaseAST>>>($2);
    $$ = ast;
  }
;


VConstDef
: ConstDef {
    auto vect = new vector<unique_ptr<BaseAST>>();
    vect-> push_back(unique_ptr<BaseAST>($1));
    $$ = vect;
  }
  | VConstDef ',' ConstDef{
    auto vect = $1;
    vect -> push_back(unique_ptr<BaseAST>($3));
    $$ = vect;
  };

VVarDef
: VarDef {
    auto vect = new vector<unique_ptr<BaseAST>>();
    vect-> push_back(unique_ptr<BaseAST>($1));
    $$ = vect;
  }
  | VVarDef ',' VarDef{
    auto vect = $1;
    vect -> push_back(unique_ptr<BaseAST>($3));
    $$ = vect;
  };


//BType
//  : INT {
//    auto ast = new BTypeAST();
//    ast->btype = "int";
//    $$ = ast;
//  }
//  ;

ConstDef
  : IDENT '=' ConstInitVal{
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->const_initval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

VarDef
  : 
  IDENT {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->type = 0;
    $$ = ast;
  }
  | IDENT '=' InitVal{
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->initval = unique_ptr<BaseAST>($3);
    ast->type = 1;
    $$ = ast;
  }
  ;


ConstInitVal
  : ConstExp{
    auto ast = new ConstInitValAST();
    ast->const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

InitVal
  : Exp{
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;


ConstExp
  : Exp{
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;


Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast -> exp = unique_ptr<BaseAST>($2);
    ast -> type = 0;
    $$ = ast;//new string("return " + *number + ";");
  }
  | LVal '=' Exp ';'
  {
    auto ast = new StmtAST();
    ast -> lval = unique_ptr<BaseAST>($1);
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  | Block
  {
    auto ast = new StmtAST();
    ast -> block = unique_ptr<BaseAST>($1);
    ast -> type = 2;
    $$ = ast;//new string("return " + *number + ";");
  }
  | Exp ';'
  {
    auto ast = new StmtAST();
    ast -> exp = unique_ptr<BaseAST>($1);
    ast -> type = 3;
    $$ = ast;//new string("return " + *number + ";");
  }
  | RETURN ';' {
    auto ast = new StmtAST();
    ast -> type = 4;
    $$ = ast;//new string("return " + *number + ";");
  }
  | ';' {
    auto ast = new StmtAST();
    ast -> type = 5;
    $$ = ast;//new string("return " + *number + ";");
  }
  | IF '(' Exp ')' StmtIF ELSE Stmt{
    auto ast = new StmtAST();
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> stmtif = unique_ptr<BaseAST>($5);
    ast -> stmtelse = unique_ptr<BaseAST>($7);
    ast -> type = 6;
    $$ = ast;//new string("return " + *number + ";");
  }
  | IF '(' Exp ')' Stmt { //if if if (if (if else) else) if if if 
    auto ast = new StmtAST();
    ast -> type = 7;
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> stmtif = unique_ptr<BaseAST>($5);
    $$ = ast;//new string("return " + *number + ";");
  }
  | WHILE '(' Exp ')' Stmt { //if if if (if (if else) else) if if if 
    auto ast = new StmtAST();
    ast -> type = 8;
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> stmtif = unique_ptr<BaseAST>($5);
    $$ = ast;//new string("return " + *number + ";");
  }
  | BREAK ';' {
    auto ast = new StmtAST();
    ast -> type = 9;
    $$ = ast;//new string("return " + *number + ";");
  }
  | CONTINUE ';' {
    auto ast = new StmtAST();
    ast -> type = 10;
    $$ = ast;//new string("return " + *number + ";");
  }
  ;

StmtIF: RETURN Exp ';' {
    auto ast = new StmtAST();
    ast -> exp = unique_ptr<BaseAST>($2);
    ast -> type = 0;
    $$ = ast;//new string("return " + *number + ";");
  }
  | LVal '=' Exp ';'
  {
    auto ast = new StmtAST();
    ast -> lval = unique_ptr<BaseAST>($1);
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  | Block
  {
    auto ast = new StmtAST();
    ast -> block = unique_ptr<BaseAST>($1);
    ast -> type = 2;
    $$ = ast;//new string("return " + *number + ";");
  }
  | Exp ';'
  {
    auto ast = new StmtAST();
    ast -> exp = unique_ptr<BaseAST>($1);
    ast -> type = 3;
    $$ = ast;//new string("return " + *number + ";");
  }
  | RETURN ';' {
    auto ast = new StmtAST();
    ast -> type = 4;
    $$ = ast;//new string("return " + *number + ";");
  }
  | ';' {
    auto ast = new StmtAST();
    ast -> type = 5;
    $$ = ast;//new string("return " + *number + ";");
  }
  | IF '(' Exp ')' StmtIF ELSE StmtIF{
    auto ast = new StmtAST();
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> stmtif = unique_ptr<BaseAST>($5);
    ast -> stmtelse = unique_ptr<BaseAST>($7);
    ast -> type = 6;
    $$ = ast;//new string("return " + *number + ";");
  }
  | WHILE '(' Exp ')' StmtIF { //if if if (if (if else) else) if if if 
    auto ast = new StmtAST();
    ast -> type = 8;
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> stmtif = unique_ptr<BaseAST>($5);
    $$ = ast;//new string("return " + *number + ";");
  }
  | BREAK ';' {
    auto ast = new StmtAST();
    ast -> type = 9;
    $$ = ast;//new string("return " + *number + ";");
  }
  | CONTINUE ';' {
    auto ast = new StmtAST();
    ast -> type = 10;
    $$ = ast;//new string("return " + *number + ";");
  }
  ;



LVal
  : IDENT{
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
;


Exp
  : LOrExp{
    auto ast = new ExpAST();
    ast->loexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp{
    auto ast = new UExpAST();
    ast->pexp = unique_ptr<BaseAST>($1);
    ast->type = 0;
    $$ = ast;
  }
  | UnaryOp UnaryExp{
    auto ast = new UExpAST();
    ast -> uop = *unique_ptr<string>($1);
    ast -> uexp = unique_ptr<BaseAST>($2);
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  | IDENT '(' VFuncRParam ')'{
    auto ast = new UExpAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> vfunc_param = unique_ptr<vector<unique_ptr<BaseAST>>>($3);
    ast -> type = 2;
    $$ = ast;//new string("return " + *number + ";");
  }
  | IDENT '('  ')'{
    auto ast = new UExpAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> vfunc_param = make_unique<vector<unique_ptr<BaseAST>>>();
    ast -> type = 3;
    $$ = ast;//new string("return " + *number + ";");
  }
  ;


VFuncRParam
  : FuncRParam{
    auto vect = new vector<unique_ptr<BaseAST>>();
    vect-> push_back(unique_ptr<BaseAST>($1));
    $$ = vect;
  }
  | VFuncRParam ',' FuncRParam{
    auto vect = $1;
    vect -> push_back(unique_ptr<BaseAST>($3));
    $$ = vect;
  };

FuncRParam: LOrExp{
    auto ast = new ExpAST();
    ast->loexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };



PrimaryExp
  :  '(' Exp ')' {
    auto ast = new PExpAST();
    ast -> exp = unique_ptr<BaseAST>($2);
    ast -> type = 0;
    $$ = ast;//new string("return " + *number + ";");
  }
  | Number{
    auto ast = new PExpAST();
    ast -> number = $1;
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  | LVal {
    auto ast = new PExpAST();
    ast->lval =  unique_ptr<BaseAST>($1);
    ast -> type = 2;
    $$ = ast;
  };

MulExp
  : UnaryExp{
    auto ast = new MExpAST();
    ast->uexp = unique_ptr<BaseAST>($1);
    ast->type = 0;
    $$ = ast;
  }
  | MulExp MulOp UnaryExp{
    auto ast = new MExpAST();
    ast -> mop = *unique_ptr<string>($2);
    ast -> uexp = unique_ptr<BaseAST>($3);
    ast -> mexp = unique_ptr<BaseAST>($1);
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  ;

AddExp
  : MulExp{
    auto ast = new AExpAST();
    ast->mexp = unique_ptr<BaseAST>($1);
    ast->type = 0;
    $$ = ast;
  }
  | AddExp AddOp MulExp{
    auto ast = new AExpAST();
    ast -> aop = *unique_ptr<string>($2);
    ast -> mexp = unique_ptr<BaseAST>($3);
    ast -> aexp = unique_ptr<BaseAST>($1);
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  ;

LOrExp
  : LAndExp{
    auto ast = new LOExpAST();
    ast->laexp = unique_ptr<BaseAST>($1);
    ast->type = 0;
    $$ = ast;
  }
  | LOrExp LOrOp LAndExp{
    auto ast = new LOExpAST();
    ast -> loop = *unique_ptr<string>($2);
    ast -> laexp = unique_ptr<BaseAST>($3);
    ast -> loexp = unique_ptr<BaseAST>($1);
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  ;

LAndExp
  : EqExp{
    auto ast = new LAExpAST();
    ast->eexp = unique_ptr<BaseAST>($1);
    ast->type = 0;
    $$ = ast;
  }
  | LAndExp LAndOp EqExp{
    auto ast = new LAExpAST();
    ast -> laop = *unique_ptr<string>($2);
    ast -> eexp = unique_ptr<BaseAST>($3);
    ast -> laexp = unique_ptr<BaseAST>($1);
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  ;

EqExp
  : RelExp{
    auto ast = new EExpAST();
    ast->rexp = unique_ptr<BaseAST>($1);
    ast->type = 0;
    $$ = ast;
  }
  | EqExp EqOp RelExp{
    auto ast = new EExpAST();
    ast -> eop = *unique_ptr<string>($2);
    ast -> rexp = unique_ptr<BaseAST>($3);
    ast -> eexp = unique_ptr<BaseAST>($1);
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  ;

RelExp
  : AddExp{
    auto ast = new RExpAST();
    ast->aexp = unique_ptr<BaseAST>($1);
    ast->type = 0;
    $$ = ast;
  }
  | RelExp RelOp AddExp{
    auto ast = new RExpAST();
    ast -> rop = *unique_ptr<string>($2);
    ast -> aexp = unique_ptr<BaseAST>($3);
    ast -> rexp = unique_ptr<BaseAST>($1);
    ast -> type = 1;
    $$ = ast;//new string("return " + *number + ";");
  }
  ;

UnaryOp
  : UOPERATOR{
    $$ = $1;
  }
  |
    AOPERATOR{
    $$ = $1;
  }
;

MulOp
  : MOPERATOR{
    $$ = $1;
  }
;

AddOp
  :  AOPERATOR{
    $$ = $1;
  }
;

RelOp
  :  ROPERATOR{
    $$ = $1;
  }
;

EqOp
  :  EOPERATOR{
    $$ = $1;
  }
;

LOrOp
  :  LOOPERATOR{
    $$ = $1;
  }
;

LAndOp
  :  LAOPERATOR{
    $$ = $1;
  }
;

Number
  : INT_CONST {
    //auto ast = new NumberAST();
    //ast -> num = *unique_ptr<string>($1);
    //$$ = new string(to_string($1));
    $$ = $1;
  }
  ;



%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}