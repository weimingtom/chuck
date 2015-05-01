/*
static void datagram_destroy(void *ptr)
{
	datagram_t c = (datagram_t)ptr;
	buffer_release(c->recv_buf);
	kn_close_sock(c->handle);
	destroy_decoder(c->_decoder);
	if(c->ud && c->destroy_ud){
		c->destroy_ud(c->ud);
	}
	free(c);				
}

static inline datagram_t _new_datagram(handle_t sock,uint32_t buffersize,decoder *_decoder)
{
	buffersize = size_of_pow2(buffersize);
    	if(buffersize < 1024) buffersize = 1024;	
	datagram_t c = calloc(1,sizeof(*c));
	c->recv_bufsize = buffersize;
	refobj_init((refobj*)c,datagram_destroy);
	c->handle = sock;
	kn_sock_setud(sock,c);
	c->_decoder = _decoder;
	if(!c->_decoder) c->_decoder = new_datagram_rawpk_decoder();
	return c;
}

datagram_t new_datagram(int domain,uint32_t buffersize,decoder *_decoder){
	handle_t l = kn_new_sock(domain,SOCK_DGRAM,IPPROTO_UDP);
	if(!l) 
		return NULL;
	return 	_new_datagram(l,buffersize,_decoder);	
}

static inline void prepare_recv(datagram_t c){
	if(c->recv_buf)
		buffer_release(c->recv_buf);
	c->recv_buf = buffer_create(c->recv_bufsize);
	c->wrecvbuf.iov_len = c->recv_bufsize;
	c->wrecvbuf.iov_base = c->recv_buf->buf;
	c->recv_overlap.iovec_count = 1;
	c->recv_overlap.iovec = &c->wrecvbuf;	
}

static inline void PostRecv(datagram_t c){
	prepare_recv(c);
	kn_sock_post_recv(c->handle,&c->recv_overlap);	
	c->doing_recv = 1;	
}

static int raw_unpack(decoder *_,void* _1){
	((void)_);
	datagram_t c = (datagram_t)_1;
	packet_t r = (packet_t)rawpacket_create1(c->recv_buf,0,c->recv_buf->size);
	c->on_packet(c,r,&c->recv_overlap.addr); 
	destroy_packet(r);
	return 0;
}

static int rpk_unpack(decoder *_,void *_1){
	((void)_);
	datagram_t c = (datagram_t)_1;
	if(c->recv_buf->size <= sizeof(uint32_t))
		return 0;	
	uint32_t pk_len = 0;
	uint32_t pk_hlen;
	buffer_read(c->recv_buf,0,(int8_t*)&pk_len,sizeof(pk_len));
	pk_hlen = kn_ntoh32(pk_len);
	uint32_t pk_total_size = pk_hlen+sizeof(pk_len);
	if(c->recv_buf->size < pk_total_size)
		return 0;	
	packet_t r = (packet_t)rpk_create(c->recv_buf,0,pk_len);
	c->on_packet(c,r,&c->recv_overlap.addr); 
	destroy_packet(r);	
	return 0;
}	

static void IoFinish(handle_t sock,void *_,int32_t bytestransfer,int32_t err_code)
{
	datagram_t c = kn_sock_getud(sock);
	c->doing_recv = 0;	
	refobj_inc((refobj*)c);
	if(bytestransfer > 0 && ((st_io*)_)->recvflags != MSG_TRUNC){
		c->recv_buf->size = bytestransfer;
		c->_decoder->unpack(c->_decoder,c);
	}
	PostRecv(c);
	refobj_dec((refobj*)c);
}

int datagram_send(datagram_t c,packet_t w,kn_sockaddr *addr)
{
	int ret = -1;
	do{	
		if(!addr){
			errno = EINVAL;
			break;
		}
		if(packet_type(w) != WPACKET && packet_type(w) != RAWPACKET){
			errno = EMSGSIZE;
			break;
		}
		st_io o;
		int32_t i = 0;
		uint32_t size = 0;
		uint32_t pos = packet_begpos(w);
		buffer_t b = packet_buf(w);
		uint32_t buffer_size = packet_datasize(w);
		while(i < MAX_WBAF && b && buffer_size)
		{
			c->wsendbuf[i].iov_base = b->buf + pos;
			size = b->size - pos;
			size = size > buffer_size ? buffer_size:size;
			buffer_size -= size;
			c->wsendbuf[i].iov_len = size;
			++i;
			b = b->next;
			pos = 0;
		}
		if(buffer_size != 0){
			errno = EMSGSIZE;
			break;		
		}
		o.iovec_count = i;
		o.iovec = c->wsendbuf;
		o.addr = *addr;
		ret = kn_sock_send(c->handle,&o);
	}while(0);
	destroy_packet(w);
	return ret;
}*/

#include "socket/wrap/datagram.h"
#include "engine/engine.h"

static int32_t (*base_engine_add)(engine*,struct handle*,generic_callback) = NULL;


int32_t datagram_send(datagram *c,packet *p,sockaddr_ *addr)
{
	int ret = -1;
	do{	
		if(!addr){
			errno = EINVAL;
			break;
		}
		if(packet_type(w) != WPACKET && packet_type(w) != RAWPACKET){
			errno = EMSGSIZE;
			break;
		}
		st_io o;
		int32_t i = 0;
		uint32_t size = 0;
		uint32_t pos = packet_begpos(w);
		buffer_t b = packet_buf(w);
		uint32_t buffer_size = packet_datasize(w);
		while(i < MAX_WBAF && b && buffer_size)
		{
			c->wsendbuf[i].iov_base = b->buf + pos;
			size = b->size - pos;
			size = size > buffer_size ? buffer_size:size;
			buffer_size -= size;
			c->wsendbuf[i].iov_len = size;
			++i;
			b = b->next;
			pos = 0;
		}
		if(buffer_size != 0){
			errno = EMSGSIZE;
			break;		
		}
		o.iovec_count = i;
		o.iovec = c->wsendbuf;
		o.addr = *addr;
		ret = kn_sock_send(c->handle,&o);
	}while(0);
	destroy_packet(w);
	return ret;
}


static int32_t imp_engine_add(engine *e,handle *h,generic_callback callback){
	int32_t ret;
	assert(e && h && callback);
	if(h->e) return -EASSENG;
	//call the base_engine_add first
	ret = base_engine_add(e,h,(generic_callback)IoFinish);
	if(ret == 0){
		((datagram*)h)->on_packet = (void(*)(datagram*,packet*))callback;
		//post the first recv request
		if(!(((socket_*)h)->status & RECVING))
			PostRecv((datagram*)h);
	}
	return ret;
}

void datagram_dctor(void *_)
{
	datagram *d = (datagram*)_;
	bytebuffer_set(&d->next_recv_buf,NULL);
	decoder_del(d->decoder_);	
}

datagram *datagram_new(int32_t fd,uint32_t buffersize,decoder *d)
{
	buffersize = size_of_pow2(buffersize);
    if(buffersize < MIN_RECV_BUFSIZE) buffersize = MIN_RECV_BUFSIZE;	
	datagram *dgarm 	 = calloc(1,sizeof(*dgarm));
	((handle*)dgarm)->fd = fd;
	dgarm->recv_bufsize  = buffersize;
	dgarm->next_recv_buf = bytebuffer_new(buffersize);
	construct_datagram_socket(&dgarm->base);
	//save socket_ imp_engine_add,and replace with self
	if(!base_engine_add)
		base_engine_add = ((handle*)dgarm)->imp_engine_add; 
	((handle*)dgarm)->imp_engine_add = imp_engine_add;
	((socket_*)dgarm)->dctor = datagram_dctor;
	dgarm->decoder_ = d ? d:rawpacket_decoder_new();
	decoder_init(dgarm->decoder_,dgarm->next_recv_buf,0);
	return c;
}