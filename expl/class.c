#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "node.h"
#include "class.h"

void reset_off();
void set_off();
int get_offset();
void update_off();
int getlabel();
ctr methods=0;;
ctr members=0;
ctr classes=0;
ClassTable *Ctable=NULL;;

static ctr class_index(){
	ctr i = classes++;
	return i;
}

static void set_index(ctr i){
	classes = i;
}

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

static void copy_members(Class child, Class par){
	field *fc,*fp,*f;

	for_each_field(fp,par->mlist){
		fc = (field*)malloc(sizeof(field));
		fc->t = fp->t;
		fc->c = fp->c;
		fc->name = strdup(fp->name);
		fc->offset = fp->offset;

		for(f=child->mlist;f->next!=NULL;f=f->next);
		f->next = fc;
		++members;
	}
}

static void copy_params(param *Mc, param *Mp){
	param *p,*temp,*q;

	for_each_param(q,Mp){
		p = (param*)malloc(sizeof(param));
		p->datatype = Mp->datatype;
		p->varname = strdup(Mp->varname);
		p->isptr = Mp->isptr;
		p->next = NULL;

		for(temp=Mc; temp->next!=NULL; temp=temp->next);
		temp->next = p;
	}
}

static void copy_ltable(symtable *child, symtable *par){
	entry p,q,temp;

	for_each_entry(q,par){
		p = (entry)malloc(sizeof(symtable));
		p->varname = strdup(q->varname);
		p->datatype = q->datatype;
		p->ctype = q->ctype;
		p->isptr = q->isptr;
		p->size = q->size;
		p->dim1 = q->dim2;
		p->dim2 = q->dim2;
		p->bind_addr = q->bind_addr;
		p->isGlobal = q->isGlobal;
		p->isdef = q->isdef;
		p->label = q->label;
		p->ltable = NULL;
		p->next = NULL;

		for(temp=child; temp->next!=NULL; temp=temp->next);
		temp->next = p;
	}
}

static void copy_methods(Class child, Class par){
	Method *p,*q,*temp;

	for_each_method(q,par->Mlist){
		p = (Method*)malloc(sizeof(Method));
		p->name = strdup(q->name);
		p->ret = q->ret;
		p->c = q->c;
		p->isdef = q->isdef;
		p->label = q->label;
		p->index = q->index;
		p->params = (param*)malloc(sizeof(param));
		copy_params(p->params,q->params);
		p->ltable = (entry)malloc(sizeof(symtable));
		copy_ltable(p->ltable,q->ltable);
		p->next = NULL;

		for(temp=child->Mlist; temp->next!=NULL; temp=temp->next);
		temp->next = p;
		++methods;
	}
}

Class makeC(const char *name, node *Members, node *Methods, Class par){
	Class c;

	if(Tlookup(name)!=NULL){
		printf("Type %s already defined\n",name);
		exit(1);
	}

	c = (Class)malloc(sizeof(ClassTable));
	c->name = strdup(name);
	c->par = par;
	c->mlist = (field*)malloc(sizeof(field));
	c->Mlist = (Method*)malloc(sizeof(Method));
	c->mlist->c = c;
	c->Mlist->c = c;

	if(par){
		members=0;
		methods=0;
		copy_members(c,par);
		copy_methods(c,par);
	}

	c->index = class_index();
	if(Members){
		set_off(members);
		get_fields(Members,c->mlist);
	}
	if(Methods){
		set_off(methods);
		get_methods(Methods,c->Mlist);
	}
	c->next = NULL;
	return c;
}

void get_methods(node *root, Method *head){
	if(!root)
		return;

	if(get_offset()>=8){
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
				printf("function %s already declared in class %s\n",
						name,M->c->name);
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
			M->c = head->c;
			M->isdef = 0;
			M->index = get_offset();
			update_off();
			M->label = getlabel();
			M->params = (param*)malloc(sizeof(param));
			param_args_list(root->right,M->params);
			M->ltable = (symtable*)malloc(sizeof(symtable));
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

void ctable_init(){
	Ctable = (Class)malloc(sizeof(ClassTable));
	set_index(0);
}

char* printClass(Class c){
	return c->name;
}

int isDescendant(Class curr, Class ancestor){
	Class c;

	c = curr;
	while(c && c!=ancestor)
		c = c->par;

	if(c)
		return 1;
	return 0;
}
