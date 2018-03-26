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
int verify_field(type,node*);
int bindPTR(node*);
int bindArray(node*);
int bindFunc(node*);
void verify_args(param*,node*);
int translateAST(node*);
YYSTYPE temp;
YYSTYPE _if;
YYSTYPE _while;
ctr errors=0;
extern ctr line_ctr;
ctr infunc=0;
ctr indec=0;
ctr inclass=0;
char *cname;
char *funcname;	//available when grammar reduced to Fheader
%}


%token NUM PLUS MINUS MUL DIV ID ASSIGN READ WRITE IF THEN ELSE ENDIF WHILE DO
%token ENDWHILE BREAK CONTINUE RELOP STR INT STRING DECL ENDDEC MAIN RET BRKP
%token ADR MOD TYPE INIT ALLOC FREE TNULL CLASS

%right ASSIGN
%left RELOP
%left PLUS MINUS
%left MUL DIV 
%right MOD
%left UMINUS
%right REF ADR

%start program
%%
program: Typedefs Classes Declarations Fdefblock Main
	{
	$$ = CON_NODE();
	$$ = make_tree($$,$3,$4);
	return translateAST($$);
	}

	| Typedefs Declarations Main
	{
		$$ = $3;
		return translateAST($$);
	}

       ;

Nothing:	{$$=NULL;}
	   ;

Typedefs: Typelist		{ttable_update($1);}
		| Nothing
;

Typelist: Typelist Typedef
		{
			$$ = CON_NODE();
			$$ = make_tree($$,$1,$2);
		}
		| Typedef	{$$=$1;}
;

Typedef: TYPE ID '{' Fields '}'
	   {
			$$ = TYPE_NODE($2->varname);
			$$ = make_tree($$,NULL,$4);
		}
;

Fields: Fields field
	  {
			$$ = CON_NODE();
			$$ = make_tree($$,$1,$2);
		}
		| field		{$$=$1;}
;

field: Type ID';'
	 {
		$$ = DEC_NODE();
		$$ = make_tree($$,$1,$2);
	}
;

Classes: Classes Class	{$$ = make_tree(CON_NODE(),$1,$2);}
	   | Class	{$$ = $1;}
		| Nothing
;

Class: CLASS ID {inclass=1; cname = $2->varname;}
	 '{' DECL Fields MethodDecs ENDDEC';' 
	{
		Cinstall(makeC(cname,$6,$7));
	}
		Fdefblock '}'
	{
		inclass=0;
		$$ = $10;
		$$->ctype = Clookup(cname);
	}
;

MethodDecs: MethodDecs MethodDec	{$$ = make_tree(CON_NODE(),$1,$2);}
		  | MethodDec	{$$=$1;}
;

MethodDec: Type Funcdec		{$$ = make_tree($2,$1,$2->right);}
		 ;

Declarations: DECL {indec=1;} Declist ENDDEC';'
			{
				if(infunc && inclass){
					Class c = Clookup(cname);
					make_lst($3,funcname,c);
				}
				else if(incfunc){
					make_lst($3,funcname,NULL);
				}
				else{
					make_gst($3);
				}
				indec=0;
			}
			| DECL ENDDEC';'
			{
				if(infunc && inclass){
					Class c = Clookup(cname);
					make_lst(NULL,funcname,c);
				}
				else if(infunc){
					make_lst(NULL,funcname,NULL);
				}
				else{
					make_gst(NULL);
				}
				indec=0;
			}
	    ;

Declist: Declist Declaration
	   {
			$$ = CON_NODE();
			$$ = make_tree($$,$1,$2);
		}
       | Declaration	{$$ = $1;}
;

Declaration: Type varlist';'	
		   {
				$$ = DEC_NODE();
				$$ = make_tree($$,$1,$2);
			}
	   ;

Type: INT	{$$ = TYPE_NODE("int");}
    | STRING	{$$ = TYPE_NODE("string");}
	| ID		{$$ = TYPE_NODE($1->varname);}
    ;

varlist: varlist','Decvar
       {
	temp = CON_NODE();
	$$ = make_tree(temp,$1,$3);
	}
       | Decvar		{$$ = $1;}
       ;

Decvar: variable	{$$ = $1;}
	  | Funcdec		{$$ = $1;}
;

variable: ID		
	{
		if(!indec)
			bindID($1);
		$$ = $1;
	}
	| Array		
	{
		if(!indec)
			bindArray($1);
		$$ = $1;
	}
	| ptr
	{
		if(!indec)
			bindPTR($1);
		$$ = $1;
	}
;

ptr: MUL ID %prec REF
   {
		$$ = PTR_NODE($2->varname);
	}
;

Array: ID Dimensions
     {
	temp = ARR_NODE();
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

Funcdec: ID'('params')'
	   {
			$$ = FNC_NODE($1->varname);
			$$ = make_tree($$,NULL,$3);
		}
;

Fdefblock: Fdefblock Fdef
	 {
		temp = CON_NODE();
		$$ = make_tree(temp,$1,$2);
	}
	 | Fdef		{$$ = $1;}
;

Fdef: Fheader Fblock		
    {
	$$ = $2;
	if(inclass)
		$$->ctype = Clookup(cname);
	else
		$$->ptr = lookup(funcname,gtable);
	infunc = 0;
	}
    ;

Fheader: Type ID '('params')'	
       {
		Class c = NULL;
		if(inclass)
			c = Clookup(cname);
		verify_func(c,$1->datatype,$2->varname,$4);
		infunc = 1;
		indec = 0;
		funcname = $2->varname;
	}
	| Type ID '('')'
	{
		Class c = NULL;
		if(inclass)
			c = Clookup(cname);
		verify_func(c,$1->datatype,$2->varname,NULL);
		infunc=1;
		indec=0;
		funcname = $2->varname;
	}
       ;

params: params','param
      {
	temp = CON_NODE();
	$$ = make_tree(temp,$1,$3);
	}
      | param	{$$ = $1;}
;

param: Type ID		{$$ = PARAM_NODE($1->datatype,$2->varname,0);}
	 | Type ptr		{$$ = PARAM_NODE($1->datatype,$2->varname,1);}
     ;

Fblock: '{' Declarations stmtlist retstmt'}'
	  {
			$$ = FND_NODE(funcname);
			$$ = make_tree($$,$3,$4);
		}

	| '{' Declarations retstmt '}'
	{
		$$ = FND_NODE(funcname);
		$$ = make_tree($$,NULL,$3);
	}
      ;

Main: INT MAIN'('')'{infunc=1;funcname="main";}	Fblock
	{
		$$ = $6;
		$$->ptr = lookup(funcname,gtable);
	}
;


stmtlist: stmtlist stmt 	{$$ = add_stmt_tree($1,$2);}
       | stmt		{$$ = $1;}
	;

stmt: READ'('variable')'';'	{$$ = make_tree($1,$3,NULL);}
    | WRITE'('expr')'';'	{$$ = make_tree($1,$3,NULL);}
	| INIT'('')'';'			{$$ = $1;}
	| FREE'('ID')'';'
	{
		bindID($3);
		if(basic_type($3->datatype))
			yyerror("cannot free in built type");
		temp = OP_NODE(O_EQ);
		temp = make_tree(temp,$3,NULL_NODE());
		$$ = make_tree($1,$3,temp);
	}
	|assign		{$$=$1;}
	|ifstmt		{$$=$1;}
	|BREAK';'	{$$=$1;}
	|CONTINUE';'	{$$=$1;}
	|whilestmt	{$$=$1;}
	|retstmt	{$$=$1;}
	|BRKP';'	{$$=$1;}
;

retstmt: RET expr';'
{
	entry fentry = lookup(funcname,gtable);
	if($2->datatype != fentry->datatype){
		printf("return type mismatch\n");
		exit(1);
	}
	$$ = make_tree($1,NULL,$2);
}
;

assign: variable ASSIGN expr';'
      {
	if($3->datatype == T_VOID)
		$3->datatype = $1->datatype;
	if($1->datatype != $3->datatype){
		yyerror("assignment error.");
		printf("%s is of %s datatype\n",$1->varname,
			printtype($1->datatype));
	}
	if($1->isptr && !$3->isptr){
		yyerror("pointer error.");
		printf("pointer %s being assigned %s value\n",$1->varname,
				printtype($3->datatype));
	}
	if(!$1->isptr && $3->isptr){
		yyerror("pointer error.");
		printf("%s being assigned pointer value\n",$1->varname);
	}
	$$ = make_tree($2,$1,$3);
	}
	| FAccess ASSIGN expr';'
	{
		if($3->datatype == T_VOID)
			$3->datatype = $1->datatype;
		if($1->datatype != $3->datatype){
			yyerror("assignment error.");
			printf("%s is of %s datatype\n",$1->varname,
				printtype($1->datatype));
		}
		$$ = make_tree($2,$1,$3);
	}
	| variable ASSIGN ALLOC'('')'';'
	{
		if(basic_type($1->datatype))
			yyerror("cannot allocate to in-built type");
		$$ = make_tree($2,$1,$3);
	}
	| FAccess ASSIGN ALLOC'('')'';'
	{
		if(basic_type($1->datatype))
			yyerror("cannot allocate to in-built type");
		$$ = make_tree($2,$1,$3);
	}
; 

ifstmt: IF'('expr')'THEN stmtlist ELSE stmtlist ENDIF';'
      {
	if($3->datatype != T_BOOL)
		yyerror("invalid expression in ()\n");

	temp=CON_NODE(); _if=make_tree(temp,$6,$8);
	$$=make_tree($1,$3,_if);
      }

      | IF'('expr')' THEN stmtlist ENDIF';'
      {
	if($3->datatype != T_BOOL)
		yyerror("invalid expression in ()\n");

	temp=CON_NODE(); _if=make_tree(temp,$6,NULL);
	$$=make_tree($1,$3,_if);
      }
;

whilestmt: WHILE'('expr')' DO stmtlist ENDWHILE';'
	 {
		if($3->datatype != T_BOOL)
			yyerror("invalid expression in ()\n");

		$$=make_tree($1,$3,$6);
	 }
	 ;

expr: expr PLUS expr	
    {
	if($1->datatype!=T_INTEGER || $3->datatype!=T_INTEGER){
	yyerror("Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = T_INTEGER;
	$$->isptr = $1->isptr | $3->isptr;
    }

    | expr MINUS expr
    {
	if($1->datatype!=T_INTEGER || $3->datatype!=T_INTEGER){
	yyerror("Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = T_INTEGER;
	$$->isptr = $1->isptr | $3->isptr;
    }

	| expr MUL expr
    {
	if($1->isptr || $3->isptr)
		printf("operation not allowed on pointers\n");
	if($1->datatype!=T_INTEGER || $3->datatype!=T_INTEGER){
	yyerror("Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = T_INTEGER;
    }

	| expr DIV expr
    {
	if($1->isptr || $3->isptr)
		printf("operation not allowed on pointers\n");
	if($1->datatype!=T_INTEGER || $3->datatype!=T_INTEGER){
	yyerror("Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = T_INTEGER;
    }

	| expr MOD expr
    {
	if($1->isptr || $3->isptr)
		printf("operation not allowed on pointers\n");
	if($1->datatype!=T_INTEGER || $3->datatype!=T_INTEGER){
	yyerror("Type mismatch.");
	printf("Operator \'%s\' expects integer operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = T_INTEGER;
    }

	| MINUS expr %prec UMINUS
	{
	if($2->isptr)
		printf("operation not allowed on pointers\n");
		if($2->datatype!=T_INTEGER){
			yyerror("Type mismatch.");
			printf("Operator \'%s\' expects integer operands\n",
				printop($2->optype));
		}
		temp = VAL_NODE(T_INTEGER,VAL_NUM(0));
		$$ = make_tree($1,temp,$2);
		$$->datatype = T_INTEGER;
	}

| expr RELOP expr
    {  
	if($1->datatype == T_VOID)
		$1->datatype = $3->datatype;
	else if($3->datatype == T_VOID)
		$3->datatype = $1->datatype;
	if($1->isptr || $3->isptr)
		printf("operation not allowed on pointers\n");
	if($1->datatype==T_STRING || $3->datatype==T_STRING){
	yyerror("Type mismatch.");
	printf("Operator \'%s\' cannot evaluate string operands\n",
		printop($2->optype));
	}
	$$ = make_tree($2,$1,$3);
	$$->datatype = T_BOOL;
    }

| '('expr')'	{$$=$2;}
| variable	{$$=$1;}
| FAccess	{$$=$1;}
| ID'('Arglist')'
{
	$$ = make_tree(FNC_NODE($1->varname),NULL,$3);
	bindFunc($$);
}
| ID'('')'
{
	$$ = make_tree(FNC_NODE($1->varname),NULL,NULL);
	bindFunc($$);
}
| ADR variable
{
	$$ = make_tree($1,NULL,$2);
	$$->datatype = $2->datatype;
	$$->isptr = 1;
}
| NUM	{$$=$1;}
| STR	{$$=$1;}
| TNULL	{$$=$1; $$->datatype = T_VOID;}
;

Arglist: Arglist','Arg	{$$ = make_tree(CON_NODE(),$1,$3);}
	   | Arg		{$$ = $1;}
;

Arg: expr
   {
		$$ = make_tree(ARG_NODE(),$1,NULL);
		$$->datatype = $1->datatype;
		$$->isptr = $1->isptr;
	}
   ;

FAccess: FAccess'.'ID
		{
			if(!verify_field($1->datatype,$3)){
				yyerror("field access error");
				printf("field %s not defined for type %s\n",
						$3->varname,printtype($1->datatype));
			}
			$$ = make_tree($3,$1,NULL);
		}
	   | ID'.'ID
		{
			bindID($1);
			if(!verify_field($1->datatype,$3)){
				yyerror("field access error");
				printf("field %s not defined for type %s\n",
						$3->varname,printtype($1->datatype));
			}
			$$ = make_tree($3,$1,NULL);
		}
;
			
%%

int verify_field(type t, node *idnode){
	field *f;
	f = Flookup(t->flist,idnode->varname);
	if(!f)
		return 0;
	idnode->datatype = f->t;
	return 1;
}

int bindID(node *idnode){
	entry fentry = lookup(funcname,gtable);
	idnode->ptr = lookup(idnode->varname,fentry->ltable);
	if(!idnode->ptr)
		idnode->ptr = lookup(idnode->varname,gtable);
	if(!idnode->ptr){
		yyerror("binding error.");
		printf("%s not declared\n",idnode->varname);
		return 0;
	}
	idnode->datatype = idnode->ptr->datatype;
	idnode->isptr = idnode->ptr->isptr;
	return 1;
}

int bindPTR(node *ptrnode){
	entry fentry = lookup(funcname,gtable);
	ptrnode->ptr = lookup(ptrnode->varname,fentry->ltable);
	if(!ptrnode->ptr)
		ptrnode->ptr = lookup(ptrnode->varname,gtable);
	if(!ptrnode->ptr){
		yyerror("binding error.\n");
		printf("%s not declared\n",ptrnode->varname);
		return 0;
	}
	else if(!ptrnode->ptr->isptr){
		yyerror("binding error.");
		printf("%s is not a pointer\n",ptrnode->varname);
		return 0;
	}
	ptrnode->datatype = ptrnode->ptr->datatype;
	ptrnode->isptr = 0;
	return 1;
}

int bindArray(node *array){
	node *idnode = array->left;
	node *dimtree = array->right;

	if(!bindID(idnode))
		return 0;
/*
	entry fentry = lookup(funcname,gtable);
	idnode->ptr = lookup(idnode->varname,fentry->ltable);
	if(!idnode->ptr)
		idnode->ptr = lookup(idnode->varname,gtable);
	if(!idnode->ptr){
		yyerror("variable ");
		printf("%s not declared\n",idnode->varname);
		return 0;
	}
*/
	if(!idnode->ptr->dim1 && !idnode->isptr){
		yyerror("variable ");
		printf("%s not declared as array\n",idnode->varname);
		return 0;
	}
	if(dimtree->nodetype==N_CON){
		if(!idnode->ptr->dim2){
			yyerror("array is 1 dimensional");
			return 0;
		}
		node *r = dimtree->left;
		node *c = dimtree->right;
		if(r->datatype != T_INTEGER || c->datatype != T_INTEGER){
			yyerror("array indices must be integers");
			return 0;
		}
	}
	else{
		if(idnode->ptr->dim2 && !idnode->isptr){
			yyerror("array is 2 dimensional");
			return 0;
		}
		if(dimtree->datatype != T_INTEGER){
			yyerror("array indices must be integers");
			return 0;
		}
	}

	array->datatype = idnode->datatype;
	array->varname = idnode->varname;
	array->isptr = 0;
	return 1;
}

int bindFunc(node *funcnode){
	node *arglist = funcnode->right;

	funcnode->ptr = lookup(funcnode->varname,gtable);
	if(!funcnode->ptr){
		printf("function %s not declared\n",funcnode->varname);
		return 0;
	}

	verify_args(funcnode->ptr->params,arglist);
	funcnode->datatype = funcnode->ptr->datatype;
	return 1;
}

void verify_args(param *list, node *args){
	param *args_type = (param*)malloc(sizeof(param));
	param_args_list(args,args_type);

	param *p,*q;
	for(p=list->next,q=args_type->next;p!=NULL;p=p->next,q=q->next){
		if(!q){
			yyerror("function expects more arguments");
			exit(1);
		}

		if(q->datatype == T_VOID)
			q->datatype = p->datatype;
		if(p->datatype != q->datatype){
			yyerror("parameter type mismatch");
			exit(1);
		}

		if(p->isptr != q->isptr){
			yyerror("pointer mismatch");
			exit(1);
		}
	}

	if(q){
		yyerror("function expects less arguments");
		exit(1);
	}
}



int translateAST(node *pr){
	if(!errors){
		put_header();
		codegen(pr);
		terminate();
	}
	else{
		printf("Total number of errors:%d\n",errors);
		exit(1);
	}
	return 0;
}


int yyerror(const char* s){
	++errors;
	printf("error:%d:%s\n",line_ctr,s);
	return 1;
}
