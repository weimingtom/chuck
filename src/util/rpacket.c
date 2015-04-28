#include "util/rpacket.h"
#include "util/wpacket.h"

allocator *g_rpk_allocator = NULL;

static packet  *rpacket_clone(packet*);
packet  	   *wpacket_makeforread(packet*);
packet         *rpacket_makeforwrite(packet*);

rpacket *rpacket_new(bytebuffer *b,uint32_t start_pos){
	rpacket *r = (rpacket*)CALLOC(g_rpk_allocator,1,sizeof(*r));
	((packet*)r)->type = RPACKET;
	if(b){
		((packet*)r)->buf = b;
		((packet*)r)->start_pos = start_pos;
		refobj_inc((refobj*)b);
		buffer_reader_init(&r->reader,b,start_pos);
		r->len_total = rpacket_read_uint16(r);
		r->len_remain = r->len_total;
	}
	((packet*)r)->makeforwrite = rpacket_makeforwrite;
	((packet*)r)->makeforread = rpacket_clone;
	return r;
}


const void *rpacket_read_binary(rpacket *r,uint16_t *len){
	void *addr = 0;
	uint16_t size = rpacket_read_uint16(r);
	if(size == 0 || size > r->len_remain) return NULL;
	if(len) *len = size;
	if(reader_check_size(&r->reader,sizeof(int16_t))){
		addr = &r->reader.cur->data[r->reader.pos];
		r->reader.pos += size;
		r->len_remain -= size;
		if(r->len_remain && r->reader.pos >= r->reader.cur->size)
			buffer_reader_init(&r->reader,r->reader.cur->next,0);
	}else{
		if(!r->binbuf){
			r->binbuf = bytebuffer_new(r->len_total);
			r->binpos = 0;
		}
		addr = r->binbuf->data + r->binpos;
		while(size)
		{
			uint32_t copy_size = r->reader.cur->size - r->reader.pos;
			copy_size = copy_size >= size ? size:copy_size;
			memcpy(r->binbuf->data + r->binpos,r->reader.cur->data + r->reader.pos,copy_size);
			size -= copy_size;
			r->reader.pos += copy_size;
			r->len_remain -= copy_size;
			r->binpos += copy_size;
			if(r->len_remain && r->reader.pos >= r->reader.cur->size)
				buffer_reader_init(&r->reader,r->reader.cur->next,0);
		}		
	}
	return addr;
}

const char *rpacket_read_string(rpacket *r){
	return rpacket_read_binary(r,NULL);
}

static packet  *rpacket_clone(packet *p){
	rpacket *ori = (rpacket*)p;
	if(p->type == RPACKET){
		rpacket *r = rpacket_new(NULL,0);
		((packet*)r)->buf = p->buf;
		((packet*)r)->start_pos = p->start_pos;
		refobj_inc((refobj*)p->buf);
		buffer_reader_init(&r->reader,ori->reader.cur,ori->reader.pos);
		r->len_total =  ori->len_total;
		r->len_remain = r->len_total;
		((packet*)r)->makeforwrite = rpacket_makeforwrite;
		((packet*)r)->makeforread = rpacket_clone;		
		return (packet*)r;
	}
	return NULL;
}


packet  *wpacket_makeforread(packet *p){
	wpacket *ori = (wpacket*)p;
	if(p->type == WPACKET){
		rpacket *r = rpacket_new(NULL,0);
		((packet*)r)->buf = p->buf;
		((packet*)r)->start_pos = p->start_pos;
		refobj_inc((refobj*)p->buf);
		buffer_reader_init(&r->reader,p->buf,p->start_pos);
		r->len_total = ori->data_size - sizeof(*ori->len);
		r->len_remain = r->len_total;
		((packet*)r)->makeforwrite = rpacket_makeforwrite;
		((packet*)r)->makeforread = rpacket_clone;		
		return (packet*)r;		
	}
	return NULL;
}



