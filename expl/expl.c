#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "types.h"
#include "class.h"
#include "y.tab.h"

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

/** INR S **/
#define INR(S)\
	printi("INR %s\n",S)

/** INR Rx **/
#define INR_R(x)\
	printi("INR R%d\n",x)

/** DCR S **/
#define DCR(S)\
	printi("DCR %s\n",S)

/** DCR Rx **/
#define DCR_R(x)\
	printi("DCR R%d\n",x)

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
#define IF_EXIT LABEL_DEC("ENDIF",IF_DOM); --IF_DOM
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
#define OBJCOPY		root->ctype && PAR->optype==O_ASN && root==PAR->right

FILE *ob_file;
int r;
int s2[100];		//number of registers saved stack
int t2=-1;			//for s2
ctr if_ctr;
ctr while_ctr;
ctr curr_while;
ctr curr_if;
ctr in_while;
ctr label;
//ctr s1[100];		//number of registers in use stack
//ctr t1=-1;			//for s1

static void reset_counters(){
	if_ctr=0;
	while_ctr=0;
	in_while=0;
	label = 0;
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

static int num_reg(){
	return r;
}

/*
static void save_regstate(){
	++t1;
	s1[t1] = r;
}

static void restore_regstate(){
	r = s1[t1];
	--t1;
}
*/

static void save_registers(int num){
	++t2;
	s2[t2] = num;
	int i;
	for(i=0; i<num; i++)
		PUSH(i);
}

static void restore_registers(){
	int i;
	for(i=s2[t2]-1; i>=0; i--)
		POP(i);
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
	CALL(0);
	POP(ret);
	int i;
	for(i=0; i<4; i++)
		POP(temp);
	free_reg();
}

ctr getlabel(){
	++label;
	return label;
}

char *putlabel(ctr label){
	char *l = (char*)calloc(10,sizeof(char));
	sprintf(l,"L_L%d",label);
	return l;
}

void put_header(){
	ctr methods;
	Class c;
	Method *M;
	reg_ind temp;

	temp = get_reg();
	printi("%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
		0,2056,0,0,0,0,0,0);
	MOV("BP","4096");
	for_each_class(c){
		methods=0;
		for_each_method(M,c->Mlist){
			MOV("[BP]",putlabel(M->label));
			INR("BP");
			methods++;
		}
		if(8-methods>0){
			MOV_RN(temp,8-methods);
			printi("ADD BP,R%d\n",temp);
		}
	}		
	printi("MOV BP,%d\n",last_addr(gtable)+1);
	printi("MOV SP,%d\n",last_addr(gtable));
	PUSH(temp);
	entry f = lookup("main",gtable);
	LABEL_CALL("L",f->label);
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
	free_reg();
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
	reg_ind classptr,methodptr;
	node *temp;
	int addr;
	ctr flabel;
	symtable *table;
	Class c;
	type t;

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

		case N_SELF:
				next = reglink(root);
				reg_ind offset = get_reg();
				MOV_RBP(next);
				MOV_RN(offset,4);
				SUB(next,offset);
				MOVA_RA(next,next);
				free_reg();
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
				if(!(PAR->nodetype==N_RD||IS_ASSIGNED||IS_ARRAY||IS_REF
							||PAR->nodetype==N_FNC||OBJCOPY))
					MOVA_RA(next,next);		//value of id
			}
			else{
				codegen(root->left);
				type t = root->left->datatype;
				Class c = root->left->ctype;
				field *f;
				if(!c)
					f = Flookup(t->flist,root->varname);
				else
					f = Flookup(c->mlist,root->varname);
				next = get_reg();
				MOV_RN(next,f->offset);
				ADD(root->left->res_reg,next);
				free_reg();
				if(!(PAR->nodetype==N_RD||IS_ASSIGNED||IS_ARRAY||IS_REF
							||PAR->nodetype==N_FNC||OBJCOPY))
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
			if(root->optype!=O_ASN){
				codegen(root->left);
				codegen(root->right);
			}
			if(root->optype!=O_ADR && root->optype!=O_ASN){
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
					codegen(root->right);
					codegen(root->left);
					left_reg = root->left->res_reg;
					right_reg = root->right->res_reg;
					c = root->right->ctype;
					if(!c)
						ASN(left_reg,right_reg);
					else{
						reg_ind temp = get_reg();
						MOVA_RA(temp,right_reg);
						ASN(left_reg,temp);
						INR_R(left_reg);
						INR_R(right_reg);
						MOVA_RA(temp,right_reg);
						ASN(left_reg,temp);
						free_reg();
					}
					if(PAR->nodetype!=N_NEW){
						reg_unlink(root->left);
						reg_unlink(root->right);
					}
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
				reg_unlink(root->left);
				codegen(temp->left);
				JMP_ENDIF;
				LABEL_DEC("ELSE",IF_DOM);
				codegen(temp->right);
			}
			else{
				JZ_ENDIF(cond);
				reg_unlink(root->left);
				codegen(temp->left);
			}
			IF_EXIT;
			break;

		case N_WHILE:
			WHILE_INIT;
			codegen(root->left);
			cond = root->left->res_reg;
			temp = root->right;
			JZ_ENDWHILE(cond);
			reg_unlink(root->left);
			codegen(temp);
			JMP_WHILE;
			WHILE_EXIT;
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
			reg_ind r = get_reg();
			MOV_RBP(next);
			MOV_RN(r,-2);
			ADD(next,r);
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
			c = root->ctype;
			if(!c){
				flabel = root->ptr->label;
				table = root->ptr->ltable;
			}
			else{
				Method *M = Mlookup(root->varname,c->Mlist);
				flabel = M->label;
				table = M->ltable;
			}
			LABEL_DEC("L",flabel);
			printi("PUSH BP\n");
			MOV("BP","SP");
			if(last_addr(table)>0){
				reg_ind temp = get_reg();
				MOV_RN(temp,last_addr(table));
				printi("ADD SP,R%d\n",temp);
				free_reg();
			}
			free_all_reg();
			codegen(root->left);
			codegen(root->right);
			break;
		case N_FNC:
			save_registers(num_reg());	//save registers in use
			codegen(root->right);		//push arguments
			param *params;

			if(root->left){
				codegen(root->left);
				c = root->left->ctype;
				change_reglink(root,root->left);
				next = root->res_reg;
				if(root->left->nodetype!=N_SELF){
					classptr = get_reg();
					MOV_RN(classptr,1);
					ADD(classptr,next);//classptr points to ptr to vft
					MOVA_RA(classptr,classptr);//classptr points to vft
					MOVA_RA(next,next);	//next contains object address
					PUSH(next);			//push object address
					PUSH(classptr);		//push vft pointer
					PUSH(next);			// return value
					methodptr = classptr;
				}
				else{
					methodptr = get_reg();
					reg_ind offset = get_reg();
					MOV_RBP(methodptr);
					MOV_RN(offset,3);
					SUB(methodptr,offset);
					free_reg();
					MOVA_RA(methodptr,methodptr);
					PUSH(next);
					PUSH(methodptr);
					PUSH(next);
				}
				Method *M = Mlookup(root->varname,c->Mlist);
				params = M->params;
				if(M->index>0){
					reg_ind findex = get_reg();
					MOV_RN(findex,M->index);
					ADD(methodptr,findex);	//methodptr points to function label
					free_reg();		//free findex
				}
				MOVA_RA(methodptr,methodptr);//methodptr has function label
				printi("CALL R%d\n",methodptr);
				free_reg();		//free methodptr register
			}
			else{
				flabel = root->ptr->label;
				params = root->ptr->params;
				next = reglink(root);		//return value
				PUSH(next);					//return value
				LABEL_CALL("L",flabel);		//function call
			}
			POP(next);					//get return value
			next = get_reg();
			if(c){
				POP(next);				//POP object address
				POP(next);				//POP vft pointer
			}
			int num_param=0;
			param *p;
			for_each_param(p,params)
				num_param++;
			while(num_param--)
				POP(next);				//pop arguments
			free_reg();
			restore_registers();
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
		case N_NEW:
			c = root->ctype;
			codegen(root->right);
			next = root->left->res_reg;
			INR_R(next);
			reg_ind classptr = get_reg();
			MOV_RN(classptr,4096+8*c->index);
			ASN(next,classptr);
			free_reg();
			reg_unlink(root->left);
			reg_unlink(root->right->right);
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
	ctable_init();
	yyparse();
	fclose(in_file);
	fclose(ob_file);

	extern errors;
	if(!errors){
		char cmd[100];
		sprintf(cmd,"./linker %s",obfilename);
		return system(cmd);
	}
	return 1;
}
