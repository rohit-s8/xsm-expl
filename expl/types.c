#include<stdlib.h>
#include "types.h"
#include "node.h"

typetable *tt;
int off;

static void reset_off(){
	off=0;
}

static int get_offset(){
	return off;
}

static void update_off(){
	++off;
}

void ttable_init(){
	tt = (typetable*)malloc(sizeof(typetable));
	
	Tinstall(makeT("int"));
	Tinstall(makeT("string"));
	Tinstall(makeT("bool"));
	Tinstall(makeT("void"));
}

static void get_typenames(node *ttree){
	if(!ttree)
		return;

	char *tname;
	switch(ttree->nodetype){
		case N_CON:
			get_typenames(ttree->left);
			get_typenames(ttree->right);
			break;
		case N_TYPE:
			tname = ttree->varname;
			Tinstall(makeT(tname));
			break;
	}
}

static void put_fields(node *ttree){
	if(!ttree)
		return;

	type t;
	char *tname;
	switch(ttree->nodetype){
		case N_CON:
			put_fields(ttree->left);
			put_fields(ttree->right);
			break;
		case N_TYPE:
			tname = ttree->varname;
			t = Tlookup(tname);
			reset_off();
			get_fields(ttree->right,t->flist);
			t->size = get_offset();
			break;
	}
}

void ttable_update(node *ttree){
	get_typenames(ttree);
	put_fields(ttree);
}

void Tinstall(type newT){
	type curr_t;
	type prev_t = tt;
	for_each_type(curr_t){
		if(strcmp(curr_t->name,newT->name)==0){
			printf("type redefinition not allowed\n");
			exit(1);
		}
		prev_t = curr_t;
	}
	prev_t->next = newT;
}

type makeT(const char* name){
	type t;
	t = (type)malloc(sizeof(typetable));
	t->name = name;
	if(strcmp(name,"int")==0 || strcmp(name,"string")==0){
		t->size=1;
		t->flist = NULL;
	}
	else if(strcmp(name,"bool")==0 || strcmp(name,"void")==0){
		t->size=0;
		t->flist=NULL;
	}
	else
		t->flist = (field*)malloc(sizeof(field));

	t->next = NULL;
	return t;
}

void get_fields(node *ftree, field *head){
	if(!head)
		return;

	if(!ftree)
		return;

	if(get_offset()==8){
		printf("user defined type can have maximum 8 fields\n");
		return;
	}

	field *f,*temp;
	type t;
	char *tname;
	switch(ftree->nodetype){
		case N_CON:
			get_fields(ftree->left,head);
			get_fields(ftree->right,head);
			break;
		case N_DEC:
			tname = ftree->left->varname;
			t = Tlookup(tname);
			if(!t){
				printf("type %s not defined\n",tname);
				exit(1);
			}

			f = (field*)malloc(sizeof(field));
			f->t = t;
			f->name = ftree->right->varname;
			f->offset = get_offset();
			update_off();
			
			for(temp=head;temp->next!=NULL;temp=temp->next);
			temp->next=f;
			break;
	}
}

type Tlookup(const char *tname){
	type t;
	for_each_type(t){
		if(strcmp(t->name,tname)==0)
			break;
	}
	return t;
}

field* Flookup(field *list, const char *name){
	field *f;
	for_each_field(f,list){
		if(strcmp(f->name,name)==0)
			break;
	}
	return f;
}

int basic_type(type t){
	if(t==T_INTEGER || t==T_STRING || t==T_BOOL || t==T_VOID)
		return 1;
	return 0;
}

unsigned int Tsize(type t){
	return t->size;
}

char* printtype(type t){
	return t->name;
}

char* printop(Operator op){
	switch(op){
		case O_ADD:
			return "+";
		case O_SUB:
			return "-";
		case O_MUL:
			return "*";
		case O_DIV:
			return "/";
		case O_LT:
			return "<";
		case O_LTE:
			return "<=";
		case O_GT:
			return ">";
		case O_GTE:
			return ">=";
		case O_EQ:
			return "==";
		case O_NE:
			return "!=";
	}
	return NULL;
}
