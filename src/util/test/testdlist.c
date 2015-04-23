#include <stdio.h>
#include "util/dlist.h"


typedef struct{
	dlistnode base;
	int n;
}nnode;


#define PUSH_BACK(DLIST,VAL)\
    {\
     nnode *node = calloc(1,sizeof(*node));\
     node->n = VAL;\
     dlist_pushback(DLIST,(dlistnode*)node);}


#define PUSH_FRONT(DLIST,VAL)\
    {\
     nnode *node = calloc(1,sizeof(*node));\
     node->n = VAL;\
     dlist_pushfront(DLIST,(listnode*)node);}


#define POP(DLIST)\
   ({  int __result = 0;\
       nnode *node;\
       do{\
       	node = (nnode*)dlist_pop(DLIST);\
       	if(node){\
       	 __result = node->n;\
       	 free(node);\
       }while(0);\
       __result;})
#endif


int main(){
	dlist l;
	dlist_init(&l);
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