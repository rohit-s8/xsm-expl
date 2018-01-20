#include <stdlib.h>
#include <string.h>
#include "pair.h"
#include "types.h"

pair_list head;

pair* make_pair(const char* str,int buf_len, ctr num){
	pair* p = (pair*)malloc(sizeof(pair));
	p->num = num;
	int i;
	for(i=0; i<buf_len; i++){
		p->str[i] = *(str+i);
	}
	p->next=NULL;
	return p;
}

pair_list add_pair(pair* _new){
	_new->next = head;
	head = _new;
	return head;
}
