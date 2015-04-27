#include "socket/socket.h"

void close_socket(handle *h)
{
	socket_ *s = (socket_*)h;
	if(!s->status & SOCKET_CLOSE){
		iorequest *req;
		while((req = (iorequest*)list_pop(&s->pending_send))!=NULL){
			if(s->status & SOCKET_STREAM)
				s->stream_callback(h,req,-1,ESOCKCLOSE);
			else if(s->status & SOCKET_DATAGRAM)
				s->datagram_callback(h,req,-1,ESOCKCLOSE,0);
		}
		while((req = (iorequest*)list_pop(&s->pending_recv))!=NULL){
			if(s->status & SOCKET_STREAM)
				s->stream_callback(h,req,-1,ESOCKCLOSE);
			else if(s->status & SOCKET_DATAGRAM)
				s->datagram_callback(h,req,-1,ESOCKCLOSE,0);
		}		
		if(s->status & SOCKET_INLOOP){
			s->status |= SOCKET_CLOSE;
		}else{
			close(h->fd);
			free(h);				
		}
	}		
}