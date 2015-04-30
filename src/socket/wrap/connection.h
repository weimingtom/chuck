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

#include "socket/wrap/decoder.h"
#include "socket/socket.h" 

#define MAX_WBAF 512
#define MAX_SEND_SIZE 65535
#define MIN_RECV_BUFSIZE 1024   

typedef struct connection{
    socket_      base;
    struct       iovec wsendbuf[MAX_WBAF];
    struct       iovec wrecvbuf[2];
    iorequest    send_overlap;
    iorequest    recv_overlap;
    uint32_t     next_recv_pos;
    bytebuffer  *next_recv_buf;        
    list         send_list;//待发送的包
    uint32_t     recv_bufsize;
    void         (*on_packet)(struct connection*,packet*);
    void         (*on_disconnected)(struct connection*,int32_t err);
    decoder     *decoder_;
    uint8_t      closing_phase;
}connection;

connection *connection_new(int32_t fd,uint32_t buffersize,decoder *d);
int32_t     connection_send(connection *c,packet *p);
void        connection_close(connection *c);


static inline void 
connection_set_discnt_callback(connection *c,
                               void(*on_disconnected)(struct connection*,int32_t))
{
    c->on_disconnected = on_disconnected;
}

#endif    
