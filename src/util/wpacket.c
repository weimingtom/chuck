#include "util/wpacket.h"
#include "util/rpacket.h"

allocator *g_wpk_allocator = NULL;

static packet *wpacket_clone(packet*);
packet        *wpacket_makeforread(packet*);
packet        *rpacket_makeforwrite(packet*);


wpacket *wpacket_new(uint16_t size){
	size = size_of_pow2(size);
    if(size < 64) size = 64;
    bytebuffer *b = bytebuffer_new(size);

	wpacket *w = (wpacket*)CALLOC(g_wpk_allocator,1,sizeof(*w));
	((packet*)w)->type = WPACKET;
	((packet*)w)->buf = b;
	refobj_inc((refobj*)b);
	buffer_writer_init(&w->writer,b,sizeof(*w->len));
	w->len = (uint16_t*)b->data;
	w->data_size = sizeof(*w->len);
	((packet*)w)->buf->size = w->data_size;
	((packet*)w)->makeforwrite = wpacket_clone;
	((packet*)w)->makeforread =  wpacket_makeforread;	
	return w;
}

static packet *wpacket_clone(packet *p){
	wpacket *ori = (wpacket*)p;	
	if(p->type == WPACKET){
		wpacket *w = (wpacket*)CALLOC(g_wpk_allocator,1,sizeof(*w));
		((packet*)w)->type = WPACKET;
		((packet*)w)->buf = p->buf;
		((packet*)w)->start_pos = p->start_pos;
		w->data_size = ori->data_size;
		refobj_inc((refobj*)p->buf);		
		((packet*)w)->makeforwrite = wpacket_clone;
		((packet*)w)->makeforread =  wpacket_makeforread;
		return (packet*)w;		
	}else
		return NULL;
}

packet *rpacket_makeforwrite(packet *p){
	rpacket *ori = (rpacket*)p;	
	if(p->type == RPACKET){
		wpacket *w = (wpacket*)CALLOC(g_wpk_allocator,1,sizeof(*w));	
		((packet*)w)->type = WPACKET;
		((packet*)w)->start_pos = p->start_pos;
		((packet*)w)->buf = p->buf;
		w->data_size = ori->len_total + sizeof(ori->len_total);
		refobj_inc((refobj*)p->buf);		
		((packet*)w)->makeforwrite = wpacket_clone;
		((packet*)w)->makeforread =  wpacket_makeforread;		
		return (packet*)w;		
	}else
		return NULL;
}