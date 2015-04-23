#include "util/bit.h"

int main(){
	int a = 2;
	int b = 4;
	int c = 128;
	int d = 512;
	int e = 9288;
	int f = 132334143241;
	bitset_show(a,sizeof(a));
	bitset_show(b,sizeof(b));
	bitset_show(c,sizeof(c));
	bitset_show(d,sizeof(d));
	bitset_show(e,sizeof(e));
	bitset_show(f,sizeof(f));
	return 0;
};