#ifndef NODE_H_
#define NODE_H_

#include "types.h"

typedef union value{
	int num;
	char *str;
}value;

//node definition
typedef struct node{
	Node nodetype;
	char *varname;
	Operator optype;
	type datatype;
	value *val;
	reg_ind res_reg;
	struct node *left,*right,*par;
}node;

node* make_node(Node nodetype, char *varname, Operator optype,
		type datatype, value *val);
value* make_value(int num, const char *str);
node* make_tree(node *root, node *left, node *right);
node* add_stmt_tree(node *main, node* _new);

#define VALnode(valtype,val)\
	make_node(N_VAL,NULL,NON,valtype,val)
#define IDnode(varname,valtype)\
	make_node(N_ID,varname,NON,valtype,NULL)
#define OPnode(optype)\
	make_node(N_OP,NULL,optype,NON,NULL)
#define makenode(nodetype)\
	make_node(nodetype,NULL,NON,NON,NULL)
#define CONnode()\
	makenode(N_CON)
#define RDnode()\
	makenode(N_RD)
#define WRnode()\
	makenode(N_WR)
#define IFnode()\
	makenode(N_IF)
#define WHILEnode()\
	makenode(N_WHILE)
#define BRKnode()\
	makenode(N_BRK)
#define CNTnode()\
	makenode(N_CNT)

#define VAL_NUM(N)\
	make_value(N,NULL)
#define VAL_STR(S)\
	make_value(0,S)
#endif
