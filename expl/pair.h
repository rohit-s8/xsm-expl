#ifndef _PAIR_H
#define _PAIR_H
#include "types.h"

typedef struct pair{
	char str[100];
	ctr  num;
	struct pair *next;
}pair;

typedef pair* pair_list;
extern pair_list head;

#define for_each_pair(p)\
	for(p=head; p!=NULL; p=p->next)

pair* make_pair(const char* str,int buf_len, ctr num);
pair_list add_pair(pair* _new);

#endif
