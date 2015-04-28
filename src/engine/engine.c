#include "engine/engine.h"


int32_t engine_add(engine *e,handle *h,generic_callback callback){
	return h->imp_engine_add(e,h,callback);
}

#ifdef _LINUX

#include "engine_imp_epoll.h"

#elif _BSD

#include "engine_imp_kqueue.h"

#else

#error "un support platform!"		

#endif