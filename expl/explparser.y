%{
#include <stdlib.h>
#include <stdio.h>
#include "node.h"
#include "types.h"
#include "class.h"
#define YYSTYPE	node* 

int yylex();
int yyerror(const char*);
void codegen(node*);
void put_header();
void terminate();
int bindID(node*);
int verify_field(type,Class,node*);
int bindPTR(node*);
int bindArray(node*);
int bindFunc(node*);
void verify_args(param*,node*);
int translateAST(node*,node*,node*);
int getlabel();
void delete_table(symtable*);
void check_Mdefs(Method*);
void check_Fdefs(symtable*);

YYSTYPE temp;
YYSTYPE _if;
YYSTYPE _while;
ctr errors=0;
extern ctr line_ctr;
ctr infunc=0;
ctr indec=0;
ctr inclass=0;
ctr inlet=0;
Class c,par;
char *cname;
char *funcname;	//available when grammar reduced to Fheader
char *letvar;
%}


%token NUM PLUS MINUS MUL DIV ID ASSIGN READ WRITE IF THEN ELSE ENDIF WHILE DO
%token ENDWHILE BREAK CONTINUE RELOP STR INT STRING DECL ENDDEC MAIN RET BRKP
%token ADR MOD TYPE INIT ALLOC FREE TNULL CLASS DEF END SELF EXTENDS NEW
%token OVERRIDE REF UMINUS AND OR NOT LET IN ENDLET

%right ASSIGN
%left OR
%left AND
%right NOT
%left RELOP
%left PLUS MINUS
%left MUL DIV 
%right MOD
%left UMINUS
%right REF ADR

%start program
%%
program: Typedefs Classdefs Declarations Fdefblock Main
	{
	check_Fdefs(gtable);
	return translateAST($2,$4,$5);
	}

	| Typedefs Classdefs Declarations Main
	{
		check_Fdefs(gtable);
		return translateAST($2,NULL,$4);
	}

       ;

Typedefs: DEF Typelist END	{$$=$2; ttable_update($2);}
		| DEF END	{$$=NULL;}
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

Classdefs: DEF Classes END	{$$=$2;}
		 | DEF END		{$$=NULL;}

Classes: Classes Class	{$$ = make_tree(CON_NODE(),$1,$2);}
	   | Class	{$$ = $1;}
;

Class: Cname '{' DECL Fields MethodDecs ENDDEC 
	{
		Cinstall(makeC(cname,$4,$5,par));
		c = Clookup(cname);
	}
		Fdefblock '}'
	{
		check_Mdefs(c->Mlist);
		inclass=0;
		c = NULL;
		$$ = $8;
	}
	| Cname '{' DECL ENDDEC 
	{
		Cinstall(makeC(cname,NULL,NULL,par));
		c = Clookup(cname);
	}
		Fdefblock '}'
	{
		check_Mdefs(c->Mlist);
		inclass=0;
		c = NULL;
		$$ = $6;
	}

;

Cname: CLASS ID	{inclass=1; cname = $2->varname; par=NULL;}
	 | CLASS ID EXTENDS ID
	{
		inclass=1;
		cname = $2->varname;
		par = Clookup($4->varname);
		if(!par){
			printf("error:%d:class %s not defined\n",$4->varname);
			++errors;
		}
	}
;

MethodDecs: MethodDecs MethodDec	{$$ = make_tree(CON_NODE(),$1,$2);}
		  | MethodDec	{$$=$1;}
;

MethodDec: Type Funcdec';'		{$$ = make_tree($2,$1,$2->right);}
		 ;

Declarations: DECL {indec=1;} Declist ENDDEC
			{
				if(infunc && inclass){
					make_lst($3,funcname,c);
				}
				else if(infunc){
					make_lst($3,funcname,NULL);
				}
				else{
					make_gst($3);
				}
				indec=0;
			}
			| DECL ENDDEC
			{
				if(infunc && inclass){
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
		| ID'('')'
		{
			$$ = FNC_NODE($1->varname);
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
		$$->ctype = c; 
	else
		$$->ptr = lookup(funcname,gtable);
	infunc = 0;
	}
    ;

Fheader: Type ID '('params')'	
       {
		verify_func(c,$1->datatype,$2->varname,$4,0);
		infunc = 1;
		indec = 0;
		funcname = $2->varname;
		if(inclass){
			Method *M = Mlookup(funcname,c->Mlist);
			M->isdef = 1;
		}
		else{
			entry f = lookup(funcname,gtable);
			f->isdef = 1;
		}
	}
	| OVERRIDE Type ID '('params')'
	{
		if(!inclass){
			printf("error:%d:cannot override function outside class\n",
					line_ctr);
			++errors;
		}
		else{
			if(!verify_func(c,$2->datatype,$3->varname,$5,1)){
				printf("error:%d:cannot override function %s.",
						line_ctr,$3->varname);
				printf("No previous definition found\n");
				++errors;
			}
			else{
				Method *M = Mlookup($3->varname,c->Mlist);
				M->c = c;
				M->label = getlabel();
				delete_table(M->ltable);
				M->isdef = 1;
			}
		}
		infunc = 1;
		indec = 0;
		funcname = $3->varname;
	}
	| Type ID '('')'
	{
		verify_func(c,$1->datatype,$2->varname,NULL,0);
		infunc=1;
		indec=0;
		funcname = $2->varname;
		if(inclass){
			Method *M = Mlookup(funcname,c->Mlist);
			M->isdef = 1;
		}
		else{
			entry f = lookup(funcname,gtable);
			f->isdef = 1;
		}
	}
	| OVERRIDE Type ID '('')'
	{
		if(!inclass){
			printf("error:%d:cannot override function outside class\n",
					line_ctr);
			++errors;
		}
		else{
			if(!verify_func(c,$2->datatype,$3->varname,NULL,1)){
				printf("error:%d:cannot override function %s.",
						line_ctr,$3->varname);
				printf("No previous definition found\n");
				++errors;
			}
			else{
				Method *M = Mlookup($3->varname,c->Mlist);
				M->c = c;
				M->label = getlabel();
				delete_table(M->ltable);
				M->isdef = 1;
			}
		}
		infunc = 1;
		indec = 0;
		funcname = $3->varname;
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
		$$->ptr->isdef = 1;
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
			yyerror("cannot free in-built type");
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
	|letstmt	{$$=$1;}
;

retstmt: RET expr';'
{
	type t;
	if(!inclass){
		entry fentry = lookup(funcname,gtable);
		t = fentry->datatype;
	}
	else{
		Method *M = Mlookup(funcname,c->Mlist);
		t = M->ret;
	}
	if($2->datatype != t){
		printf("return type mismatch\n");
		exit(1);
	}
	$$ = make_tree($1,NULL,$2);
}
;

assign: variable ASSIGN expr';'
      {
	if($3->datatype == T_VOID){
		$3->datatype = $1->datatype;
		$3->ctype = $1->ctype;
	}
	if($1->datatype && ($1->datatype!=$3->datatype)){
		yyerror("assignment error.");
		printf("%s is of %s datatype\n",$1->varname,
			printtype($1->datatype));
	}
	if($1->ctype && ($1->ctype!=$3->ctype)){
		yyerror("Incompatible class types");
		if($3->ctype)
			printf("%s is of class type %s and %s is of class type %s\n",
				$1->varname,$1->ctype->name,$3->varname,$3->ctype->name);
		else
			printf("cannot assign non-class data to object %s\n",
					$1->varname);
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
		if($3->datatype == T_VOID){
			$3->datatype = $1->datatype;
			$3->ctype = $1->ctype;
		}
		if(($1->datatype!=$3->datatype)){
			yyerror("assignment error.");
			char *type;
			if(!$1->ctype)
				type = printtype($1->datatype);
			else
				type = printClass($1->ctype);
			printf("%s is of %s type\n",$1->varname,type);
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
	| variable ASSIGN NEW ID'('')'';'
	{
		if(!$1->ctype)
			yyerror("cannot allocate to non class object");
		Class curr = Clookup($4->varname);
		if(!curr){
			printf("error:%d:%s is not a class\n",$4->varname);
			++errors;
		}
		if(!isDescendant(curr,$1->ctype)){
			printf("error:%d:Type mismatch."
					"%s class cannot refer to %s class\n",
					$1->ctype->name,curr->name);
			++errors;
		}
		$3->ctype = curr;	
		temp = make_tree(OP_NODE(O_ASN),$1,ALOC_NODE());
		$$ = make_tree($3,$1,temp);
		$1->par = temp;
	}
	| FAccess ASSIGN NEW ID'('')'';'
	{
		if(!$1->ctype)
			yyerror("cannot allocate to non class object");
		Class curr = Clookup($4->varname);
		if(!curr){
			printf("error:%d:%s is not a class\n",$4->varname);
			++errors;
		}
		if(!isDescendant(curr,$1->ctype)){
			printf("error:%d:Type mismatch."
					"%s class cannot refer to %s class\n",
					$1->ctype->name,curr->name);
			++errors;
		}
		$3->ctype = curr;	
		temp = make_tree(OP_NODE(O_ASN),$1,ALOC_NODE());
		$$ = make_tree($3,$1,temp);
		$1->par = temp;
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

letstmt: LET
	  {
			if(inclass){
				printf("error:%d: let cannot be used in class scope\n",
						line_ctr);
				++errors;
			}
		} 
		'(' ID ASSIGN expr ')'
		{
			bindID($4);
			if(!$4->ptr->isGlobal){
				printf("error:%d:let variable must have the same name as a"
						"global variable\n",line_ctr);
				++errors;
			}

			if($4->datatype != T_INTEGER || $6->datatype != T_INTEGER){
				printf("error:%d:Type mismatch. operands must be int\n",
							line_ctr);
				++errors;
			}
			$4->ptr->inlet = 1;
		}
		IN {inlet++;}
		stmtlist ENDLET
		{
			inlet--;
			$4->ptr->inlet = (inlet)? 1:0;
			temp = make_tree($5,$4,$6);
			$$ = make_tree($1,temp,$11);
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

| expr AND expr
{
	if($1->datatype!=T_BOOL || $3->datatype!=T_BOOL){
		printf("error:%d:Type mismatch. operator && expects boolean operands\n",
				line_ctr);
		++errors;
	}

	$$=make_tree($2,$1,$3);
	$$->datatype = T_BOOL;
}

| expr OR expr
{
	if($1->datatype!=T_BOOL || $3->datatype!=T_BOOL){
		printf("error:%d:Type mismatch. operator || expects boolean operands\n",
				line_ctr);
		++errors;
	}

	$$=make_tree($2,$1,$3);
	$$->datatype = T_BOOL;
}

| NOT expr
{
	if($2->datatype!=T_BOOL){
		printf("error:%d:Type mismatch. operator ! expects boolean operands\n",
				line_ctr);
		++errors;
	}

	$$=make_tree($1,$2,NULL);
	$$->datatype = T_BOOL;
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
| variable
{
	$1->inlet = $1->ptr->inlet;
	$$ = $1;
}
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
| MethodCall	{$$ = $1;}
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
			if($1->ctype){
				printf("error:%d:invlaid access of class member\n",line_ctr);
				++errors;
			}
			else if(!verify_field($1->datatype,NULL,$3)){
				yyerror("field access error");
				printf("field %s not defined for type %s\n",
						$3->varname,printtype($1->datatype));
			}
			$$ = make_tree($3,$1,NULL);
		}
	   | ID'.'ID
		{
			bindID($1);
			if($1->ctype){
				printf("error:%d:invalid access of class members\n",line_ctr);
				++errors;
			}
			else if(!verify_field($1->datatype,NULL,$3)){
				yyerror("field access error");
				printf("field %s not defined for type %s\n",
						$3->varname,printtype($1->datatype));
			}
			$$ = make_tree($3,$1,NULL);
		}
		| SELF'.'ID
		{
			char *fname = $3->varname;
			field m;
			if(!inclass){
				printf("error:%d:cannot resolve \'self\' outside a class\n",
							line_ctr);
				++errors;
			}
			if(!verify_field(NULL,c,$3)){
				printf("error:%d:%s not a member of %s class\n",
						line_ctr,fname,c->name);
				++errors;
			}
			$1->ctype = c;
			$$ = make_tree($3,$1,NULL);
		}
;

MethodCall: ID'.'ID'('Arglist')'
		  {
			bindID($1);
			if(!$1->ctype){
				printf("error:%d:%s is not a class\n",line_ctr,$1->varname);
				++errors;
			}
				$$ = make_tree(FNC_NODE($3->varname),$1,$5);
				bindFunc($$);
		}
		  | ID'.'ID'('')'
		{
			bindID($1);
			if(!$1->ctype){
				printf("error:%d:%s is not a class\n",line_ctr,$1->varname);
				++errors;
			}
				$$ = make_tree(FNC_NODE($3->varname),$1,NULL);
				bindFunc($$);
		}
		|	SELF'.'ID'('Arglist')'
		{
			if(!inclass){
				printf("error:%d:cannot resolve \'self\' outside a class\n",
							line_ctr);
				++errors;
			}
				$1->ctype = c;
				$$ = make_tree(FNC_NODE($3->varname),$1,$5);
				bindFunc($$);
		}
		|	SELF'.'ID'('')'
		{
			if(!inclass){
				printf("error:%d:cannot resolve \'self\' outside a class\n",
							line_ctr);
				++errors;
			}
				$1->ctype = c;
				$$ = make_tree(FNC_NODE($3->varname),$1,NULL);
				bindFunc($$);
		}
		| FAccess'.'ID'('Arglist')'
		{
			$$ = make_tree(FNC_NODE($3->varname),$1,$5);
			bindFunc($$);
		}
		| FAccess'.'ID'('')'
		{
			$$ = make_tree(FNC_NODE($3->varname),$1,NULL);
			bindFunc($$);
		}
;
			
%%

int verify_field(type t, Class c, node *idnode){
	field *list;
	field *f;

	if(c)
		list = c->mlist;
	else
		list = t->flist;
	f = Flookup(list,idnode->varname);
	if(!f)
		return 0;
	idnode->datatype = f->t;
	idnode->ctype = f->c;
	return 1;
}

int bindID(node *idnode){
	symtable *table;

	if(inclass){
		Method *M = Mlookup(funcname,c->Mlist);
		table = M->ltable;
	}
	else{	
		entry fentry = lookup(funcname,gtable);
		table = fentry->ltable;
	}

	idnode->ptr = lookup(idnode->varname,table);
	if(!idnode->ptr && inclass){
		printf("error:%d:variable %s not declared\n",line_ctr,idnode->varname);
		++errors;
		return 0;
	}
	else if(!idnode->ptr)
		idnode->ptr = lookup(idnode->varname,gtable);
	if(!idnode->ptr){
		yyerror("binding error.");
		printf("%s not declared\n",idnode->varname);
		return 0;
	}
	idnode->datatype = idnode->ptr->datatype;
	idnode->ctype = idnode->ptr->ctype;
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
	node *class = funcnode->left;
	param *params;
	type ret;
	int def;

	if(!class){
		funcnode->ptr = lookup(funcnode->varname,gtable);
		if(!funcnode->ptr){
			printf("function %s not declared\n",funcnode->varname);
			return 0;
		}
		params = funcnode->ptr->params;
		ret = funcnode->ptr->datatype;
		def = funcnode->ptr->isdef;
	}
	else{
		Method *M = Mlookup(funcnode->varname,class->ctype->Mlist);
		if(!M){
			printf("error:%d:%s not declared in class %s\n",
					line_ctr,funcnode->varname,class->ctype->name);
			++errors;
			return 0;
		}
		params = M->params;
		ret = M->ret;
	}
	verify_args(params,arglist);
	funcnode->datatype = ret;
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



int translateAST(node *classdefs, node *fdefs, node *main){
	if(!errors){
		put_header();
		codegen(classdefs);
		codegen(fdefs);
		codegen(main);
		terminate();
	}
	else{
		printf("Total number of errors:%d\n",errors);
		exit(1);
	}
	return 0;
}

void check_Mdefs(Method *Mlist){
	Method *M;
	for_each_method(M,Mlist){
		if(!M->isdef){
			printf("Method %s not defined\n",M->name);
			++errors;
		}
	}
}

void check_Fdefs(symtable *table){
	entry e;
	for_each_entry(e,gtable){
		if(e->ltable){
			if(!e->isdef){
				printf("Function %s not defined\n",e->varname);
				++errors;
			}
		}
	}
}

int yyerror(const char* s){
	++errors;
	printf("error:%d:%s\n",line_ctr,s);
	return 1;
}
