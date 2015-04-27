#include "net/socket.h"

extern void stream_socket_close(handle *h);
extern int32_t stream_socket_send(handle *h,iorequest*,int32_t);
extern int32_t stream_socket_recv(handle *h,iorequest*,int32_t);

void    close_socket(handle *h)
{
	socket_ *s = (socket_*)h;
	if(s->status & SOCKET_CLOSE){
		return;
	}else if(s->status & SOCKET_STREAM){
		stream_socket_close(h);
	}else if(s->status & SOCKET_DATAGRAM){

	}
}

int32_t socket_send(handle *h,iorequest *req,int32_t flag)
{
	socket_ *s = (socket_*)h;
	if(!h->e){
		return -ENOASSENG;
	}else if(s->status & SOCKET_CLOSE){
		return -ESOCKCLOSE;
	}else if(s->status & SOCKET_STREAM){
		return stream_socket_send(h,req,flag);
	}else if(s->status & SOCKET_DATAGRAM){
		
	}
	return -1;
}

int32_t socket_recv(handle *h,iorequest *req,int32_t flag)
{
	socket_ *s = (socket_*)h;
	if(!h->e){
		return -ENOASSENG;
	}else if(s->status & SOCKET_CLOSE){
		return -ESOCKCLOSE;
	}else if(s->status & SOCKET_STREAM){
		return stream_socket_recv(h,req,flag);
	}else if(s->status & SOCKET_DATAGRAM){
		
	}
	return -1;
}
