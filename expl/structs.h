#ifndef STRUCTS_H_
#define STRUCTS_H_

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
	N_NULL,
	N_SELF,
	N_NEW
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

// struct declarations
struct field;
struct ClassTable;
struct symtable;

//type table definitions
typedef struct typetable{
	char *name;
	unsigned int size;
	struct field *flist;
	struct typetable *next;
}typetable;
typedef typetable* type;	//pointer to type table entry

//field definitions
typedef struct field{
	type t;
	struct ClassTable *c;
	char *name;
	int offset;
	struct field *next;
}field;

// function parameter
typedef struct param{
	type datatype;
	char *varname;
	int isptr;
	struct param *next;
}param;

// Class Methods
typedef struct ClassMethod{
	char *name;
	type ret;
	struct ClassTable *c;
	int isdef;
	int label;
	ctr index;
	param *params;
	struct symtable *ltable;
	struct classMethod *next;
}Method;

// Class Table
typedef struct ClassTable{
	char *name;
	field *mlist;
	Method *Mlist;
	ctr index;
	struct ClassTable *par;
	struct ClassTable *next;
}ClassTable;
typedef ClassTable* Class;

// value for value  node
typedef union value{
	int num;
	char *str;
}value;

// symbol table structure
typedef struct symtable{
	char *varname;
	type datatype;
	Class ctype;
	int isptr;
	unsigned int size;
	unsigned int dim1;
	unsigned int dim2;
	int bind_addr;
	int isGlobal;
	int isdef;
	ctr label;
	param *params;
	struct symtable *ltable;
	struct symtable *next;
}symtable;
typedef symtable* entry;

//node definition
typedef struct node{
	Node nodetype;
	char *varname;
	Operator optype;
	type datatype;
	Class ctype;
	value *val;
	reg_ind res_reg;
	entry ptr;
	int isptr;
	struct node *left,*right,*par;
}node;

#endif
