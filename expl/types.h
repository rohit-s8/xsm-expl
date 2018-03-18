#ifndef TYPES_H_
#define TYPES_H_

typedef unsigned int	reg_ind;
typedef unsigned int	ctr;

//non type
#define NON -1

//node types
typedef enum Node{
	N_VAL=0,
	N_OP, 
	N_ID,
	N_PTR,
	N_RD, 
	N_WR,
	N_CON,
	N_IF,
	N_WHILE,
	N_BRK,
	N_CNT,
	N_BRKP,
	N_ARR,
	N_RET,
	N_TYPE,
	N_PARAM,
	N_FNC,		//function call
	N_FND,		//function definition
	N_ARG,
	N_DEC,
	N_INIT,
	N_ALOC,
	N_FREE,
	N_NULL
}Node;

//operator types
typedef enum Operator{
	O_ADD=0,
	O_SUB,
	O_MUL,
	O_DIV,
	O_MOD,
	O_ASN,
	O_LT,
	O_LTE,
	O_GT,
	O_GTE,
	O_EQ,
	O_NE,
	O_ADR,
	O_AND,
	O_OR,
	O_NOT
}Operator;

//type table definitions

struct field;

typedef struct typetable{
	char *name;
	unsigned int size;
	struct field *flist;
	struct typetable *next;
}typetable;

typedef typetable* type;	//pointer to type table entry
extern typetable *tt;
#define for_each_type(t)\
	for(t=tt->next;t!=NULL;t=t->next)

//field definitions
typedef struct field{
	type t;
	char *name;
	int offset;
	struct field *next;
}field;
#define for_each_field(f,flist)\
	for(f=flist->next;f!=NULL;f=f->next)

#include "node.h"
struct node;
type Tlookup(const char *name);
void Tinstall(type newT);
type makeT(const char *name);
void get_fields(struct node* ftree, field *head);
unsigned int Tsize(type t);
field* Flookup(field *list, const char *name);
int basic_type(type t);
void ttable_update(struct node* ttree);
void ttable_init();
char* printop(Operator op);
char* printtype(type t);

#define T_INTEGER	Tlookup("int")
#define T_STRING	Tlookup("string")
#define T_BOOL		Tlookup("bool")
#define T_VOID		Tlookup("void")

#endif
