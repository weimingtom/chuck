#include "engine/engine.h"


int32_t engine_add(engine *e,handle *h,generic_callback callback){
	int32_t ret = h->imp_engine_add(e,h,callback);
	if(ret == 0) h->e = e;
	return ret;
}

int32_t engine_remove(handle *h){
	if(!h->e) return -ENOASSENG;
	int32_t ret = event_remove(h->e,h);
	if(ret == 0) h->e = NULL;
	return ret;	
}

#ifdef _LINUX

#include "engine_imp_epoll.h"

#elif _BSD

#include "engine_imp_kqueue.h"

#else

#error "un support platform!"		

#endif