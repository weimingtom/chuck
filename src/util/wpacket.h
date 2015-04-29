/*
    Copyright (C) <2015>  <sniperHW@163.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _WPACKET_H
#define _WPACKET_H

#include "util/packet.h"
#include "mem/allocator.h"
#include "util/endian.h"


typedef struct
{
    packet          base;
    buffer_writer   writer;
    TYPE_HEAD      *len;
}wpacket;


wpacket *wpacket_new(uint16_t size);


static inline void wpacket_data_copy(wpacket *w,bytebuffer *buf)
{
    char *ptr = buf->data;
    bytebuffer *from = ((packet*)w)->head;
    uint16_t    pos  = ((packet*)w)->spos; 
    uint16_t    size = ((packet*)w)->len_packet; 
    do{
        uint16_t copy_size = from->size - pos;
        if(copy_size > size) copy_size = size;
        memcpy(ptr,from->data+pos,copy_size);
        size -= copy_size;
        ptr  += copy_size;
        from  = from->next;
        pos   = 0;
    }while(size);
    buf->size = ((packet*)w)->len_packet;
}

static inline void copy_on_write(wpacket *w)
{
    uint32_t size = size_of_pow2(((packet*)w)->len_packet);
    if(size < 64) size = 64;
    bytebuffer *newbuff = bytebuffer_new(size);
    wpacket_data_copy(w,newbuff);
    refobj_dec((refobj*)((packet*)w)->head);
    ((packet*)w)->head = newbuff;
    //set writer to the end
    buffer_writer_init(&w->writer,newbuff,((packet*)w)->len_packet);
    w->len = (uint16_t*)newbuff->data;
}


static inline void wpacket_expand(wpacket *w,uint16_t size)
{
    size = size_of_pow2(size);
    if(size < 64) size = 64;
    w->writer.cur->next = bytebuffer_new(size);
    buffer_writer_init(&w->writer,w->writer.cur->next,0);
}

static inline void wpacket_write(wpacket *w,char *in,uint16_t size)
{
    uint16_t packet_len = ((packet*)w)->len_packet;
    uint16_t new_size = packet_len + size;
    assert(new_size > packet_len);
    if(new_size < packet_len){
        return;
    }
    if(!w->writer.cur)
        copy_on_write(w);
    do{
        uint16_t ret;
        if(!w->writer.cur || 0 == (ret = (uint16_t)buffer_write(&w->writer,in,(uint32_t)size)))
            wpacket_expand(w,size);
        else{
            in += ret;
            size -= ret;
        }
    }while(size);
    ((packet*)w)->len_packet = new_size;
    *w->len = _hton16(new_size - sizeof(*w->len)); 
}

static inline void wpacket_write_uint8(wpacket *w,uint8_t value)
{   
    wpacket_write(w,(char*)&value,sizeof(value));
}

static inline void wpacket_write_uint16(wpacket *w,uint16_t value)
{
    value = _hton16(value);    
    wpacket_write(w,(char*)&value,sizeof(value));
}

static inline void wpacket_write_uint32(wpacket *w,uint32_t value)
{   
    value = _hton32(value);
    wpacket_write(w,(char*)&value,sizeof(value));
}

static inline void wpacket_write_uint64(wpacket *w,uint64_t value)
{   
    value = _hton64(value);
    wpacket_write(w,(char*)&value,sizeof(value));
}

static inline void wpacket_write_double(wpacket *w,uint64_t value)
{   
    wpacket_write(w,(char*)&value,sizeof(value));
}

static inline void wpacket_write_binary(wpacket *w,const void *value,uint16_t size)
{
    assert(value);
    wpacket_write_uint16(w,size);
    wpacket_write(w,(char*)value,size);
}

static inline void wpacket_write_string(wpacket *w ,const char *value)
{
    wpacket_write_binary(w,value,strlen(value)+1);
}


#endif  