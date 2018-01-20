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
		yylval=VALnode(INTEGER,VAL_NUM(num));
		return NUM;
		}

{string}	{
		char *temp = strndup(yytext+1,yyleng-2);
		yylval=VALnode(STRING,VAL_STR(temp));
		return STR;
		}

{id}		{
		char *temp = strndup(yytext,yyleng);
		yylval=IDnode(temp,INTEGER);
		return ID;
		}

"read"		{yylval=RDnode(); return READ;}

"write"		{yylval=WRnode(); return WRITE;}

"begin"		{return BEG;}

"end"		{return END;}

"if"		{yylval=IFnode(); return IF;}

"then"		{return THEN;}

"else"		{return ELSE;}

"endif"		{return ENDIF;}

"while"		{yylval=WHILEnode(); return WHILE;}

"do"		{return DO;}

"endwhile"	{return ENDWHILE;}

"break"		{yylval=BRKnode(); return BREAK;}

"continue"	{yylval=CNTnode(); return CONTINUE;}

[<]		{yylval=OPnode(O_LT); return RELOP;}

[>]		{yylval=OPnode(O_GT); return RELOP;}

"<="		{yylval=OPnode(O_LTE); return RELOP;}

">="		{yylval=OPnode(O_GTE); return RELOP;}

"=="		{yylval=OPnode(O_EQ); return RELOP;}

"!="		{yylval=OPnode(O_NE); return RELOP;}

[+]		{yylval=OPnode(O_ADD); return AROP1;}

[-]		{yylval=OPnode(O_SUB); return AROP1;}

[*]		{yylval=OPnode(O_MUL); return AROP2;}

[/]		{yylval=OPnode(O_DIV); return AROP2;}

[=]		{yylval=OPnode(O_ASN); return ASSIGN;}

[ \t]

[\n]		{++line_ctr;}

[();]		{return *yytext;}

.		{eprint(line_ctr,"unknown character\n");exit(1);}

%%
