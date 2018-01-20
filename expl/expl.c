#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "y.tab.h"
#include "node.h"

/** print instruction **/
#define printi(...)\
	fprintf(target_file, __VA_ARGS__)

/** MOV Rx,N **/
#define MOV_RN(x,N)\
	printi("MOV R%d,%d\n",x,N)

/** MOV Rx,S **/
#define MOV_RS(x,S)\
	printi("MOV R%d,\"%s\"\n",x,S)

/** MOV Rx,Ry **/
#define MOV_RR(x,y)\
	printi("MOV R%d,R%d\n",x,y)

/** MOV Rx,[y] **/
#define MOV_RA(x,y)\
	printi("MOV R%d,[%d]\n",x,y)

/** MOV [x],Ry **/
#define MOV_AR(x,y)\
	printi("MOV [%d],R%d\n",x,y)

/** MOV [x],[y] **/
#define MOV_AA(x,y)\
	printi("MOV [%d],[%d]\n",x,y)

/** ADD Rx,Ry **/
#define ADD(x,y)\
	printi("ADD R%d,R%d\n",x,y)

/** SUB Rx,Ry **/
#define SUB(x,y)\
	printi("SUB R%d,R%d\n",x,y)

/** MUL Rx,Ry **/
#define MUL(x,y)\
	printi("MUL R%d,R%d\n",x,y)

/** DIV Rx,Ry **/
#define DIV(x,y)\
	printi("DIV R%d,R%d\n",x,y)

/** MOV [ID_ADDRESS=x],Ry **/
#define ASN(x,y)\
	MOV_AR(x,y)

/** LT Rx,Ry **/
#define LT(x,y)\
	printi("LT R%d,R%d\n",x,y)

/** LE Rx,Ry **/
#define LE(x,y)\
	printi("LE R%d,R%d\n",x,y)

/** GT Rx,Ry **/
#define GT(x,y)\
	printi("GT R%d,R%d\n",x,y)

/** GE Rx,Ry **/
#define GE(x,y)\
	printi("GE R%d,R%d\n",x,y)

/** EQ Rx,Ry **/
#define EQ(x,y)\
	printi("EQ R%d,R%d\n",x,y)

/** NE Rx,Ry **/
#define NE(x,y)\
	printi("NE R%d,R%d\n",x,y)

/** INC Rx **/
#define INC(x)\
	printi("INC R%d\n",x)

/** DEC Rx **/
#define DEC(x)\
	printi("DEC R%d\n",x)

/** PUSH Rx **/
#define PUSH(x)\
	printi("PUSH R%d\n",x)

/** POP Rx **/
#define POP(x)\
	printi("POP R%d\n",x)

/** INT N **/
#define INT(N)\
	printi("INT %d\n",N);

/** CALL N **/
#define CALL(N)\
	printi("CALL %d\n",N)

/** RET **/
#define RET printi("RET\n")

/** L_Xy: **/
#define LABEL_DEC(X,y)\
	printi("L_%s%d:\n",X,y)

/** JMP/CALL/JZ/JNZ [Rx,] label **/
#define LABEL_JMP(label,dom)\
	printi("JMP L_%s%d\n",label,dom)
#define LABEL_CALL(label,dom)\
	printi("CALL L_%s%d\n",label,dom)
#define LABEL_JZ(label,dom,reg)\
	printi("JZ R%d,L_%s%d\n",reg,label,dom);
#define LABEL_JNZ(label,dom,reg)\
	printi("JNZ R%d,L_%s%d\n",reg,label,dom)

/** Label domains **/
#define IF_DOM curr_if
#define WHILE_DOM curr_while

/** Set IF label domain and put declaration **/
#define GETIN_IF IF_DOM = ++if_ctr; LABEL_DEC("IF",IF_DOM)
/** IF label exit **/
#define GETOUT_IF LABEL_DEC("ENDIF",IF_DOM)

/** Set WHILE label domain and put declaration **/
#define GETIN_WHILE\
	++in_while; WHILE_DOM = ++while_ctr;\
	LABEL_DEC("WHILE",WHILE_DOM)
/** WHILE label exit **/
#define GETOUT_WHILE\
	LABEL_DEC("ENDWHILE",WHILE_DOM);\
	--in_while; --WHILE_DOM

/** Test if in loop **/
#define IN_LOOP if(in_while)

/** test if identifier is being assigned **/
#define PAR root->par
#define ID_ASSIGN\
       	PAR->nodetype==N_OP && PAR->optype==O_ASN && PAR->left==root

reg_ind r;
ctr if_ctr;
ctr while_ctr;
ctr curr_while;
ctr curr_if;
ctr in_while=0;
FILE *target_file;

static void reset_counters(){
	if_ctr=0;
	while_ctr=0;
}

static void free_all_reg(){
	r=0;
}

static reg_ind get_reg(node *n){
	n->res_reg = r++;
	return (r-1);
}

static void free_reg(node *n){
	n->res_reg=-1;
	--r;
}

static void use_reg(node *n, reg_ind r){
	n->res_reg = r;
}

void put_header(){
	fprintf(target_file,"%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
		0,2056,0,0,0,0,0,0);
	printi("MOV SP,5021\n");
	printi("MOV BP,4096\n");
}

static void put_write(node *n){
	reg_ind temp = r++;
	MOV_RS(temp,"Write");
	PUSH(temp);
	MOV_RN(temp,-2);
	PUSH(temp);
	PUSH(n->res_reg);
	PUSH(temp);
	PUSH(temp);
	CALL(0);
	int i;
	for(i=0;i<5;i++)
       	POP(temp);
	--r;
}

static void put_read(node *n){
	reg_ind temp = r++;
	MOV_RS(temp,"Read");
	PUSH(temp);
	MOV_RN(temp,-1);
	PUSH(temp);
	PUSH(n->res_reg);
	PUSH(temp);
	PUSH(temp);
	CALL(0);
	int i;
	for(i=0;i<5;i++)
       	POP(temp);
	--r;
}

void terminate(){
	INT(10);
}

void codegen(node *root){
	reg_ind next,left_reg,right_reg;
	reg_ind cond;
	node *temp;
	int addr;

	if(root==NULL)
		return;

	switch(root->nodetype){
		case N_CON:
			codegen(root->left);
			codegen(root->right);
			break;
		case N_VAL:
			next = get_reg(root);
			if(root->datatype == INTEGER)
				MOV_RN(next,root->val->num);
			else if(root->datatype == STRING)
				MOV_RS(next,root->val->str);
			break;
		case N_ID:
			next = get_reg(root);
			addr = *(root->varname)-'a'+4096;
			if(!(root->par->nodetype==N_RD || ID_ASSIGN))
				MOV_RA(next,addr);
			else if(root->par->nodetype==N_RD)
				MOV_RN(next,addr);
			else
				free_reg(root);
			break;
		case N_OP:
			codegen(root->left);
			codegen(root->right);
			left_reg = root->left->res_reg;
			right_reg = root->right->res_reg;
			switch(root->optype){
				case O_ADD:
					ADD(left_reg,right_reg);
					break;
				case O_SUB:
					SUB(left_reg,right_reg);
					break;
				case O_MUL:
					MUL(left_reg,right_reg);
					break;
				case O_DIV:
					DIV(left_reg,right_reg);
					break;
				case O_ASN:
					addr=*(root->left->varname)-'a'+4096;
					ASN(addr,right_reg);
					free_reg(root->right);
					break;
				case O_LT:
					LT(left_reg,right_reg);
					break;
				case O_LTE:
					LE(left_reg,right_reg);
					break;
				case O_GT:
					GT(left_reg,right_reg);
					break;
				case O_GTE:
					GE(left_reg,right_reg);
					break;
				case O_EQ:
					EQ(left_reg,right_reg);
					break;
				case O_NE:
					NE(left_reg,right_reg);
					break;
			}
			if(root->optype!=O_ASN){
				use_reg(root,left_reg);
				use_reg(root->left,-1);
				free_reg(root->right);
			}
			break;

		case N_RD:
			codegen(root->left);
			put_read(root->left);
			free_reg(root->left);
			break;
		case N_WR:
			codegen(root->left);
			put_write(root->left);
			free_reg(root->left);
			break;

		case N_IF:
			GETIN_IF;
			codegen(root->left);
			cond = root->left->res_reg;
			temp = root->right;
			if(temp->right){
				LABEL_JZ("ELSE",IF_DOM,cond); 
				codegen(temp->left);
				LABEL_JMP("ENDIF",IF_DOM);
				LABEL_DEC("ELSE",IF_DOM);
				codegen(temp->right);
			}
			else{
				LABEL_JZ("ENDIF",IF_DOM,cond);
				codegen(temp->left);
			}
			GETOUT_IF;
			free_reg(root->left);
			break;

		case N_WHILE:
			GETIN_WHILE;
			codegen(root->left);
			cond = root->left->res_reg;
			temp = root->right;
			LABEL_JZ("ENDWHILE",WHILE_DOM,cond);
			codegen(temp);
			LABEL_JMP("WHILE",WHILE_DOM);
			GETOUT_WHILE;
			free_reg(root->left);
			break;
			
		case N_BRK:
			IN_LOOP{
				LABEL_JMP("ENDWHILE",WHILE_DOM);
			}
			break;
		case N_CNT:
			IN_LOOP{
				LABEL_JMP("WHILE",WHILE_DOM);
			}
			break;
	}

}


int main(int argc, char *argv[]){
	FILE *in_file;
	char* obfilename;
	void yyset_in ( FILE *_in_str);
	char* obfile(char* filename);

	in_file = fopen(argv[1],"r");
	obfilename = obfile(argv[1]);
	target_file = fopen(obfilename,"w");
	yyset_in(in_file);
	free_all_reg();
	reset_counters();
	yyparse();
	fclose(in_file);
	fclose(target_file);
	
	return 0;
}
