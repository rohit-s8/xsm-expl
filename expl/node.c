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
	temp = CONnode();
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
		case INTEGER:
			return "integer";
		case STRING:
			return "string";
		case BOOL:
			return "bool";
	}
	return NULL;
}
