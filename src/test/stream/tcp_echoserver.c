#include "engine/engine.h"
#include "socket/socket_helper.h"
#include "socket/acceptor.h"
#include "util/time.h"
#include "session.h"


/*
int timer_callback(uint32_t event,void *ud){
	if(event == TEVENT_TIMEOUT){	
		printf("client_count:%d,totalbytes:%ld MB/s\n",client_count,totalbytes/1024/1024);
		totalbytes = 0;
	}
	return 0;
}*/

static void on_connection(int32_t fd,sockaddr_ *_,void *ud){
	printf("on_connection\n");
	engine *e = (engine*)ud;
	easy_noblock(fd,1);
	handle *h = new_stream_socket(fd);
	engine_add(e,h,(generic_callback)transfer_finish);
	struct session *s = session_new(h);
	session_recv(s);
	++client_count;
	printf("%d\n",client_count);	
}


int main(int argc,char **argv){
	signal(SIGPIPE,SIG_IGN);
	engine *e = engine_new();
	sockaddr_ server;
	easy_sockaddr_ip4(&server,argv[1],atoi(argv[2]));
	int32_t fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	easy_addr_reuse(fd,1);
	easy_noblock(fd,1);
	if(0 == easy_listen(fd,&server)){
		handle *accptor = acceptor_new(fd,e);
		engine_add(e,accptor,(generic_callback)on_connection);
		last_tick = systick64();
		engine_run(e);
	}else{
		printf("server start error\n");
	}
	return 0;
}
