#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "node.h"
#include "class.h"

node* make_node(Node nodetype, char *varname, Operator optype,
		type datatype, Class ctype, value *val, int isptr){
	node* t = (node*)malloc(sizeof(node));
	t->nodetype = nodetype;
	t->optype = optype;
	t->isptr = isptr;
	t->varname = varname;
	if(varname!=NULL){
		t->varname = (char*)malloc(100*sizeof(char));
		strcpy(t->varname,varname);
	}
	t->datatype = datatype;
	t->ctype = ctype;
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

int addr_off = 0;	//address offset from BP
symtable *gtable;	//global symbol table
extern ctr line_ctr;

static int get_next_addr(){
	return addr_off;
}

static void alloc(unsigned int size){
	addr_off += size;
}

static void set_addr(int a){
	addr_off = a;
}

void param_args_list(node *root, param *head){
	if(!root)
		return;

	if(root->nodetype==N_CON){
		param_args_list(root->left,head);
		param_args_list(root->right,head);
	}
	else{
		param *p = (param*)malloc(sizeof(param));
		p->datatype = root->datatype;
		p->isptr = root->isptr;
		if(root->nodetype==N_PARAM)
			p->varname = root->varname;

		param *tail = head;
		while(tail->next!=NULL)
			tail = tail->next;
		tail->next = p;
	}
}

entry id_entry(node* idnode, type datatype, Class c, int isptr){
	entry e = (entry)malloc(sizeof(symtable));
	e->varname = strdup(idnode->varname);
	e->datatype = datatype;
	e->ctype = c;
	e->isptr = isptr;
	e->size = 1;
	e->dim1 = 0;
	e->dim2 = 0;
	e->bind_addr = get_next_addr();
	alloc(e->size);
	e->isGlobal = 0;
	e->params = NULL;
	e->ltable = NULL;
	e->next = NULL;
	return e;
}

#define ISNUM(node)\
	node->nodetype == N_VAL && node->datatype == T_INTEGER

entry array_entry(node *array, type datatype, Class c){
	entry e = (entry)malloc(sizeof(symtable));
	node *idnode = array->left;
	node *dimtree = array->right;
	e->varname = strdup(idnode->varname);
	e->datatype = datatype;
	e->ctype = c;
	e->isptr = 0;
	if(dimtree->nodetype == N_CON){
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
	e->isGlobal = 0;
	e->params = NULL;
	e->ltable = NULL;
	e->next = NULL;

	return e;
}

entry func_entry(node *fnode, type t, Class c){
	entry e = (entry)malloc(sizeof(symtable));
	node *params = fnode->right;
	
	e->varname = fnode->varname;
	e->datatype = t;
	e->ctype = c;
	e->isptr = 0;
	e->size = 0;
	e->dim1 = 0;
	e->dim2 = 0;
	e->bind_addr = -100;
	e->isGlobal = 1;
	e->params = (param*)malloc(sizeof(param));
	param_args_list(params,e->params);
	e->ltable = (symtable*)malloc(sizeof(symtable));
	e->next = NULL;
}


void installID(entry e, symtable *table){
	if(table==gtable)
		e->isGlobal = 1;		//for global variables

	if(!table->next){
		table->next = e;
		return;
	}

	entry curr;
	entry prev;
	for_each_entry(curr,table){
		if(strcmp(curr->varname,e->varname)==0){
			printf("error:%d:variable %s already declared\n",
					line_ctr,e->varname);
			exit(1);
		}
		prev = curr;
	}
	prev->next = e;
}

static void installEntries(symtable *table, node *varlist, type t, Class c){
	if(!varlist)
		return;

	switch(varlist->nodetype){
		case N_CON:
			installEntries(table,varlist->left,t,c);
			installEntries(table,varlist->right,t,c);
			break;

		case N_ID:
			installID(id_entry(varlist,t,c,0),table);
			break;

		case N_ARR:
			installID(array_entry(varlist,t,c),table);
			break;

		case N_PTR:
			installID(id_entry(varlist,t,c,1),table);
			break;

		case N_FNC:
			installID(func_entry(varlist,t,c),table);
			break;
	}
}

static void make_symtable(symtable *table, node *decl){
	if(!decl)
		return;

	type t;
	Class c;
	switch(decl->nodetype){
		case N_CON:
			make_symtable(table,decl->left);
			make_symtable(table,decl->right);
			break;

		case N_DEC:
			t = decl->left->datatype;
			c = decl->left->ctype;
			installEntries(table,decl->right,t,c);
			break;
	}
}

void make_gst(node *decl){
	gtable = (symtable*)malloc(sizeof(symtable));
	set_addr(4096);
	make_symtable(gtable,decl);
	installID(func_entry(FNC_NODE("main"),T_INTEGER),gtable);
}

void make_lst(node *decl, const char *funcname, Class c){
	param *p;
	param *params;
	symtable *table;

	if(!c){
		entry fentry = lookup(funcname,gtable);
		params = fentry->params;
		table = fentry->ltable;
	}
	else{
		Method *M = Mlookup(funcname,c->Mlist);
		params = M->params;
		table = M->ltable;
	}

	int num_param=0;
	for_each_param(p,params){
		num_param++;
	}
	set_addr(-num_param-2);
	for_each_param(p,params){
		installID(id_entry(ID_NODE(p->varname),p->datatype,c,p->isptr),table);
	}
	set_addr(1);
	make_symtable(table,decl);
}

entry lookup(char *name, symtable* table){
	entry e;
	for_each_entry(e,table){
		if(strcmp(e->varname,name)==0)
			break;
	}
	return e;
}

int get_id_addr(node *idnode){
	return idnode->ptr->bind_addr;
}

int verify_func(Class c, type ret, const char *funcname, node *params){
	param *params;
	if(!c){
		entry e = lookup(funcname,gtable);
		if(!e){
			printf("function %s not declared\n",funcname);
			exit(1);
		}
		if(e->datatype != ret){
			printf("return type mismatch\n");
			exit(1);
		}
		params = e->params;
	}
	else{
		Method *M = Mlookup(funcname,c->Mlist);
		if(!M){
			printf("functions %s not declared in class %s\n",
					funcname,printClass(c));
			exit(1);
		}
		if(M->ret != ret){
			printf("return type mismatch\n");
			exit(1);
		}
		params = M->plist;
	}

	param *list = (param*)malloc(sizeof(param));
	param_args_list(params,list);

	param *p,*q;
	for(p=params->next,q=list->next; p!=NULL; p=p->next,q=q->next){
		if(!q){
			printf("function %s expects more parameters\n",funcname);
			exit(1);
		}
		else if(p->datatype != q->datatype || p->isptr!=q->isptr){
			yyerror("mismatch in function parameters\n");
			exit(1);
		}
		else if(strcmp(p->varname,q->varname)!=0){
			yyerror("mismatch in function parameters\n");
			exit(1);
		}
	}

	if(!p && q){
		printf("function %s expects less parameters\n",funcname);
		exit(1);
	}
}

int last_addr(symtable *table){
	entry curr,lid;
	lid=table;
	for_each_entry(curr,table){
		if(curr->bind_addr<0)
			continue;
		lid=curr;
	}
	if(lid!=table)
		return (lid->bind_addr+lid->size-1);
	else if(table==gtable)
		return 4095;
	else
		return 0;
}

void print_symtable(symtable *table){
	entry e;
	printf("name\ttype\tsize\tbinding\tdim1\tdim2\tisGlobal\n");
	for_each_entry(e,table){
		printf("%s\t%s\t%-4u\t%-7d\t%-4u\t%-4u\t%-2d\n",
				e->varname,printtype(e->datatype),e->size,
				e->bind_addr,e->dim1,e->dim2,e->isGlobal);
	}
}
