%{
#include <stdlib.h>
#include <stdio.h>
#include "node.h"
#include "types.h"
#define YYSTYPE	node* 

int yylex();
int eprint(ctr, const char*);
int yyerror(const char*);
void codegen(node*);
void put_header();
void terminate();
YYSTYPE temp;
YYSTYPE _if;
YYSTYPE _while;
ctr errors=0;
extern ctr line_ctr;
%}


%token NUM AROP1 AROP2 ID BEG END ASSIGN READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE RELOP STR
%right ASSIGN
%left RELOP
%left AROP1
%left AROP2

%%
program: BEG stmlist END
	{
	if(!errors){
		put_header();
		codegen($2);
		terminate();
	}
	else{
		printf("Total number of errors:%d\n",errors);
		exit(1);
	}
	return 0;
	}
       ;

stmlist: stmlist stmt 	{$$ = add_stmt_tree($1,$2);}
       | stmt		{$$ = $1;}
	;

stmt: READ'('ID')'';'		{$$ = make_tree($1,$3,NULL);}
    | WRITE'('expr')'';'	{$$ = make_tree($1,$3,NULL);}
|assign		{$$=$1;}
|ifstmt		{$$=$1;}
|BREAK';'	{$$=$1;}
|CONTINUE';'	{$$=$1;}
|whilestmt	{$$=$1;}
;

assign: ID ASSIGN expr';'
      {
	if($1->datatype != $3->datatype){
		eprint(line_ctr,"Type mismatch");
		printf("%s is of %s datatype\n",$1->varname,
			printtype($1->datatype));
	}
	$$ = make_tree($2,$1,$3);}
      ;

ifstmt: IF'('expr')'THEN stmlist ELSE stmlist ENDIF';'
      {
	if($3->datatype == STRING)
		eprint(line_ctr,"invalid expression in ()");

	temp=CONnode(); _if=make_tree(temp,$6,$8);
	$$=make_tree($1,$3,_if);
      }

      | IF'('expr')' THEN stmlist ENDIF';'
      {
	if($3->datatype == STRING)
		eprint(line_ctr,"invalid expression in ()");

	temp=CONnode(); _if=make_tree(temp,$6,NULL);
	$$=make_tree($1,$3,_if);
      }
;

whilestmt: WHILE'('expr')' DO stmlist ENDWHILE';'
	 {
		if($3->datatype == STRING)
			eprint(line_ctr,"invalid expression in ()");

		$$=make_tree($1,$3,$6);
	 }
	 ;

expr: expr AROP1 expr	
    {
	if($1->datatype!=INTEGER || $3->datatype!=INTEGER){
	eprint(line_ctr,"Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = INTEGER;
    }

    | expr AROP2 expr
    {
	if($1->datatype!=INTEGER || $3->datatype!=INTEGER){
	eprint(line_ctr,"Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = INTEGER;
    }

| expr RELOP expr
    {
	if($1->datatype!=INTEGER || $3->datatype!=INTEGER){
	eprint(line_ctr,"Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = BOOL;
    }

| '('expr')'	{$$=$2;}
| ID	{$$=$1;}
| NUM	{$$=$1;}
| STR	{$$=$1;}
;
%%


int eprint(ctr line, const char* s){
	++errors;
	fprintf(stderr,"error:%d:%s\n",line,s);
	return 1;
}

int yyerror(const char* s){
	++errors;
	printf("error: %s\n",s);
	return 1;
}
