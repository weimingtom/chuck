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
	rpacket *r1 = packet_makeforread(w1);
	printf("%d\n",rpacket_read_uint32(r1));
	printf("%d\n",rpacket_read_uint32(r1));
	printf("%d\n",rpacket_read_uint32(r1));
	printf("%d\n",rpacket_read_uint32(r1));
	printf("%s\n",rpacket_read_string(r1));
	printf("%s\n",rpacket_read_string(r1));


	//test copy on write
	wpacket *w2 = packet_makeforwrite(w1);
	wpacket_write_string(w2,"lakuku");

	printf("--------------r2---------------\n");
	//write to w2 didn't change w1
	rpacket *r2 = packet_makeforread(w1);
	printf("%d\n",rpacket_read_uint32(r2));
	printf("%d\n",rpacket_read_uint32(r2));
	printf("%d\n",rpacket_read_uint32(r2));
	printf("%d\n",rpacket_read_uint32(r2));
	printf("%s\n",rpacket_read_string(r2));
	printf("%s\n",rpacket_read_string(r2));

	printf("--------------r3---------------\n");
	rpacket *r3 = packet_makeforread(w2);	
	printf("%d\n",rpacket_read_uint32(r3));
	printf("%d\n",rpacket_read_uint32(r3));
	printf("%d\n",rpacket_read_uint32(r3));
	printf("%d\n",rpacket_read_uint32(r3));
	printf("%s\n",rpacket_read_string(r3));
	printf("%s\n",rpacket_read_string(r3));
	printf("%s\n",rpacket_read_string(r3));

	return 0;
}