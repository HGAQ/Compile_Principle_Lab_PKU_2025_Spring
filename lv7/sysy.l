%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "sysy.tab.hpp"

using namespace std;

%}




/* 空白符和注释 */
WhiteSpace    [ \t\n\r]*
LineComment   "//".*
BlockComment   \/\*([^\*]*|[\*]+[^\*\/])*[\*]+\/

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

/* Operator */
UOperator     ("!")
MOperator     ("*"|"/"|"%")
AOperator     ("+"|"-")
LOOperator    "||"
LAOperator    "&&"
EOperator     ("=="|"!=")
ROperator     ("<"|">"|"<="|">=")

%%

{WhiteSpace}    { /* 忽略, 不做任何操作 */ }
{LineComment}   { /* 忽略, 不做任何操作 */ }
{BlockComment}   { /* 忽略, 不做任何操作 */ }

"int"         { return INT; }
"return"      { return RETURN; }
"const"       { return CONST; }
"if"          { return IF; }
"else"        { return ELSE; }
"while"       { return WHILE; }
"break"       { return BREAK; }
"continue"       { return CONTINUE; }

{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

{UOperator}   { yylval.str_val = new string(yytext); return UOPERATOR; }
{MOperator}   { yylval.str_val = new string(yytext); return MOPERATOR; }
{AOperator}   { yylval.str_val = new string(yytext); return AOPERATOR; }
{LOOperator}  { yylval.str_val = new string(yytext); return LOOPERATOR;} 
{LAOperator}  { yylval.str_val = new string(yytext); return LAOPERATOR;} 
{EOperator}  { yylval.str_val = new string(yytext); return  EOPERATOR;} 
{ROperator}  { yylval.str_val = new string(yytext); return  ROPERATOR;} 

.               { return yytext[0]; }

%%