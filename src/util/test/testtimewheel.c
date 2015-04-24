#include <stdio.h>
#include "util/time.h"
#include "util/timewheel.h"

int count = 0;

int32_t callback(uint64_t _1,void *_2){
	printf("%s,%lld\n",(const char *)_2,systick64() - _1);
	++count;
	return 0;
}


void test1(){
	wheelmgr *m = wheelmgr_new();
	wheelmgr_register(m,50,callback,"ms");	
	uint64_t begin = systick64();
	while(1){
		wheelmgr_tick(m,systick64());
		if(systick64() - begin > 1000)
			break;
		SLEEPMS(100);
	}
	printf("%lld,%d\n",systick64() - begin,count);	
}

void test2(){
	wheelmgr *m = wheelmgr_new();
	wheelmgr_register(m,60*1000,callback,"min");
	wheelmgr_register(m,3600*1000,callback,"hour");
	while(1){
		wheelmgr_tick(m,systick64());
		SLEEPMS(0);
	}	
}


int main(){
	test1();
	test2();
	return 0;
}