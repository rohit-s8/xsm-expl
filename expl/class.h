#ifndef CLASS_H_
#define CLASS_H_

#include "types.h"
#include "node.h"

typedef struct ClassMethod{
	char *name;
	type ret;
	param *plist;
	struct classMethod *next;
}Method;

typedef struct ClassTable{
	char *name;
	field *mlist;
	Method *Mlist;
	struct ClassTable *par;
	struct ClassTable *next;
}ClassTable;

typedef ClassTable* Class;
extern ClassTable *Ctable;

#define for_each_method(M,list)\
	for(M=list->next;M!=NULL;M=M->next)

#define for_each_class(c)\
	for(c=Ctable->next;c!=NULL;c=c->next)


void Cinstall(Class C);
Class makeC(const char *name, node *members, node *methods);
void get_methods(node *root, Method *head);
Class Clookup(const char *name);
Method* MLookup(Method *list, const char *name);
void ctable_init();
char* printClass(Class C);

#endif
