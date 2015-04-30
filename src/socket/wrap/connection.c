#include "socket/wrap/connection.h"
#include "engine/engine.h"

enum{
	RECVING   = 1 << 5,
	SENDING   = 1 << 6,
};

static inline void prepare_recv(connection *c){
	bytebuffer *buf;
	uint32_t    pos;
	int32_t     i = 0;
	uint32_t    free_buffer_size;
	uint32_t    recv_size;
	if(!c->next_recv_buf){
		c->next_recv_buf = bytebuffer_new(c->recv_bufsize);
		c->next_recv_pos = 0;
	}
	buf = c->next_recv_buf;
	pos = c->next_recv_pos;
	recv_size = c->recv_bufsize;
	do
	{
		free_buffer_size = buf->cap - pos;
		free_buffer_size = recv_size > free_buffer_size ? free_buffer_size:recv_size;
		c->wrecvbuf[i].iov_len = free_buffer_size;
		c->wrecvbuf[i].iov_base = buf->data + pos;
		recv_size -= free_buffer_size;
		pos += free_buffer_size;
		if(recv_size && pos >= buf->cap)
		{
			pos = 0;
			if(!buf->next)
				buf->next = bytebuffer_new(c->recv_bufsize);
			buf = buf->next;
		}
		++i;
	}while(recv_size);
	c->recv_overlap.iovec_count = i;
	c->recv_overlap.iovec = c->wrecvbuf;
}

static inline void PostRecv(connection *c){
	((socket_*)c)->status |= RECVING;
	prepare_recv(c);
	stream_socket_recv((handle*)c,&c->recv_overlap,IO_POST);		
}

static inline int32_t Recv(connection *c,int32_t *err){
	int32_t ret;
	prepare_recv(c);
	if(err) *err = 0;
	ret = stream_socket_recv((handle*)c,&c->recv_overlap,IO_NOW);		
	if(ret < 0 && err)
		*err = -ret;
	return ret; 
}

static inline int32_t Send(connection *c,int32_t *err,int32_t flag){
	int32_t ret;
	if(err) *err = 0;
	ret = stream_socket_send((handle*)c,&c->send_overlap,flag);		
	if(ret < 0 && err)
		*err = -ret;
	if(flag == IO_POST || -ret == EAGAIN)
		((socket_*)c)->status |= SENDING;
	return ret; 
}


static inline void update_next_recv_pos(connection *c,int32_t _bytestransfer)
{
	assert(_bytestransfer >= 0);
	uint32_t bytestransfer = (uint32_t)_bytestransfer;
	uint32_t size;
	decoder *decoder_ = c->decoder_;
	if(!decoder_->buff)
		decoder_init(decoder_,c->next_recv_buf,c->next_recv_pos);
	decoder_->size += bytestransfer;
	do{
		size = c->next_recv_buf->cap - c->next_recv_pos;
		size = size > bytestransfer ? bytestransfer:size;
		c->next_recv_buf->size += size;
		c->next_recv_pos += size;
		bytestransfer -= size;
		if(c->next_recv_pos >= c->next_recv_buf->cap)
		{
			bytebuffer_set(&c->next_recv_buf,c->next_recv_buf->next);
			c->next_recv_pos = 0;
		}
	}while(bytestransfer);
}


static inline void _close(connection *c,int32_t err){
	if(c->closing_phase == 2) return;
	c->closing_phase = 2;
	if(c->on_disconnected) c->on_disconnected(c,err);
	close_socket((handle*)c);
}

static void RecvFinish(connection *c,int32_t bytestransfer,int32_t err_code)
{
	int32_t total_recv = 0;
	packet *pk;
	if(c->closing_phase) return;	
	do{	
		if(bytestransfer == 0 || (bytestransfer < 0 && err_code != EAGAIN)){
			_close(c,err_code);
			return;	
		}else if(bytestransfer > 0){
			total_recv += bytestransfer;
			update_next_recv_pos(c,bytestransfer);
			int32_t unpack_err;
			do{
				pk = c->decoder_->unpack(c->decoder_,&unpack_err);
				if(pk){
					c->on_packet(c,pk);
					packet_del(pk);
				}else if(unpack_err != 0){
					_close(c,err_code);
					break;
				}
			}while(pk);
			if(total_recv >= c->recv_bufsize){
				PostRecv(c);
				return;
			}else{
				bytestransfer = Recv(c,&err_code);
				if(bytestransfer < 0 && err_code == EAGAIN) 
					return;
			}
		}
	}while(1);
}

static inline iorequest *prepare_send(connection *c)
{
	int32_t     i = 0;
	packet     *w = (packet*)list_begin(&c->send_list);
	bytebuffer *b;
	uint32_t pos;
	iorequest * O = NULL;
	uint32_t    buffer_size = 0;
	uint32_t    size = 0;
	uint32_t    send_size_remain = MAX_SEND_SIZE;
	while(w && i < MAX_WBAF && send_size_remain > 0)
	{
		pos = ((packet*)w)->spos;
		b =   ((packet*)w)->head;
		buffer_size = ((packet*)w)->len_packet;
		while(i < MAX_WBAF && b && buffer_size && send_size_remain > 0)
		{
			c->wsendbuf[i].iov_base = b->data + pos;
			size = b->size - pos;
			size = size > buffer_size ? buffer_size:size;
			size = size > send_size_remain ? send_size_remain:size;
			buffer_size -= size;
			send_size_remain -= size;
			c->wsendbuf[i].iov_len = size;
			++i;
			b = b->next;
			pos = 0;
		}
		if(send_size_remain > 0) w = (packet*)((listnode*)w)->next;
	}
	if(i){
		c->send_overlap.iovec_count = i;
		c->send_overlap.iovec = c->wsendbuf;
		O = (iorequest*)&c->send_overlap;
	}
	return O;

}
static inline void update_send_list(connection *c,int32_t _bytestransfer)
{
	assert(_bytestransfer >= 0);
	packet     *w;
	uint32_t    bytestransfer = (uint32_t)_bytestransfer;
	uint32_t    size;
	do{
		w = (packet*)list_begin(&c->send_list);
		assert(w);
		if((uint32_t)bytestransfer >= ((packet*)w)->len_packet)
		{
			list_pop(&c->send_list);
			bytestransfer -= ((packet*)w)->len_packet;
			packet_del(w);
		}else{
			do{
				size = ((packet*)w)->head->size - ((packet*)w)->spos;
				size = size > (uint32_t)bytestransfer ? (uint32_t)bytestransfer:size;
				bytestransfer -= size;
				((packet*)w)->spos += size;
				((packet*)w)->len_packet -= size;
				if(((packet*)w)->spos >= ((packet*)w)->head->size)
				{
					((packet*)w)->spos = 0;
					((packet*)w)->head = ((packet*)w)->head->next;
				}
			}while(bytestransfer);
		}
	}while(bytestransfer);
}


static void SendFinish(connection *c,int32_t bytestransfer,int32_t err_code)
{
	if(bytestransfer == 0 || (bytestransfer < 0 && err_code != EAGAIN)){
		_close(c,err_code);
	}else{		
		for(;;){
			update_send_list(c,bytestransfer);
			if(!prepare_send(c)) {
				((socket_*)c)->status ^= SENDING;
				if(c->closing_phase)
					_close(c,0);
				return;
			}
			bytestransfer = Send(c,&err_code,IO_NOW);
			if(bytestransfer < 0 && err_code == EAGAIN) 
				return;		
		}		
	}
}

static void IoFinish(handle *sock,void *_,int32_t bytestransfer,int32_t err_code)
{
	iorequest  *io = ((iorequest*)_);
	connection *c  = (connection*)sock;
	if(io == (iorequest*)&c->send_overlap)
		SendFinish(c,bytestransfer,err_code);
	else if(io == (iorequest*)&c->recv_overlap)
		RecvFinish(c,bytestransfer,err_code);
	else{
		_close(c,0);	
	}
}	

static int32_t imp_engine_add(engine *e,handle *h,generic_callback callback){
	int32_t ret;
	assert(e && h && callback);
	if(h->e) return -EASSENG;
	//call the base_engine_add first
	ret = ((connection*)h)->base_engine_add(e,h,(generic_callback)IoFinish);
	if(ret == 0){
		((connection*)h)->on_packet = (void(*)(struct connection*,packet*))callback;
		//post the first recv request
		if(!(((socket_*)h)->status & RECVING))
			PostRecv((connection*)h);
	}
	return ret;
}

int32_t connection_send(connection *c,packet *p){
	if(p->type != WPACKET && p->type != RAWPACKET){
		packet_del(p);
		return -EINVIPK;
	}	
	if(c->closing_phase){
		packet_del(p);
		return -ESOCKCLOSE;
	}	
	list_pushback(&c->send_list,(listnode*)p);
	if(!(((socket_*)c)->status & SENDING)){
		prepare_send(c);
		return Send(c,NULL,IO_POST);
	}
	return -EAGAIN;
}

void connection_dctor(void *_)
{
	connection *c = (connection*)_;
	packet *p;
	while((p = (packet*)list_pop(&c->send_list))!=NULL)
		packet_del(p);
	bytebuffer_set(&c->next_recv_buf,NULL);
	decoder_del(c->decoder_);	
}

connection *connection_new(int32_t fd,uint32_t buffersize,decoder *d)
{
	buffersize = size_of_pow2(buffersize);
    if(buffersize < MIN_RECV_BUFSIZE) buffersize = MIN_RECV_BUFSIZE;	
	connection *c 	 = calloc(1,sizeof(*c));
	((handle*)c)->fd = fd;
	c->recv_bufsize  = buffersize;
	c->next_recv_buf = bytebuffer_new(buffersize);
	construct_stream_socket(&c->base);
	//save socket_ imp_engine_add,and replace with self
	c->base_engine_add = ((handle*)c)->imp_engine_add;
	((handle*)c)->imp_engine_add = imp_engine_add;
	((socket_*)c)->dctor = connection_dctor;
	c->decoder_ = d ? d:rawpacket_decoder_new();
	decoder_init(c->decoder_,c->next_recv_buf,0);
	return c;
}

void connection_close(connection *c){
	if(c->closing_phase) return;
	c->closing_phase = 1;
	if(!(((socket_*)c)->status & SENDING)){
		//no pending send,close immediate
		_close(c,0);
	}else{
		disable_read((handle*)c);
		//shutdown read
		shutdown(((handle*)c)->fd,SHUT_RD);
		//添加定时器确保待发送数据发送完毕或发送超时才调用调用refobj_dec
		/*engine_t e = kn_sock_engine(c->handle);
		if(e){
			if(c->sendtimer){
				 kn_del_timer(c->sendtimer);
				 c->sendtimer = NULL;
			}
			c->sendtimer = kn_reg_timer(e,5000,cb_lastsend,c);
			if(!c->sendtimer)
				_force_close(c,0);			
		}*/			
	} 	
}
