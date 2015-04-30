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

#ifndef _CONNECTION_H_
#define _CONNECTION_H_


/*
//面向数据流的连接
#include "kendynet.h"
#include "packet.h"
#include "rpacket.h"
#include "wpacket.h"
#include "rawpacket.h"
#include "kn_refobj.h"
#include "kn_timer.h"
#include "decoder.h"


typedef struct connection
{
    refobj   refobj;    
    handle_t handle;
    struct    iovec wsendbuf[MAX_WBAF];
    struct    iovec wrecvbuf[2];
    st_io      send_overlap;
    st_io      recv_overlap;
    void*    ud;
    void      (*destroy_ud)(void*);
    uint32_t unpack_size; //还未解包的数据大小
    uint32_t unpack_pos;
    uint32_t next_recv_pos;
    buffer_t next_recv_buf;
    buffer_t unpack_buf;
    kn_list  send_list;//待发送的包
    kn_list  send_call_back;
    uint32_t recv_bufsize;
    void     (*on_packet)(struct connection*,packet_t);
    void     (*on_disconnected)(struct connection*,int err);
    kn_timer_t sendtimer;
    decoder* _decoder;
    uint8_t  doing_send:1;
    uint8_t  doing_recv:1;
    uint8_t  close_step:2;        
}connection,*connection_t;

typedef void (*CCB_SEND_FINISH)(connection_t);

typedef struct{
    kn_list_node list;
    CCB_SEND_FINISH callback;
    packet_t pk;
}st_send_finish_callback;

typedef void  (*CCB_PROCESS_PKT)(connection_t,packet_t);
typedef void (*CCB_DISCONNECTD)(connection_t,int err);

connection_t new_connection(handle_t sock,uint32_t buffersize,decoder *_decoder);
void    connection_close(connection_t c);
int      connection_send(connection_t c,packet_t p,CCB_SEND_FINISH);
static inline handle_t connection_gethandle(connection_t c){
    return c->handle;
} 

static inline void connection_setud(connection_t c,void *ud,void (*destroy_ud)(void*)){
    c->ud = ud;
    c->destroy_ud = destroy_ud;
}
static inline void* connection_getud(connection_t c){
    return c->ud;
}

static inline int connection_isclose(connection_t c){
    return c->close_step > 0;
}

int     connection_associate(engine_t,connection_t conn,CCB_PROCESS_PKT,CCB_DISCONNECTD);


typedef struct {
    decoder base;
    uint32_t maxpacket_size;
}rpk_decoder;

typedef struct {
    decoder base;
}rawpk_decoder;

decoder* new_rpk_decoder(uint32_t maxpacket_size);
decoder* new_rawpk_decoder();

*/


#endif    
