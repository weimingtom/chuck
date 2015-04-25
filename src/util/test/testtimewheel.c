#include <stdio.h>
#include "util/time.h"
#include "util/timewheel.h"

int count = 0;

int32_t callback1(uint64_t _1,void *_2){
	printf("%s,%lld\n",(const char *)_2,systick64() - _1);
	return 0;
}

uint64_t i;

int32_t callback2(uint64_t _1,void *_2){
	//if((i - _1))
	if(count == 35999888){
		printf("here\n");
	}
	printf("%s,%lld,%d\n",(const char *)_2,i - _1,count);
	return 0;
}


void test1(){
	wheelmgr *m = wheelmgr_new();
	wheelmgr_register(m,systick64(),50,callback1,"ms");	
	uint64_t begin = systick64();
	while(1){
		wheelmgr_tick(m,systick64());
		if(systick64() - begin > 1000)
			break;
		//SLEEPMS(0);
	}
	printf("%lld,%d\n",systick64() - begin,count);	
}

void test2(){
	
	wheelmgr *m = wheelmgr_new();
	i = systick64();	
	//wheelmgr_register(m,i,3*60*1000,callback2,"min");
	wheelmgr_register(m,i,1112,callback2,"min");
	//wheelmgr_register(m,i,1000,callback2,"hour");
	while(1){
		++count;
		wheelmgr_tick(m,++i);//systick64());
		//SLEEPMS(0);
	}	
}


int main(){
	//test1();
	test2();
	return 0;
}