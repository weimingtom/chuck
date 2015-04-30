#include "socket/wrap/decoder.h"
#include "packet/rawpacket.h"
#include "packet/rpacket.h"

static packet *rawpk_unpack(decoder *d,int32_t *err){
	rawpacket  *raw;
	bytebuffer *next;
	if(err) *err = 0;
	if(!d->size) return NULL;

	raw = rawpacket_new_by_buffer(d->buff);
	d->size -= d->buff->size;
	if(d->buff->next){
		next = d->buff->next;
		refobj_dec((refobj*)d->buff);
		d->buff = next;		
	}else{
		refobj_dec((refobj*)d->buff);
		d->buff = NULL;
	}

	return (packet*)raw;
}

static packet *rpk_unpack(decoder *d,int32_t *err){
	TYPE_HEAD     pk_len;
	uint32_t      pk_total,size;
	buffer_reader reader;
	rpacket      *rpk;
	bytebuffer   *next;
	if(err) *err = 0;

	if(d->size <= SIZE_HEAD)
		return NULL;

	buffer_reader_init(&reader,d->buff,d->pos);
	if(SIZE_HEAD != buffer_read(&reader,(char*)&pk_len,SIZE_HEAD))
		return NULL;
	pk_len = _ntoh16(pk_len);
	pk_total = pk_len + SIZE_HEAD;
	if(pk_total > d->max_packet_size){
		if(err) *err = DERR_TOOLARGE;
		return NULL;
	}
	if(pk_total > d->size)
		return NULL;

	rpk = rpacket_new(d->buff,d->pos);

	do{
		size = d->buff->size - d->pos;
		size = pk_total > size ? size:pk_total;
		d->pos    += size;
		pk_total  -= size;
		d->size -= size;
		if(d->pos >= d->buff->cap)
		{
			if(!d->buff->next){
				assert(pk_total == 0);
				refobj_dec((refobj*)d->buff);
				d->buff = NULL;
				break;
			}
			d->pos = 0;
			next = d->buff->next;
			refobj_dec((refobj*)d->buff);
			d->buff = next;
		}
	}while(pk_total);
	return (packet*)rpk;
}


decoder *rpacket_decoder_new(uint32_t max_packet_size){
	decoder *d = calloc(1,sizeof(*d));
	d->unpack = rpk_unpack;
	d->max_packet_size = max_packet_size;
	return d;
}

decoder *rawpacket_decoder_new(){
	decoder *d = calloc(1,sizeof(*d));
	d->unpack = rawpk_unpack;
	return d;	
}

void decoder_del(decoder *d){
	if(d->dctor) d->dctor(d);
	if(d->buff) refobj_dec((refobj*)d->buff);
	free(d);
}