#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "node.h"
#include "class.h"

ClassTable *Ctable;
void reset_off();
int Mcount;

void Cinstall(Class C){
	Class c,p;
	p = Ctable;
	for_each_class(c){
		if(strcmp(c->name,C->name)==0){
			printf("Class %s already defined\n",C->name);
			exit(1);
		}
		p = c;
	}
	p->next = C;
}

Class makeC(const char *name, node *members, node *methods){
	Class c;
	void reset_off();

	if(Tlookup(name)!=NULL){
		printf("Type %s already defined\n",name);
		exit(1);
	}

	c = (Class)malloc(sizeof(ClassTable));
	c->name = name;
	c->mlist = (field*)malloc(sizeof(field));
	reset_off();
	get_fields(members,c->mlist);
	c->Mlist = (Method*)malloc(sizeof(Method));
	Mcount=0;
	get_methods(methods,c->Mlist);
	c->par = NULL;
	c->next = NULL;
	return c;
}

void get_methods(node *root, Method *head){
	if(!root)
		return;

	if(Mcount>=8){
		printf("Too many functions in class. Max 8 allowed\n");
		exit(1);
	}

	char *name;
	type t;
	Method *M,*temp;
	Method *Mlookup(const char*, Method*);
	switch(root->nodetype){
		case N_CON:
			get_methods(root->left,head);
			get_methods(root->right,head);
			break;
		case N_FNC:
			name = root->varname;
			M = Mlookup(name,head);
			if(M!=NULL){
				printf("function %s already declared in class\n",name);
				exit(1);
			}
			t = Tlookup(root->left->varname);
			if(!t){
				printf("type %s not defined\n",root->left->varname);
				exit(1);
			}

			M = (Method*)malloc(sizeof(Method));
			M->name = name;
			M->ret = t;
			M->params = (param*)malloc(sizeof(param));
			param_args_list(root->right,M->params);
			M->ltable = (symtable*)malloc(sizeof(symtable));
			M->next = NULL;

			for(temp=head; temp->next!=NULL; temp=temp->next);
			temp->next = M;
			++Mcount;
			break;
	}
}

Class Clookup(const char *name){
	Class c;
	for_each_class(c){
		if(strcmp(c->name,name)==0)
			break;
	}
	return c;
}

Method* Mlookup(const char *name, Method *list){
	Method *M;
	for_each_method(M,list){
		if(strcmp(M->name,name)==0)
			break;
	}
	return M;
}

void ctable_init(){
	Ctable = (Class)malloc(sizeof(ClassTable));
}

char* printClass(Class c){
	return c->name;
}
