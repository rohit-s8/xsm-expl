#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "node.h"
#include "class.h"

ClassTable *Ctable;
void reset_off();

void Cinstall(Class C){
	Class c,p;
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
	get_methods(methods,c->Mlist);
	c->par = NULL;
	c->next = NULL;
	return c;
}

void get_methods(node *root, Method *head){
	if(!root)
		return;

	char *name;
	type t;
	Method *M,*temp;
	switch(root->nodetype){
		case N_CON:
			get_methods(root->left);
			get_methods(root->right);
			break;
		case N_FNC:
			name = root->varname;
			if(Mlookup(name,head)!=NULL){
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
			M->plist = (param*)malloc(sizeof(param));
			param_args_list(root->right,M->params);
			M->next = NULL;

			for(temp=head; temp->next!=NULL; temp=temp->next);
			temp->next = M;
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

void Ctable_init(){
	Ctable = (Class)malloc(sizeof(ClassTable));
}

char* printClass(Class c){
	return c->name;
}
