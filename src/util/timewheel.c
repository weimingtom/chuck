#include <assert.h>
#include <stdio.h>
#include "timewheel.h"
#include "time.h"
#include "dlist.h"


enum{
	wheel_sec = 0,  //1000 ms
	wheel_hour, //3599 s
	wheel_day,  //23   h
};


typedef struct {
	uint8_t  type;
	uint16_t cur;
	dlist    items[0]; 
}wheel;

#define wheel_size(T) (T==wheel_sec?1000:T==wheel_hour?3599:T==wheel_day?23:-1)

static wheel* wheel_new(uint8_t type){
	wheel *w;
	if(type == wheel_sec){
		w = calloc(1,sizeof(*w)*1000*sizeof(dlist));
	}else if(type == wheel_hour){
		w = calloc(1,sizeof(*w)*3599*sizeof(dlist));
	}else if(type == wheel_day){
		w = calloc(1,sizeof(*w)*23*sizeof(dlist));	
	}else 
		return NULL;
	w->type = type;
	w->cur = 0;	
	uint16_t size = (uint16_t)wheel_size(type);
	uint16_t i = 0;
	for(; i < size; ++i){
		dlist_init(&w->items[i]);
	}
	return w;	
}

typedef struct timer{
	dlistnode     node;
	uint32_t      timeout;
	uint64_t      expire;
	int32_t       (*callback)(uint64_t,void*); 
	void         *ud;
}timer;

typedef struct wheelmgr{
	wheel 		*wheels[wheel_day+1];
	uint64_t     lasttime;
}wheelmgr;


static inline void set(wheelmgr *m,timer *t,uint64_t tick){
	uint64_t remain = t->expire > tick ? t->expire - tick:0;
	wheel *w;
	do{
		w = m->wheels[wheel_sec];
		uint64_t s = wheel_size(wheel_sec) - w->cur;
		if(s > remain) break;
		remain -= s;
		remain /= 1000;
		w = m->wheels[wheel_hour];
		s = wheel_size(wheel_hour) - w->cur;
		if(s > remain) break;
		remain -= s;
		remain /= 3600;
		w = m->wheels[wheel_day];
		s = wheel_size(wheel_day) - w->cur;
		if(s > remain) break;
		t = NULL;		
	}while(0);
	assert(t != NULL);
	uint16_t i = (w->cur + remain)%(wheel_size(w->type));
	//printf("%s,%d,%d,%d\n",
	//	   w->type == wheel_sec ? "wheel_sec" : w->type == wheel_hour ? "wheel_hour":"wheel_day" ,
	//	   w->cur,i,remain);
	dlist_pushback(&w->items[i],(dlistnode*)t);
}

static void fire(wheelmgr *m,wheel *w,uint64_t tick){
	timer *t;
	dlist *items = &w->items[w->cur];
	if(w->type == wheel_sec){
		while((t = (timer*)dlist_pop(items))){
			int32_t ret = t->callback(t->expire,t->ud);
			if(ret >= 0 && ret <= MAX_TIMEOUT){
				if(ret > 0){
					t->timeout = ret;
				}
				t->expire = tick + t->timeout;
				//register again
				set(m,t,tick);
			}else{
				if(ret > 0){
					//todo: log
				}
				free(t);
			}
		}
	}else{
		while((t = (timer*)dlist_pop(items))){
			//find a suitable wheel
			set(m,t,tick);
		}
		fire(m,m->wheels[w->type-1],tick);	
	}

	uint16_t size = (uint16_t)wheel_size(w->type);
	assert(size < 3600);	
	w->cur = (w->cur+1)%size;	
	if(w->cur == 0 && w->type != wheel_day){
		fire(m,m->wheels[w->type+1],tick);
	}
}

void wheelmgr_tick(wheelmgr *m,uint64_t now){
	if(!m->lasttime){
		m->lasttime = now-1;
	}
	while(m->lasttime != now){
		fire(m,m->wheels[wheel_sec],++m->lasttime);
	}
} 

timer *wheelmgr_register(wheelmgr *m,uint32_t timeout,int32_t(*callback)(uint64_t,void*),void*ud){
	if(timeout == 0 || timeout > MAX_TIMEOUT || !callback)
		return NULL;
	uint64_t now = systick64(); 
	timer *t = calloc(1,sizeof(*t));
	t->timeout = timeout;
	t->expire = now + timeout;
	t->callback = callback;
	t->ud = ud;
	set(m,t,now);
	return t;
}

wheelmgr *wheelmgr_new(){
	wheelmgr *t = calloc(1,sizeof(*t));
	int i = 0;
	for(; i < wheel_day+1; ++i){
		t->wheels[i] = wheel_new(i);
	}
	return t;
}

void unregister_timer(timer *t){
	dlist_remove((dlistnode*)t);
	t->callback(TEVENT_DESTROY,t->ud);
	free(t);
}

void wheelmgr_del(wheelmgr *m){
	int i = 0;
	for(; i < wheel_day+1; ++i){
		uint16_t j = 0;
		uint16_t size = wheel_size(m->wheels[i]->type);
		for(; j < size; ++j){
			dlist *items = &m->wheels[i]->items[j];
		    timer *t;
			while((t = (timer*)dlist_pop(items))){
				t->callback(TEVENT_DESTROY,t->ud);
				free(t);				
			}
		}
		free(m->wheels[i]);
	}
	free(m);	
}
