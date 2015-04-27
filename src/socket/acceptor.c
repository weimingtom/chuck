#include <assert.h>
#include "socket/acceptor.h"
#include "engine.h"

static int32_t imp_engine_add(engine *e,handle *h,generic_callback callback)
{
	assert(e && h && callback);
	int32_t ret = event_add(e,h,EVENT_READ);
	if(ret == 0)
		((acceptor*)h)->callback = (void (*)(int32_t fd,sockaddr_*))callback;
	return ret;
}


static int _accept(handle *h,sockaddr_ *addr){
	socklen_t len;
	int32_t fd; 
	while((fd = accept(h->fd,(struct sockaddr*)addr,&len)) < 0){
#ifdef EPROTO
		if(errno == EPROTO || errno == ECONNABORTED)
#else
		if(errno == ECONNABORTED)
#endif
			continue;
		else
			return -errno;
	}
/*	int flags;
	int dummy = 0;
	if ((flags = fcntl(fd, F_GETFL, dummy)) < 0){
		printf("fcntl get error\n");
    		close(fd);
    		return -1;
	}
	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) <0){
    		printf("fcntl set  FD_CLOEXEC error\n");
    		close(fd);
    		return -1;	
	}
*/	
	return fd;
}

static void process_accept(handle *h,int32_t events){
    int fd;
    sockaddr_ addr;
    for(;;){
    	fd = _accept(h,&addr);
    	if(fd < 0)
    	   break;
    	else{
		   ((acceptor*)h)->callback(fd,&addr);
    	}      
    }
}

handle *acceptor_new(int32_t fd){
	handle *h = calloc(1,sizeof(*h));
	h->fd = fd;
	h->on_events = process_accept;
	h->imp_engine_add = imp_engine_add;
	return h;
}

void    acceptor_del(handle *h){
	close(h->fd);
	free(h);
} 