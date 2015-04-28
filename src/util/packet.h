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

#ifndef _PACKET_H
#define _PACKET_H

#include "util/bytebuffer.h"
#include "util/refobj.h"

enum{
	WPACKET = 1,
	RPACKET,
	RAWPACKET,
	PACKET_END,
};

typedef struct packet
{
    listnode    node;   
    bytebuffer* buf;
    struct packet*  (*makeforwrite)(struct packet*);
    struct packet*  (*makeforread)(struct packet*);        
    uint16_t    start_pos;
    uint8_t     type; 
}packet;


#define packet_makeforwrite(p) ((packet*)(p))->makeforwrite(p)
#define packet_makeforread(p) ((packet*)(p))->makeforread(p)

void packet_del(packet*);


#endif