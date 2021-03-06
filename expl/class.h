#ifndef CLASS_H_
#define CLASS_H_

#include "structs.h"

#define for_each_method(M,list)\
	for(M=list->next;M!=NULL;M=M->next)

#define for_each_class(c)\
	for(c=Ctable->next;c!=NULL;c=c->next)

extern ClassTable *Ctable;
void Cinstall(Class C);
Class makeC(const char *name, node *members, node *methods, Class par);
void get_methods(node *root, Method *head);
Class Clookup(const char *name);
Method* MLookup(const char *name, Method *list);
void ctable_init();
void copy_lists(Class child, Class par);
char* printClass(Class C);
int isDescendant(Class curr, Class ancestor);

#endif
