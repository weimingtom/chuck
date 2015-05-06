#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util/string.h"
#include "util/list.h"
#include "util/bytebuffer.h"
#include "packet/rawpacket.h"
#include "socket/socket_helper.h"

static int32_t (*base_engine_add)(engine*,struct handle*,generic_callback) = NULL;

enum{
	SENDING   = SOCKET_END << 2,
};

#define RECV_BUFFSIZE 1024*16

typedef struct{
	listnode node;
	void     (*cb)(redisReply*,void *ud)
	void    *ud;
}reply_cb;

typedef struct redis_conn{
    socket_      		base;
    struct       		iovec wrecvbuf[1];
    char         		recvbuf[RECV_BUFFSIZE];
    reply_parse_context context; 
    struct       		iovec wsendbuf[MAX_WBAF];
    iorequest    		send_overlap;
    iorequest    		recv_overlap;       
    list         		send_list;
    list         		waitreplys;
    void         		(*on_disconnected)(struct redis_conn*,int32_t err);
}redis_conn;


typedef struct{
	listnode node;
	char    *buff;
	size_t   b;
	size_t   e;
}word;



rawpacket *convert(list *l,size_t space){
	char  tmp[32];
	char  c;
	char *end = "\r\n";
	word *w;
	buffer_writer writer;
	bytebuffer *buffer;
	rawpacket *p;	
	space += sprintf(tmp,"%u",list_size(l)) + 3;//plus head *,tail \r\n
	buffer = bytebuffer_new(space);
	buffer_writer_init(&writer,buffer,0);
	//write the head;
	c = '*';
	buffer_write(&writer,&c,sizeof(c));
	buffer_write(&writer,tmp,strlen(tmp));
	buffer_write(&writer,end,2);

	c = '$';
	while(NULL != (w = (word*)list_pop(l))){
		sprintf(tmp,"%u",w->e - w->b);
		buffer_write(&writer,&c,sizeof(c));	
		buffer_write(&writer,tmp,strlen(tmp));
		buffer_write(&writer,end,2);

		buffer_write(&writer,w->buff+w->b,w->e - w->b);
		buffer_write(&writer,end,2);

		free(w);
	}
	buffer_write(&writer,&c,sizeof(c));
	p = rawpacket_new_by_buffer(buffer,0);
	refobj_dec((refobj*)buffer);
	return p;
}


packet *build_request(const char *cmd){
	list l;
	list_init(&l);
	char  tmp[32];
	size_t len   = strlen(cmd);
	word  *w = NULL;
	size_t i,j,space;
	i = j = space = 0;
	for(; i < len; ++i){
		if(cmd[i] != ' '){
			if(!w){ 
				w = calloc(1,sizeof(*w));
				w->b = i;
				w->buff = request;
			}
		}else{
			if(w){
				//word finish
				w->e = i;
				space += (sprintf(tmp,"%u",(w->e - w->b)) + 3);//plus head $,tail \r\n
				space += (w->e - w->b) + 2;//plus tail \r\n
				list_pushback(&l,(listnode*)w);	
				w = NULL;
				--i;
			}
		}
	}
	w->e = i;
	space += (sprintf(tmp,"%u",(w->e - w->b)) + 3);//plus head $,tail \r\n
	space += (w->e - w->b) + 2;//plus tail \r\n
	list_pushback(&l,(listnode*)w);
	return (packet*)convert(&l,space);
}

#define STATUS  '+'    //状态回复（status reply）
#define ERROR   '-'    //错误回复（error reply）
#define IREPLY  ':'    //整数回复（integer reply）
#define BREPLY  '$'    //批量回复（bulk reply）
#define MBREPLY '*'    //多条批量回复（multi bulk reply）


handle *redis_connect(engine *e,sockaddr_ *addr,void (*on_disconnect)(handle*,int32_t err))
{
	int32_t fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(fd < 0) return NULL;
	if(0 != easy_connect(fd,addr,NULL)){
		close(fd);
		return NULL;
	}

	redis_conn *conn = calloc(1,sizeof(*conn));
	((handle*)conn)->fd = fd;
	construct_stream_socket(&conn->base);

	return (handle*)conn;
}

static inline void prepare_recv(redis_conn *c){
	c->wrecvbuf[0].iov_len = RECV_BUFFSIZE;
	c->wrecvbuf[0].iov_base = c->recvbuf;	
	c->recv_overlap.iovec_count = 1;
	c->recv_overlap.iovec = c->wrecvbuf;
}

static inline int32_t Send(redis_conn *c,int32_t flag){
	int32_t ret = stream_socket_send((handle*)c,&c->send_overlap,flag);		
	if(ret < 0 && -ret == EAGAIN)
		((socket_*)c)->status |= SENDING;
	return ret; 
}

static inline void PostRecv(redis_conn *c){
	prepare_recv(c);
	stream_socket_recv((handle*)c,&c->recv_overlap,IO_POST);		
}

static inline int32_t Recv(redis_conn *c){
	prepare_recv(c);
	return stream_socket_recv((handle*)c,&c->recv_overlap,IO_NOW);		
}

static inline iorequest *prepare_send(redis_conn *c)
{
	int32_t     i = 0;
	packet     *w = (packet*)list_begin(&c->send_list);
	bytebuffer *b;
	iorequest * O = NULL;
	uint32_t    buffer_size,size,pos;
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

static inline void update_send_list(redis_conn *c,int32_t _bytestransfer)
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


static void SendFinish(redis_conn *c,int32_t bytestransfer)
{
	update_send_list(c,bytestransfer);
	if(((socket_*)c)->status & SOCKET_CLOSE)
			return;
	if(!prepare_send(c)) {
		((socket_*)c)->status ^= SENDING;
		return;
	}
	Send(c,IO_POST);		
}

static void RecvFinish(redis_conn *c,int32_t bytestransfer,int32_t err_code)
{
	int32_t total_recv = 0;
	do{	
		if(bytestransfer == 0 || (bytestransfer < 0 && err_code != EAGAIN)){
			_close(c,err_code);
			return;	
		}else if(bytestransfer > 0){

			if(0 != process_reply(c))
				return;

			if(total_recv >= c->recv_bufsize){
				PostRecv(c);
				return;
			}else{
				bytestransfer = Recv(c);
				if(bytestransfer < 0 && (err_code = -bytestransfer) == EAGAIN) 
					return;
				else if(bytestransfer > 0)
					total_recv += bytestransfer;
			}
		}
	}while(1);
}

static void IoFinish(handle *sock,void *_,int32_t bytestransfer,int32_t err_code)
{
	iorequest  *io = ((iorequest*)_);
	redis_conn *c  = (redis_conn*)sock;
	if(((socket_*)c)->status & SOCKET_CLOSE)
		return;
	if(io == (iorequest*)&c->send_overlap && bytestransfer > 0)
		SendFinish(c,bytestransfer);
	else if(io == (iorequest*)&c->recv_overlap)
		RecvFinish(c,bytestransfer,err_code);
}


int32_t redis_query(handle *h,const char *str,void (*cb)(redisReply*,void *ud),void *ud){
	redis_conn *conn = (redis_conn*)h;
	if(((socket_*)h)->status & SOCKET_CLOSE)
		return -ESOCKCLOSE;
	if(!h->e)
		return -ENOASSENG;
	packet *p = build_request(str);
	reply_cb *repobj = calloc(1,sizeof(*repobj));
	if(cb){
		repobj->cb = cb;
		repobj->ud = ud;
		list_pushback(&conn->waitreplys,(listnode*)repobj);
	}
	list_pushback(&conn->send_list,(listnode*)p);
	if(!(((socket_*)conn)->status & SENDING)){
		prepare_send(conn);
		ret = Send(conn,IO_NOW);
		if(ret < 0 && ret == -EAGAIN) 
			return -EAGAIN;
		else if(ret > 0)
			update_send_list(conn,ret);
		return ret;
	}
}