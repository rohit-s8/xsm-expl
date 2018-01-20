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

/** Value node (number or string constant) **/
#define VAL_NODE(datatype,val)\
	make_node(N_VAL,NULL,NON,datatype,val)

/** Identifier node **/
#define ID_NODE(varname,datatype)\
	make_node(N_ID,varname,NON,datatype,NULL)

/** Operator node **/
#define OP_NODE(optype)\
	make_node(N_OP,NULL,optype,NON,NULL)

#define makenode(nodetype)\
	make_node(nodetype,NULL,NON,NON,NULL)
/** Connector node **/
#define CON_NODE()\
	makenode(N_CON)

/** Read instruction node **/
#define RD_NODE()\
	makenode(N_RD)

/** Write instruction node **/
#define WR_NODE()\
	makenode(N_WR)

/** if construct node **/
#define IF_NODE()\
	makenode(N_IF)

/** while construct node **/
#define WHILE_NODE()\
	makenode(N_WHILE)

/** break statement node **/
#define BRK_NODE()\
	makenode(N_BRK)

/** continue statement node **/
#define CNT_NODE()\
	makenode(N_CNT)

/** make integer or string values **/
#define VAL_NUM(N)\
	make_value(N,NULL)
#define VAL_STR(S)\
	make_value(0,S)
#endif
