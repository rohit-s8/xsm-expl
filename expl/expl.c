#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "y.tab.h"
#include "node.h"

/** print instruction **/
#define printi(...)\
	fprintf(ob_file, __VA_ARGS__)

/** MOV A,B **/
#define MOV(A,B)\
	printi("MOV %s,%s\n",A,B)

/** MOV Rx,BP **/
#define MOV_RBP(x)\
	printi("MOV R%d,BP\n",x)

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

/** MOV [Rx],Ry **/
#define MOVA_AR(x,y)\
	printi("MOV [R%d],R%d\n",x,y)

/** MOV Rx,[Ry] **/
#define MOVA_RA(x,y)\
	printi("MOV R%d,[R%d]\n",x,y)

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

/** MOD Rx,Ry **/
#define MOD(x,y)\
	printi("MOD R%d,R%d\n",x,y)

/** MOV [ADDRESS=x],Ry **/
#define ASN(x,y)\
	MOVA_AR(x,y)

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

/** CALL N **/
#define CALL(N)\
	printi("CALL %d\n",N)

/** RET **/
#define RET printi("RET\n")

/** BRKP **/
#define BRKP printi("BRKP\n")

/** L_X[y]: **/
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
#define IF_INIT IF_DOM = ++if_ctr; LABEL_DEC("IF",IF_DOM)
/** IF label exit **/
#define IF_EXIT LABEL_DEC("ENDIF",IF_DOM)
/** JZ ELSE, JZ ENDIF, JMP ENDIF **/
#define JZ_ELSE(reg)\
	LABEL_JZ("ELSE",IF_DOM,reg)
#define JZ_ENDIF(reg)\
	LABEL_JZ("ENDIF",IF_DOM,reg)
#define JMP_ENDIF\
	LABEL_JMP("ENDIF",IF_DOM)

/** Set WHILE label domain and put declaration **/
#define WHILE_INIT\
	++in_while; WHILE_DOM = ++while_ctr;\
	LABEL_DEC("WHILE",WHILE_DOM)
/** WHILE label exit **/
#define WHILE_EXIT\
	LABEL_DEC("ENDWHILE",WHILE_DOM);\
	--in_while; --WHILE_DOM
/** JZ ENDWHILE, JMP WHILE, JMP ENDWHILE **/
#define JZ_ENDWHILE(reg)\
	LABEL_JZ("ENDWHILE",WHILE_DOM,reg)
#define JMP_ENDWHILE\
	LABEL_JMP("ENDWHILE",WHILE_DOM)
#define JMP_WHILE\
	LABEL_JMP("WHILE",WHILE_DOM)

/** Test if in loop **/
#define islooping if(in_while)

/** test if identifier is being assigned **/
#define PAR root->par
#define IS_ASSIGNED\
       	PAR->nodetype==N_OP && PAR->optype==O_ASN && PAR->left==root
#define IS_ARRAY\
	PAR->nodetype==N_ARR && PAR->left==root
#define IS_REF\
	PAR->optype==O_ADR
#define IS_FREED	PAR->nodetype==N_FREE

reg_ind r;
ctr if_ctr;
ctr while_ctr;
ctr curr_while;
ctr curr_if;
ctr in_while;
FILE *ob_file;
ctr s1[100];		//number of registers in use stack
ctr s2[100];		//number of registers saved stack
ctr t1=-1;			//for s1
ctr t2=-1;			//for s2

static void reset_counters(){
	if_ctr=0;
	while_ctr=0;
	in_while=0;
}

static void free_all_reg(){
	r=0;
}

static reg_ind get_reg(){
	return r++;
}

static void free_reg(){
	--r;
}

static ctr num_reg(){
	return r;
}

static void save_regstate(){
	++t1;
	s1[t1] = r;
}

static void restore_regstate(){
	r = s1[t1];
	--t1;
}

static void save_registers(ctr num){
	++t2;
	s2[t2] = num;
	int i;
	for(i=0; i<num; i++){
		PUSH(i);
	}
}

static void restore_registers(){
	int i;
	for(i=s2[t2]-1; i>=0; i--){
		POP(i);
	}
	--t2;
}

static reg_ind reglink(node *n){
	n->res_reg = get_reg();
	return n->res_reg;
}

static void reg_unlink(node *n){
	n->res_reg=-1;
	free_reg();
}

static void change_reglink(node *n, node *m){
	n->res_reg = m->res_reg;
	m->res_reg = -1;
}

static void library_call(const char* fcode, reg_ind arg1, reg_ind arg2,
							reg_ind ret){
	reg_ind temp = get_reg();
	MOV_RS(temp,fcode);
	PUSH(temp);
	PUSH(arg1);
	PUSH(arg2);
	PUSH(temp);
	PUSH(ret);
	save_regstate();
	free_all_reg();
	CALL(0);
	restore_regstate();
	POP(ret);
	int i;
	for(i=0; i<4; i++)
		POP(temp);
	free_reg();
}

void put_header(){
	printi("%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
		0,2056,0,0,0,0,0,0);
	MOV("BP","4096");
	printi("MOV SP,%d\n",last_addr(gtable));
	reg_ind temp = get_reg();
	PUSH(temp);
	free_reg();
	LABEL_CALL("main",0);
	temp = get_reg();
	POP(temp);
	free_reg();
	LABEL_JMP("EXIT",0);
}

static void put_write(node *n){
	save_registers(num_reg());
	reg_ind syscallno = get_reg();
	MOV_RN(syscallno,-2);
	library_call("Write",syscallno,n->res_reg,n->res_reg);
	free_reg();
	restore_registers();
}

static void put_read(node *n){
	save_registers(num_reg());
	reg_ind syscallno = get_reg();
	MOV_RN(syscallno,-1);
	library_call("Read",syscallno,n->res_reg,n->res_reg);
	free_reg();
	restore_registers();
}

static void put_init(){
	save_registers(num_reg());
	reg_ind temp = get_reg();
	library_call("Heapset",temp,temp,temp);
	free_reg();
	restore_registers();
}

static void put_alloc(reg_ind ret){
	reg_ind size = get_reg();
	MOV_RN(size,8);
	library_call("Alloc",size,size,ret);
	free_reg();
}

static void put_free(node *id){
	save_registers(num_reg());
	library_call("Free",id->res_reg,id->res_reg,id->res_reg);
	restore_registers();
}

void terminate(){
	LABEL_DEC("EXIT",0);
	reg_ind temp = get_reg();
	MOV_RS(temp,"Exit");
	PUSH(temp);
	MOV_RN(temp,0);
	int i;
	for(i=0; i<4; i++){
		PUSH(temp);
	}
	CALL(0);
}

static reg_ind get_array_addr(node *array){
	reg_ind base,offset,next;
	node *idnode,*dimtree;

	idnode = array->left;
	dimtree = array->right;
	base = idnode->res_reg;		//base address
	if(dimtree->datatype==T_INTEGER)
		offset = dimtree->res_reg;
	else{
		offset = dimtree->left->res_reg;
		next = reglink(dimtree);
		MOV_RN(next,idnode->ptr->dim2);
		MUL(offset,next);
		ADD(offset,dimtree->right->res_reg);
		reg_unlink(dimtree);
		change_reglink(dimtree,dimtree->left);
		reg_unlink(dimtree->right);
	}

	ADD(base,offset);
	change_reglink(array,idnode);
	reg_unlink(dimtree);
	return base;
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
			next = reglink(root);
			if(root->datatype == T_INTEGER){
				MOV_RN(next,root->val->num);
			}
			else if(root->datatype == T_STRING)
				MOV_RS(next,root->val->str);
			break;

		case N_PTR:
		case N_ID:
			if(!root->left){
				next = reglink(root);
				addr = get_id_addr(root);

				if(root->ptr->isGlobal){
					MOV_RN(next,addr);
				}
				else{
					MOV_RBP(next);
					if(addr!=0){
						reg_ind offset = get_reg();
						MOV_RN(offset,addr);
						ADD(next,offset);
						free_reg();	//free offset
					}
					//next register now has address of id
				}
				if(root->nodetype==N_PTR || (root->isptr && IS_ARRAY))
					MOVA_RA(next,next);		//address of variable pointed to
				if(!(PAR->nodetype==N_RD||IS_ASSIGNED||IS_ARRAY||IS_REF))
					MOVA_RA(next,next);		//value of id
			}
			else{
				codegen(root->left);
				type t = root->left->datatype;
				field *f;
				f = Flookup(t->flist,root->varname);
				next = get_reg();
				MOV_RN(next,f->offset);
				ADD(root->left->res_reg,next);
				free_reg();
				if(!(PAR->nodetype==N_RD||IS_ASSIGNED||IS_ARRAY||IS_REF))
					MOVA_RA(root->left->res_reg,root->left->res_reg);
				change_reglink(root,root->left);
			}
			break;
		case N_ARR:
			codegen(root->left);
			codegen(root->right);
			reg_ind addr_reg = get_array_addr(root);
			if(!(PAR->nodetype==N_RD || IS_ASSIGNED || IS_REF))
				MOVA_RA(addr_reg,addr_reg);
			break;
		case N_OP:
			codegen(root->left);
			codegen(root->right);
			if(root->optype!=O_ADR){
				left_reg = root->left->res_reg;
				right_reg = root->right->res_reg;
			}
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
				case O_MOD:
					MOD(left_reg,right_reg);
					break;
				case O_ASN:
					ASN(left_reg,right_reg);
					reg_unlink(root->right);
					reg_unlink(root->left);
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
				case O_ADR:
					change_reglink(root,root->right);
					break;
			}
			if(root->optype!=O_ASN && root->optype!=O_ADR){
				change_reglink(root,root->left);
				reg_unlink(root->right);
			}
			break;

		case N_RD:
			codegen(root->left);
			put_read(root->left);
			reg_unlink(root->left);
			break;
		case N_WR:
			codegen(root->left);
			put_write(root->left);
			reg_unlink(root->left);
			break;

		case N_IF:
			IF_INIT;
			codegen(root->left);
			cond = root->left->res_reg;
			temp = root->right;
			if(temp->right){
				JZ_ELSE(cond); 
				codegen(temp->left);
				JMP_ENDIF;
				LABEL_DEC("ELSE",IF_DOM);
				codegen(temp->right);
			}
			else{
				JZ_ENDIF(cond);
				codegen(temp->left);
			}
			IF_EXIT;
			reg_unlink(root->left);
			break;

		case N_WHILE:
			WHILE_INIT;
			codegen(root->left);
			cond = root->left->res_reg;
			temp = root->right;
			JZ_ENDWHILE(cond);
			codegen(temp);
			JMP_WHILE;
			WHILE_EXIT;
			reg_unlink(root->left);
			break;
			
		case N_BRK:
			islooping{
				JMP_ENDWHILE;
			}
			break;
		case N_CNT:
			islooping{
				JMP_WHILE;
			}
			break;
		case N_RET:
			codegen(root->right);
			reg_ind res = root->right->res_reg;
			next = get_reg();
			reg_ind temp = get_reg();
			MOV_RBP(next);
			MOV_RN(temp,-2);
			ADD(next,temp);
			MOVA_AR(next,res);
			free_reg();
			free_reg();
			MOV("SP","BP");
			printi("POP BP\n");
			RET;
			reg_unlink(root->right);
			break;
		case N_ARG:
			codegen(root->left);
			PUSH(root->left->res_reg);
			reg_unlink(root->left);
			break;
		case N_FND:
			LABEL_DEC(root->varname,0);
			printi("PUSH BP\n");
			MOV("BP","SP");
			printi("MOV SP,%d\n",last_addr(root->ptr->ltable));
			printi("ADD SP,BP\n");
			codegen(root->left);
			codegen(root->right);
			break;
		case N_FNC:
			save_registers(num_reg());	//save registers in use
			next = reglink(root);		//return value
			codegen(root->right);		//push arguments
			PUSH(next);
			save_regstate();			//saving number of registers in use
			free_all_reg();
			LABEL_CALL(root->varname,0);	//function call
			restore_regstate();			//restore register state
			POP(next);					//get return value
			int num_param=0;
			param *p;
			for_each_param(p,root->ptr->params){
				num_param++;
			}
			next = get_reg();
			while(num_param--){
				POP(next);				//pop arguments
			}
			free_reg();
			restore_registers();		//restore registers
			break;
		case N_BRKP:
			BRKP;
			break;

		case N_INIT:
			put_init();
			break;
		case N_ALOC:
			save_registers(num_reg());
			next = reglink(root);
			put_alloc(next);
			restore_registers();
			break;
		case N_FREE:
			codegen(root->left);
			put_free(root->left);
			reg_unlink(root->left);
			codegen(root->right);
			break;
		case N_NULL:
			next = reglink(root);
			MOV_RN(next,0);
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
	ob_file = fopen(obfilename,"w");
	yyset_in(in_file);
	free_all_reg();
	reset_counters();
	ttable_init();
	yyparse();
	fclose(in_file);
	fclose(ob_file);

	char cmd[100];
	sprintf(cmd,"./linker %s",obfilename);

	return system(cmd);
}
