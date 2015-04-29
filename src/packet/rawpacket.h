#ifndef _RAWPACKET_H
#define _RAWPACKET_H

#include "packet/packet.h"
#include "mem/allocator.h"
#include "util/endian.h"

extern allocator* g_rawpk_allocator;

typedef struct
{
    packet          base;
    buffer_writer   writer;
}rawpacket;

rawpacket *rawpacket_new(uint32_t size);

//will add reference count of b
rawpacket *rawpacket_new_by_buffer(bytebuffer *b);

static inline void rawpacket_expand(rawpacket *raw,uint32_t newsize)
{

	newsize = size_of_pow2(newsize);
    if(newsize < 64) newsize = 64;
    bytebuffer *newbuff = bytebuffer_new(newsize);
   	bytebuffer *oldbuff = ((packet*)raw)->head;
   	memcpy(newbuff->data,oldbuff->data,oldbuff->size);
   	newbuff->size = oldbuff->size;
   	refobj_dec((refobj*)oldbuff);
    ((packet*)raw)->head = newbuff;
    //set writer to the end
    buffer_writer_init(&raw->writer,newbuff,((packet*)raw)->len_packet);
}

static inline void rawpacket_copy_on_write(rawpacket *raw)
{
	rawpacket_expand(raw,((packet*)raw)->len_packet);
}

static inline void rawpacket_append(rawpacket *raw,void *_,uint32_t size)
{
	char *in = (char*)_;
    if(!raw->writer.cur)
        rawpacket_copy_on_write(raw);
    else{
		uint32_t packet_len = ((packet*)raw)->len_packet;
    	uint32_t new_size = packet_len + size;
    	assert(new_size >= packet_len);
    	if(new_size <= packet_len) return;
    	if(new_size > ((packet*)raw)->head->cap){
    		rawpacket_expand(raw,new_size);
    	}
    	buffer_write(&raw->writer,in,size);
    	((packet*)raw)->len_packet = new_size;
    }	
}

static inline void *rawpacket_data(rawpacket *raw,uint32_t *size){
	if(size) *size = ((packet*)raw)->len_packet;
	return (void*)(((packet*)raw)->head->data + ((packet*)raw)->spos);
}

#endif