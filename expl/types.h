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
	N_RD, 
	N_WR,
	N_CON,
	N_IF,
	N_WHILE,
	N_BRK,
	N_CNT,
	N_ARR
}Node;

//operator types
typedef enum Operator{
	O_ADD=0,
	O_SUB,
	O_MUL,
	O_DIV,
	O_ASN,
	O_LT,
	O_LTE,
	O_GT,
	O_GTE,
	O_EQ,
	O_NE
}Operator;

//supported data types
typedef enum type{
	T_INTEGER=0,
	T_STRING, 
	T_BOOL
}type;	

char* printop(Operator op);
char* printtype(type t);

#endif
