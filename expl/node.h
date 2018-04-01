#ifndef NODE_H_
#define NODE_H_

#include "structs.h"

#define for_each_entry(e,table)\
	for(e=table->next;e!=NULL;e=e->next)
#define for_each_param(p,plist)\
	for(p=plist->next;p!=NULL;p=p->next)

extern symtable *gtable;
node* make_node(Node nodetype, char *varname, Operator optype,
		type datatype, Class ctype, value *val, int isptr);
value* make_value(int num, const char *str);
node* make_tree(node *root, node *left, node *right);
node* add_stmt_tree(node *main, node* _new);
void param_args_list(node *root, param *head);
entry id_entry(node *idnode, type t, Class c, int isptr);
entry array_entry(node *arraynode, type t, Class c);
entry func_entry(node *funcnode, type t, Class c);
void installID(entry e, symtable *table);
entry lookup(char* name, symtable *table);
void make_lst(node *decl, const char* funcname, Class c);
void make_gst(node *decl);
int get_id_addr(node *idnode);
int verify_func(Class c, type ret, const char *funcname, node *params,
				int Override);
int last_addr(symtable *table);

/** Value node (number or string constant) **/
#define VAL_NODE(datatype,val)\
	make_node(N_VAL,NULL,NON,datatype,NULL,val,0)

/** Identifier node **/
#define ID_NODE(varname)\
	make_node(N_ID,varname,NON,NULL,NULL,NULL,0)

/** Pointer deferencing/declaration node **/
#define PTR_NODE(varname)\
	make_node(N_PTR,varname,NON,NULL,NULL,NULL,0)

/** Operator node **/
#define OP_NODE(optype)\
	make_node(N_OP,NULL,optype,NULL,NULL,NULL,0)

/** Type node **/
#define TYPE_NODE(tname)\
	make_node(N_TYPE,tname,NON,Tlookup(tname),Clookup(tname),NULL,0)

/** parameter node **/
#define PARAM_NODE(datatype,paramname,isptr)\
	make_node(N_PARAM,paramname,NON,datatype,NULL,NULL,isptr)

/* function call node */
#define FNC_NODE(funcname)\
	make_node(N_FNC,funcname,NON,NULL,NULL,NULL,0)

/* function defintion node */
#define FND_NODE(funcname)\
	make_node(N_FND,funcname,NON,NULL,NULL,NULL,0)

#define makenode(nodetype)\
	make_node(nodetype,NULL,NON,NULL,NULL,NULL,0)
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

/** declaration node **/
#define DEC_NODE()\
	makenode(N_DEC)

/** array node **/
#define ARR_NODE()\
	makenode(N_ARR)

/** return statement node **/
#define RET_NODE()\
	makenode(N_RET)

/** argument node **/
#define ARG_NODE()\
	makenode(N_ARG)

/** breakpoint node **/
#define BRKP_NODE()\
	makenode(N_BRKP)

 /** initialize node **/
#define INIT_NODE()\
	makenode(N_INIT)

/** alloc node **/
#define ALOC_NODE()\
	makenode(N_ALOC)

/** free node **/
#define FREE_NODE()\
	makenode(N_FREE)

/** NULL node **/
#define NULL_NODE()\
	makenode(N_NULL)

/** SELF node **/
#define SELF_NODE()\
	makenode(N_SELF)

/** NEW node **/
#define NEW_NODE()\
	makenode(N_NEW)

/** make integer or string values **/
#define VAL_NUM(N)\
	make_value(N,NULL)
#define VAL_STR(S)\
	make_value(0,S)
#endif
