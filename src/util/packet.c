#include "util/packet.h"
#include "util/wpacket.h"
#include "util/rpacket.h"

void packet_del(packet *p){
	if(p->type == RPACKET && ((rpacket*)p)->binbuf)
		refobj_dec((refobj*)((rpacket*)p)->binbuf);
	refobj_dec((refobj*)p->head);
	free(p);
}