#ifndef  _BIT_H
#define _BIT_H

#include <stdint.h>
#include <stdio.h>

typedef struct
{
	uint32_t size;
	uint32_t bits[];
}bitset;


static inline bitset* bitset_new(uint32_t size)
{
	uint32_t _size = size % sizeof(uint32_t) == 0 ? size/sizeof(uint32_t):size/sizeof(uint32_t)+1;
	bitset *bs = calloc(1,sizeof(*bs)+sizeof(uint32_t)*_size);
	bs->size = size;
	return bs;
}

static inline void bitset_del(bitset *bs)
{
	free(bs);
}

static inline void bitset_set(bitset *bs,uint32_t index)
{
	if(index <= bs->size){
		uint32_t b_index = index / (sizeof(uint32_t)*8);
		index %= (sizeof(uint32_t)*8);
		bs->bits[b_index] = bs->bits[b_index] | (1 << index);
	}
}

static inline void bitset_clear(bitset *bs,uint32_t index)
{
	if(index <= bs->size){
		uint32_t b_index = index / (sizeof(uint32_t)*8);
		index %= (sizeof(uint32_t)*8);
		bs->bits[b_index] = bs->bits[b_index] & (~(1 << index));
	}
}

static inline int32_t bitset_test(bitset *bs,uint32_t index)
{
	if(index <= bs->size){
		uint32_t b_index = index / (sizeof(uint32_t)*8);
		index %= (sizeof(uint32_t)*8);
		return bs->bits[b_index] & (1 << index)?1:0;
	}else
		return 0;
}

static inline bitset_show(void *ptr,uint32_t size){
	bitset *b = bitset_new(size);
	memcpy(b->bits,ptr,size);
	int i = b->size-1;
	for(; i >=0; --i){
		printf("%d",bitset_test(b,i)?1:0);
	} 
	printf("\n");
	bitset_del(b);	
}

#endif
