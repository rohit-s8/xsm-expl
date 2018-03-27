%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pair.h"
#include "types.h"

void put_address(char*,int);
ctr inst_ctr;
%}

%option noyywrap
label	"L_"[a-zA-Z][a-zA-Z0-9_]*	

%%

{label}":\n"	{add_pair(make_pair(yytext,yyleng-2,inst_ctr));}

{label}		{put_address(yytext,yyleng);}

[\n]		{ECHO; if(inst_ctr<2056)inst_ctr++; else inst_ctr+=2;}

.		{ECHO;}

%%

#define WRITE(x,y) do{ if(fwrite(x,y,1,yyout)) {} }while(0)

void put_address(char* yytext, int yyleng){
	char* label_val = strndup(yytext,yyleng);
	char addr[8];
	pair *p;
	for_each_pair(p){
		if((strcmp(p->str,label_val)==0)){
			sprintf(addr,"%d",p->num);
			int len = strlen(addr);
			WRITE(addr,len);
			break;
		}
	}
	if(!p)
		ECHO;
}

int main(int argc, char *argv[]){
	FILE *in,*temp;
	char* exefilename;
	char* exefile(char* filename);

	in = fopen(argv[1],"r");
	temp = fopen(".temp","w");
	inst_ctr=2048;
	yyin = in;
	yyout = temp;
	yylex();
	fclose(in);
	fclose(temp);
	printf("Code size: %d words\n", inst_ctr-2048);

	exefilename = exefile(argv[1]);
	yyin = fopen(".temp","r");
	yyout = fopen(exefilename,"w");
	yylex();
	fclose(yyin);
	fclose(yyout);

	return 0;
}
