#ifndef TYPES_H_
#define TYPES_H_

#include "structs.h"

#define for_each_type(t)\
	for(t=tt->next;t!=NULL;t=t->next)

#define for_each_field(f,flist)\
	for(f=flist->next;f!=NULL;f=f->next)

extern typetable *tt;
type Tlookup(const char *name);
void Tinstall(type newT);
type makeT(const char *name);
void get_fields(struct node* ftree, field *head);
unsigned int Tsize(type t);
field* Flookup(field *list, const char *name);
int basic_type(type t);
void ttable_update(struct node* ttree);
void ttable_init();
char* printop(Operator op);
char* printtype(type t);

#define T_INTEGER	Tlookup("int")
#define T_STRING	Tlookup("string")
#define T_BOOL		Tlookup("bool")
#define T_VOID		Tlookup("void")

#endif
