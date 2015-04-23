#include <stdio.h>
#include "util/list.h"


typedef struct{
	listnode base;
	int n;
}nnode;


#define PUSH_BACK(LIST,VAL)\
    {\
     nnode *node = calloc(1,sizeof(*node));\
     node->n = VAL;\
     list_pushback(LIST,(listnode*)node);}


#define PUSH_FRONT(LIST,VAL)\
    {\
     nnode *node = calloc(1,sizeof(*node));\
     node->n = VAL;\
     list_pushfront(LIST,(listnode*)node);}


#define POP(LIST)\
   ({  int __result = 0;\
       nnode *node;\
       do{\
       	node = (nnode*)list_pop(LIST);\
       	if(node){\
       	 __result = node->n;\
       	 free(node);\
       }while(0);\
       __result;})
#endif


int main(){
	list l;
	list_init(&l);
	int i = 0;
	for(;i < 10;++i){
		PUSH_BACK(l,i);
	}

	for(;i >= 0;i--){
		print("%d\n",POP(l));
	}

	for(i = 0;i < 10;++i){
		PUSH_BACK(l,i);
	}

	for(;i >= 0;i--){
		print("%d\n",POP(l));
	}


	return 0;
}