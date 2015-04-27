#include "socket/socket_helper.h"


static int32_t _bind(int32_t fd,sockaddr_ *addr){
	return bind(fd,(struct sockaddr*)addr,sizeof(*addr));
}

int32_t easy_listen(int32_t fd,sockaddr_ *server){
	errno = 0;
	if(_bind(fd,server) != 0)
		 return -errno;
	if(listen(fd,SOMAXCONN) != 0)
		return -errno;
	return 0;
}

int32_t easy_connect(int32_t fd,sockaddr_ *server,sockaddr_ *local){
	errno = 0;	
	if(local && 0 != _bind(fd,local))
		return -errno;
	int32_t ret = connect(fd,(struct sockaddr*)server,sizeof(*server));
	return ret == 0 ? ret : -errno;
}