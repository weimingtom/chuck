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
#ifndef _CHUCK_H
#define _CHUCK_H

#include    <unistd.h>
#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */
#include    <net/if.h>
#include    <sys/ioctl.h>
#include    <netinet/tcp.h>
#include    <fcntl.h>
#include    <stdint.h>

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression)\
    ({ long int __result;\
    do __result = (long int)(expression);\
    while(__result == -1L&& errno == EINTR);\
    __result;})
#endif


#define MAX_UINT32 0xffffffff
#define likely(x) __builtin_expect(!!(x), 1)  
#define unlikely(x) __builtin_expect(!!(x), 0)

static inline int32_t is_pow2(uint32_t size){
    return !(size&(size-1));
}

static inline uint32_t size_of_pow2(uint32_t size)
{
    if(is_pow2(size)) return size;
    size = size-1;
    size = size | (size>>1);
    size = size | (size>>2);
    size = size | (size>>4);
    size = size | (size>>8);
    size = size | (size>>16);
    return size + 1;
}

static inline uint8_t get_pow2(uint32_t size)
{
    uint8_t pow2 = 0;
    if(!is_pow2(size)) size = (size << 1);
    while(size > 1){
        pow2++;
        size = size >> 1;
    }
    return pow2;
}

static inline uint32_t align_size(uint32_t size,uint32_t align)
{
    align = size_of_pow2(align);
    if(align < 4) align = 4;
    uint32_t mod = size % align;
    if(mod == 0) 
        return size;
    else
        return (size/align + 1) * align;
}
    
#endif
