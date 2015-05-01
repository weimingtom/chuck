#include "engine/engine.h"
#include "socket/socket_helper.h"
#include "util/time.h"
#include "util/timerfd.h"
#include "socket/wrap/connection.h"
#include "packet/wpacket.h"
#include "packet/rpacket.h"
#include "socket/connector.h"


static void on_packet(connection *c,packet *p){
	rpacket *rpk = (rpacket*)p;
	uint64_t id = rpacket_peek_uint64(rpk);
	if(id == (uint64_t)c){
		connection_send(c,make_writepacket(p));
	}
}


static void on_connected(int32_t fd,int32_t err,void *ud){
	if(fd >= 0 && err == 0){
		engine *e = (engine*)ud;
		connection *c = connection_new(fd,65535,rpacket_decoder_new(1024));
		engine_add(e,(handle*)c,(generic_callback)on_packet);
		packet *p = (packet*)wpacket_new(64);
		wpacket_write_uint64((wpacket*)p,(uint64_t)c);
		wpacket_write_string((wpacket*)p,"hello world\n");
		connection_send(c,p);		
	}else{
		printf("connect error\n");
	}
}


int main(int argc,char **argv){
	signal(SIGPIPE,SIG_IGN);
	engine *e = engine_new();
	sockaddr_ server;
	easy_sockaddr_ip4(&server,argv[1],atoi(argv[2]));
	uint32_t size = atoi(argv[3]);
	uint32_t i = 0;
	for( ; i < size; ++i){
		int32_t fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		easy_noblock(fd,1);
		int32_t ret;
		if(0 == (ret = easy_connect(fd,&server,NULL)))
			on_connected(fd,0,e);
		else if(ret == -EINPROGRESS){
			handle *contor = connector_new(fd,e,2000);
			engine_add(e,contor,(generic_callback)on_connected);			
		}else{
			printf("connect to %s %d error\n",argv[1],atoi(argv[2]));
		}
	}
	engine_run(e);
	return 0;
}