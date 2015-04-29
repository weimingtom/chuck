#include "util/wpacket.h"
#include "util/rpacket.h"

int main(){
	wpacket *w1 = wpacket_new(64);
	wpacket_write_uint32(w1,1);
	wpacket_write_uint32(w1,2);
	wpacket_write_uint32(w1,3);
	wpacket_write_uint32(w1,4);
	wpacket_write_string(w1,"haha nihao");
	//buffer expand
	wpacket_write_string(w1,"afdafdfdfaserasdvasdafadfaeffdffdafeadvdsdfadsffdsfe");
	
	printf("--------------r1---------------\n");
	rpacket *r1 = make_readpacket(w1);
	printf("%d\n",rpacket_read_uint32(r1));
	printf("%d\n",rpacket_read_uint32(r1));
	printf("%d\n",rpacket_read_uint32(r1));
	printf("%d\n",rpacket_read_uint32(r1));
	printf("%s\n",rpacket_read_string(r1));
	printf("%s\n",rpacket_read_string(r1));


	//test copy on write
	wpacket *w2 = make_writepacket(w1);
	wpacket_write_string(w2,"lakuku");

	printf("--------------r2---------------\n");
	//write to w2 didn't change w1
	rpacket *r2 = make_readpacket(w1);
	printf("%d\n",rpacket_read_uint32(r2));
	printf("%d\n",rpacket_read_uint32(r2));
	printf("%d\n",rpacket_read_uint32(r2));
	printf("%d\n",rpacket_read_uint32(r2));
	printf("%s\n",rpacket_read_string(r2));
	printf("%s\n",rpacket_read_string(r2));

	printf("--------------r3---------------\n");
	rpacket *r3 = make_readpacket(w2);	
	printf("%d\n",rpacket_read_uint32(r3));
	printf("%d\n",rpacket_read_uint32(r3));
	printf("%d\n",rpacket_read_uint32(r3));
	printf("%d\n",rpacket_read_uint32(r3));
	printf("%s\n",rpacket_read_string(r3));
	printf("%s\n",rpacket_read_string(r3));
	printf("%s\n",rpacket_read_string(r3));

	printf("--------------r3---------------\n");
	wpacket *w3 = make_writepacket(r3);
	wpacket_write_string(w3,"lalakuku");
	rpacket *r4 = make_readpacket(w3);	
	printf("%d\n",rpacket_read_uint32(r4));
	printf("%d\n",rpacket_read_uint32(r4));
	printf("%d\n",rpacket_read_uint32(r4));
	printf("%d\n",rpacket_read_uint32(r4));
	printf("%s\n",rpacket_read_string(r4));
	printf("%s\n",rpacket_read_string(r4));
	printf("%s\n",rpacket_read_string(r4));
	printf("%s\n",rpacket_read_string(r4));
	

	packet_del((packet*)w1);
	packet_del((packet*)w2);
	packet_del((packet*)w3);	

	packet_del((packet*)r1);
	packet_del((packet*)r2);
	packet_del((packet*)r3);
	packet_del((packet*)r4);

	return 0;
}