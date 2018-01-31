%{
#include <stdlib.h>
#include <stdio.h>
#include "node.h"
#include "types.h"
#define YYSTYPE	node* 

int yylex();
int yyerror(const char*);
void codegen(node*);
void put_header();
void terminate();
int bindID(node*);
YYSTYPE temp;
YYSTYPE _if;
YYSTYPE _while;
ctr errors=0;
extern ctr line_ctr;
type t;
%}


%token NUM AROP1 AROP2 ID ASSIGN READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE RELOP STR INT STRING DECL ENDDEC
%right ASSIGN
%left RELOP
%left AROP1
%left AROP2

%%
program: Declarations stmlist
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

Declarations: DECL Declist ENDDEC';'
	    ;

Declist: Declist Declaration
       | Declaration
;

Declaration: Type varlist';'
	   ;

Type: INT	{t = T_INTEGER;}
    | STRING	{t = T_STRING;}
    ;

varlist: varlist','variable
       | variable
       ;

variable: ID		{installID(single_entry($1,t));}
	| Array		{installID(array_entry($1,t));}
;

Array: ID Dimensions
     {
	temp = makenode(N_ARR);
	$$ = make_tree(temp,$1,$2);
	}
	
     ;

Dimensions: Dimensions '['expr']'
	  {
		temp = CON_NODE();
		$$ = make_tree(temp,$1,$3);
	  }
	  | '['expr']'		{$$ = $2;}
;


stmlist: stmlist stmt 	{$$ = add_stmt_tree($1,$2);}
       | stmt		{$$ = $1;}
	;

stmt: READ'('ID')'';'		{bindID($3); $$ = make_tree($1,$3,NULL);}
    | READ'('Array')'';'	{bindArray($3); $$=make_tree($1,$3,NULL);}
    | WRITE'('expr')'';'	{$$ = make_tree($1,$3,NULL);}
|assign		{$$=$1;}
|ifstmt		{$$=$1;}
|BREAK';'	{$$=$1;}
|CONTINUE';'	{$$=$1;}
|whilestmt	{$$=$1;}
;

assign: ID ASSIGN expr';'
      {
	bindID($1);
	if($1->datatype != $3->datatype){
		yyerror("Type mismatch.");
		printf("%s is of %s datatype\n",$1->varname,
			printtype($1->datatype));
	}
	$$ = make_tree($2,$1,$3);}

|	Array ASSIGN expr';'
	{
		bindArray($1);
		if($1->datatype != $3->datatype){
			yyerror("Type mismatch.");
			printf("%s is of %s datatype\n",
				$1->left->varname,printtype($1->datatype));
		}
		$$ = make_tree($2,$1,$3);
	}
      ;

ifstmt: IF'('expr')'THEN stmlist ELSE stmlist ENDIF';'
      {
	if($3->datatype == T_STRING)
		yyerror("invalid expression in ()\n");

	temp=CON_NODE(); _if=make_tree(temp,$6,$8);
	$$=make_tree($1,$3,_if);
      }

      | IF'('expr')' THEN stmlist ENDIF';'
      {
	if($3->datatype == T_STRING)
		yyerror("invalid expression in ()\n");

	temp=CON_NODE(); _if=make_tree(temp,$6,NULL);
	$$=make_tree($1,$3,_if);
      }
;

whilestmt: WHILE'('expr')' DO stmlist ENDWHILE';'
	 {
		if($3->datatype != T_BOOL)
			yyerror("invalid expression in ()\n");

		$$=make_tree($1,$3,$6);
	 }
	 ;

expr: expr AROP1 expr	
    {
	if($1->datatype!=T_INTEGER || $3->datatype!=T_INTEGER){
	yyerror("Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = T_INTEGER;
    }

    | expr AROP2 expr
    {
	if($1->datatype!=T_INTEGER || $3->datatype!=T_INTEGER){
	yyerror("Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = T_INTEGER;
    }

| expr RELOP expr
    {
	if($1->datatype!=T_INTEGER || $3->datatype!=T_INTEGER){
	yyerror("Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = T_BOOL;
    }

| '('expr')'	{$$=$2;}
| ID	{bindID($1); $$=$1;}
| Array	{bindArray($1); $$=$1;}
| NUM	{$$=$1;}
| STR	{$$=$1;}
;
%%

int bindID(node *idnode){
	idnode->ptr = lookup(idnode->varname);
	if(!idnode->ptr){
		yyerror("variable ");
		printf("%s not declared\n",idnode->varname);
		return 0;
	}
	idnode->datatype = idnode->ptr->datatype;
	return 1;
}

int bindArray(node *array){
	node *idnode = array->left;
	node *dimtree = array->right;

	idnode->ptr = lookup(idnode->varname);
	if(!idnode->ptr){
		yyerror("variable ");
		printf("%s not declared\n",idnode->varname);
		return 0;
	}
	if(!idnode->ptr->dim1){
		yyerror("variable ");
		printf("%s not declared as array\n",idnode->varname);
		return 0;
	}
	if(dimtree->datatype != T_INTEGER){
		node *r = dimtree->left;
		node *c = dimtree->right;
		if(r->datatype != T_INTEGER || c->datatype != T_INTEGER){
			yyerror("array indices must be integers\n");
			return 0;
		}
	}

	array->datatype = idnode->datatype = idnode->ptr->datatype;
	return 1;
}

int yyerror(const char* s){
	++errors;
	printf("error:%d:%s",line_ctr,s);
	return 1;
}
