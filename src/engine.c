#include "engine.h"

#ifdef _LINUX

#include "engine_imp_epoll.c"

#elif _BSD

#include "engine_imp_kqueue.c"

#else

#error "un support platform!"		

#endif