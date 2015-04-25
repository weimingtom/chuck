#include <assert.h>
#include <stdio.h>
#include "timewheel.h"
#include "time.h"
#include "dlist.h"


enum{
	wheel_sec = 0,  
	wheel_hour,     
	wheel_day,      
};

typedef struct {
	uint8_t  type;
	uint16_t cur;
	dlist    items[0]; 
}wheel;

#define wheel_size(T) (T==wheel_sec?1000:T==wheel_hour?3600:T==wheel_day?24:-1)
#define down_size(T) (T==wheel_sec?1:T==wheel_hour?1000:T==wheel_day?3600:-1)

static wheel* wheel_new(uint8_t type){
	if(type >  wheel_day)
		return NULL;
	wheel *w = calloc(1,sizeof(*w)*wheel_size(type)*sizeof(dlist));	
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


static inline void add2wheel(wheelmgr *m,wheel *w,timer *t,uint64_t remain){
	uint64_t slots = wheel_size(w->type) - w->cur;
	if(slots > remain){
		uint16_t i = (w->cur + remain)%(wheel_size(w->type));
		dlist_pushback(&w->items[i],(dlistnode*)t);		
	}else{
		if(w->type == wheel_day)
			return;
		remain -= slots;
		remain /= wheel_size(w->type);
		return add2wheel(m,m->wheels[w->type+1],t,remain);		
	}
}

static inline void set(wheelmgr *m,timer *t,uint64_t tick,wheel *w){
	assert(t->expire > tick);
	if(t->expire > tick){
		uint64_t remain = t->expire - tick;
		add2wheel(m,w?w:m->wheels[wheel_sec],t,remain);
	}
}

static inline void down(wheelmgr *m,timer *t,uint64_t tick,wheel *w){
	assert(w->cur == 0);
	assert(t->expire >= tick);
	if(t->expire >= tick){
		uint64_t remain = (t->expire - tick) - wheel_size(w->type-1)-1;
		remain /= down_size(w->type);
		uint16_t i = w->cur + remain;
		dlist_pushback(&w->items[i],(dlistnode*)t);		
	}	
}

static void _tick(wheelmgr *m,wheel *w,uint64_t tick){
	timer *t;
	dlist *items = &w->items[w->cur];
	while((t = (timer*)dlist_pop(items))){
		//find a suitable wheel
		down(m,t,tick,m->wheels[w->type-1]);
	}
	uint16_t size = wheel_size(w->type); 
	w->cur = (w->cur+1)%size;			
	if(w->cur == 0 && w->type != wheel_day){
		_tick(m,m->wheels[w->type+1],tick);
	}	
}

static void fire(wheelmgr *m,wheel *w,uint64_t tick){
	timer *t;	
	uint16_t size = wheel_size(w->type); 
	w->cur = (w->cur+1)%size;			
	if(w->cur == 0 && w->type != wheel_day){
		_tick(m,m->wheels[w->type+1],tick);
	}
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
				set(m,t,tick,NULL);
			}else{
				if(ret > 0){
					//todo: log
				}
				free(t);
			}
		}
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

timer *wheelmgr_register(wheelmgr *m,uint64_t now,uint32_t timeout,int32_t(*callback)(uint64_t,void*),void*ud){
	if(timeout == 0 || timeout > MAX_TIMEOUT || !callback)
		return NULL;
	//uint64_t now = systick64();
	timer *t = calloc(1,sizeof(*t));
	t->timeout = timeout;
	t->expire = now + timeout;
	t->callback = callback;
	t->ud = ud;
	set(m,t,now,NULL);
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
