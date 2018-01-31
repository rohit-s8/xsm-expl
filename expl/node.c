#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "node.h"

node* make_node(Node nodetype, char *varname, Operator optype,
		type datatype, value *val){
	node* t = (node*)malloc(sizeof(node));
	t->nodetype = nodetype;
	t->optype = optype;
	t->varname = varname;
	if(varname!=NULL){
		t->varname = (char*)malloc(100*sizeof(char));
		strcpy(t->varname,varname);
	}
	t->datatype = datatype;
	t->val = val;
	t->res_reg=-1;
	t->ptr = NULL;
	t->left = t->right = t->par = NULL;
	return t;
}

value* make_value(int num, const char *str){
	value *v;
	v = (value*)malloc(sizeof(value));
	if(str==NULL)
		v->num = num;
	else{
		char *temp = strdup(str);
		v->str = temp;
	}
	return v;
}

node* make_tree(node *root, node *left, node *right){
	root->left = left;
	root->right = right;
	if(left)
		left->par = root;
	if(right)
		right->par = root;
	return root;
}

node* add_stmt_tree(node *main, node *_new){
	node *temp;
	temp = CON_NODE();
	temp = make_tree(temp,main,_new);
	return temp;
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

char* printtype(type t){
	switch(t){
		case T_INTEGER:
			return "integer";
		case T_STRING:
			return "string";
		case T_BOOL:
			return "bool";
	}
	return NULL;
}

unsigned int next_addr = 4096;
symtable *table = NULL;
extern ctr line_ctr;

unsigned int get_next_addr(){
	return next_addr;
}

static void alloc(unsigned int size){
	next_addr += size;
}


entry single_entry(node* idnode, type datatype){
	entry e = (entry)malloc(sizeof(symtable));
	e->varname = strdup(idnode->varname);
	e->datatype = datatype;
	e->size = 1;
	e->dim1 = 0;
	e->dim2 = 0;
	e->bind_addr = get_next_addr();
	alloc(e->size);
	e->next = NULL;
	return e;
}

#define ISNUM(node)\
	node->nodetype == N_VAL && node->datatype == T_INTEGER

entry array_entry(node *array, type datatype){
	entry e = (entry)malloc(sizeof(symtable));
	node *idnode = array->left;
	node *dimtree = array->right;
	e->varname = strdup(idnode->varname);
	e->datatype = datatype;
	if(dimtree->nodetype != N_VAL){
		node *r = dimtree->left;
		node *c = dimtree->right;
		if(!ISNUM(r) || !ISNUM(c)){
			printf("error:%d:array dimensions must be positive integer constants\n",line_ctr);
			exit(1);
		}
		e->dim1 = r->val->num;
		e->dim2 = c->val->num;
		e->size = e->dim1 * e->dim2;
	}
	else{
		if(dimtree->datatype != T_INTEGER){
			printf("error:%d:array dimensions must be positive integer constants\n",line_ctr);
			exit(1);
		}
		e->dim1 = dimtree->val->num;
		e->dim2 = 0;
		e->size = e->dim1;
	}

	if(e->size >= 1000){
		printf("error:%d:array size too large\n",line_ctr);
		exit(1);
	}

	e->bind_addr = get_next_addr();
	alloc(e->size);
	e->next = NULL;

	return e;
}

void installID(entry e){
	if(!table){
		table = e;
		return;
	}
	entry curr;
	entry prev;
	for_each_entry(curr){
		if(strcmp(curr->varname,e->varname)==0){
			printf("error:%d:variable %s already declared",
					line_ctr,e->varname);
			exit(1);
		}
		prev = curr;
	}
	prev->next = e;
}

entry lookup(char *name){
	entry e;
	for_each_entry(e){
		if(strcmp(e->varname,name)==0)
			break;
	}
	return e;
}

unsigned int get_id_addr(node *idnode){
	return idnode->ptr->bind_addr;
}
