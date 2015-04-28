#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <assert.h>

extern int32_t pipe2(int pipefd[2], int flags);

typedef struct{
	int32_t epfd;
	struct  epoll_event* events;
	int32_t maxevents;
	int32_t notifyfds[2];//0 for read,1 for write
}epoll_;

int32_t event_add(engine *e,handle *h,int32_t events){
	assert((events & EPOLLET) == 0);
	struct epoll_event ev = {0};
	epoll_ *ep = (epoll_*)e;
	ev.data.ptr = h;
	ev.events = events;
	errno = 0;
	if(0 != epoll_ctl(ep->epfd,EPOLL_CTL_ADD,h->fd,&ev)) 
		return -errno;
	h->events = events;
	h->e = e;
	return 0;
}


int32_t event_remove(handle *h){
	epoll_ *ep = (epoll_*)h->e;
	struct epoll_event ev = {0};
	errno = 0;
	if(0 != epoll_ctl(ep->epfd,EPOLL_CTL_DEL,h->fd,&ev)) 
		return -errno; 
	h->events = 0;
	h->e = NULL;
	return 0;	
}

int32_t event_mod(handle *h,int32_t events){
	assert((events & EPOLLET) == 0);	
	struct epoll_event ev = {0};
	epoll_ *ep = (epoll_*)h->e;
	ev.data.ptr = h;
	ev.events = events;
	errno = 0;
	if(0 != epoll_ctl(ep->epfd,EPOLL_CTL_MOD,h->fd,&ev)) 
		return -errno; 
	h->events = events;		
	return 0;	
}


int32_t event_enable(handle *h,int32_t events){
	return event_mod(h,h->events | events);
}

int32_t event_disable(handle *h,int32_t events){
	return event_mod(h,h->events & (~events));
}



engine* engine_new(){
	int32_t epfd = epoll_create1(EPOLL_CLOEXEC);
	if(epfd < 0) return NULL;
	int32_t tmp[2];
	if(pipe2(tmp,O_NONBLOCK|O_CLOEXEC) != 0){
		close(epfd);
		return NULL;
	}		
	epoll_ *ep = calloc(1,sizeof(*ep));
	ep->epfd = epfd;
	ep->maxevents = 64;
	ep->events = calloc(1,(sizeof(*ep->events)*ep->maxevents));
	ep->notifyfds[0] = tmp[0];
	ep->notifyfds[1] = tmp[1];

	struct epoll_event ev = {0};
	ev.data.fd = ep->notifyfds[0];
	ev.events = EPOLLIN;
	if(0 != epoll_ctl(ep->epfd,EPOLL_CTL_ADD,ev.data.fd,&ev)){
		close(epfd);
		close(tmp[0]);
		close(tmp[1]);
		free(ep->events);
		free(ep);
		return NULL;
	}	
	return (engine*)ep;
}

void engine_del(engine *e){
	epoll_ *ep = (epoll_*)e;
	close(ep->epfd);
	close(ep->notifyfds[0]);
	close(ep->notifyfds[1]);
	free(ep->events);
	free(ep);
}


int32_t engine_run(engine *e){
	epoll_ *ep = (epoll_*)e;
	for(;;){
		errno = 0;
		int32_t i;
		handle *h;
		int32_t nfds = TEMP_FAILURE_RETRY(epoll_wait(ep->epfd,ep->events,ep->maxevents,-1));
		if(nfds > 0){
			for(i=0; i < nfds ; ++i)
			{
				if(ep->events[i].data.fd == ep->notifyfds[0]){
					int32_t _;
					while(TEMP_FAILURE_RETRY(read(ep->notifyfds[0],&_,sizeof(_))) > 0);
					return 0;	
				}else{
					h = (handle*)ep->events[i].data.ptr;
					h->on_events(h,ep->events[i].events);;
				}
			}
			if(nfds == ep->maxevents){
				free(ep->events);
				ep->maxevents <<= 2;
				ep->events = calloc(1,sizeof(*ep->events)*ep->maxevents);
			}				
		}else if(nfds < 0){
			return -errno;
		}	
	}
	return 0;
}


void engine_stop(engine *e){
	epoll_ *ep = (epoll_*)e;
	int32_t _;
	TEMP_FAILURE_RETRY(write(ep->notifyfds[1],&_,sizeof(_)));
}

/*kn_timer_t kn_reg_timer(engine_t e,uint64_t timeout,int32_t(*cb)(uint32_t,void*),void *ud){
	kn_epoll *ep = (kn_epoll*)e;
	if(!ep->timerfd){
		ep->timerfd = kn_new_timerfd(1);
		((handle_t)ep->timerfd)->ud = wheelmgr_new();
		kn_event_add(ep,ep->timerfd,EPOLLIN | EPOLLOUT);			
	}
	return wheelmgr_register(((handle_t)ep->timerfd)->ud,timeout,cb,ud);
}*/
