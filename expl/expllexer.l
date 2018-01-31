%{
#include "y.tab.h"
#include "node.h"
#include <stdlib.h>
#include <string.h>

ctr line_ctr=1;
%}

%option noyywrap
number	[0-9]+
string [\"][^\"]*[\"]
id	[a-z]

%%

{number}	{
		int num=atoi(yytext);
		yylval=VAL_NODE(T_INTEGER,VAL_NUM(num));
		return NUM;
		}

{string}	{
		char *temp = strndup(yytext+1,yyleng-2);
		yylval=VAL_NODE(T_STRING,VAL_STR(temp));
		return STR;
		}

{id}		{
		char *temp = strndup(yytext,yyleng);
		yylval=ID_NODE(temp);
		return ID;
		}

"decl"		{return DECL;}

"enddec"	{return ENDDEC;}

"read"		{yylval=RD_NODE(); return READ;}

"write"		{yylval=WR_NODE(); return WRITE;}

"if"		{yylval=IF_NODE(); return IF;}

"then"		{return THEN;}

"else"		{return ELSE;}

"endif"		{return ENDIF;}

"while"		{yylval=WHILE_NODE(); return WHILE;}

"do"		{return DO;}

"endwhile"	{return ENDWHILE;}

"break"		{yylval=BRK_NODE(); return BREAK;}

"continue"	{yylval=CNT_NODE(); return CONTINUE;}

"int"		{return INT;}

"string"	{return STRING;}

[<]		{yylval=OP_NODE(O_LT); return RELOP;}

[>]		{yylval=OP_NODE(O_GT); return RELOP;}

"<="		{yylval=OP_NODE(O_LTE); return RELOP;}

">="		{yylval=OP_NODE(O_GTE); return RELOP;}

"=="		{yylval=OP_NODE(O_EQ); return RELOP;}

"!="		{yylval=OP_NODE(O_NE); return RELOP;}

[+]		{yylval=OP_NODE(O_ADD); return AROP1;}

[-]		{yylval=OP_NODE(O_SUB); return AROP1;}

[*]		{yylval=OP_NODE(O_MUL); return AROP2;}

[/]		{yylval=OP_NODE(O_DIV); return AROP2;}

[=]		{yylval=OP_NODE(O_ASN); return ASSIGN;}

[ \t]

[\n]		{++line_ctr;}

[();,]		{return *yytext;}

"["		{return *yytext;}

"]"		{return *yytext;}

.		{yyerror("unknown character\n");printf("%c\n",*yytext);exit(1);}

%%
