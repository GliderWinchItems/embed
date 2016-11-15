#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sdcardio.h"
#include "sdcard_utils.h"

void block_dump(sdcard_block_t *p, int32_t blocknum)
{
	int i, j;
	
	printf("** blockno: %d\n", blocknum);

	for(i=0; i<16; i+=1)
	{
		printf("%3d> ", i*32);
		for(j=0; j<32; j+=1)
			printf("%02x ", p->int8[i*32+j] & 0xff);
		printf("\n");
	}
	
	printf("** last pid: %lld\n", p->blk.pid);
	printf("\n");
}

int block_isze(sdcard_block_t *b)
{
	int32_t *p = &b->int32[0];
	int32_t *q = &b->int32[SD_BLOCKSIZE/sizeof(int32_t)];

	while(p < q)
		if(*--q != 0)
			return 1==0;
	
	return 1==1;
}

char *block_isok(sdcard_block_t *b, sdcard_isok_t *k)
{
	#define	RETURN(msg) do { return k->err_msg = (msg); } while(1==0)
	int64_t pid_delta;
	char *c;
	
	if(!k->inited)
	{
		k->inited = (1==1);
		k->this_pid = -1;
	}
	
	k->last_pid = k->this_pid;
	k->this_pid = b->blk.pid;

	k->no_packets = 0;

	pid_delta = k->this_pid - k->last_pid;
	
	if(pid_delta == 0)
		RETURN("pid delta zero");
		
	if(pid_delta < 0)
		RETURN("pid delta negative");
	
	if(pid_delta > 250)
		RETURN("pid delta too big");
		
	c = &b->blk.data[SD_BLOCKSIZE-sizeof(int64_t)];
	
	while(c > &b->blk.data[0])
	{
		int size;
		
		if(*--c == 0)
			continue;
		else
			c++;

		k->no_packets += 1;
		
		size = (int)*--c & 0xff;

		if(size < 1)
			RETURN("packet size too small");

		if(size > 250)
			RETURN("packet size too big");
			
		c -= size;
	}
	
	if(c != &b->blk.data[0])
		RETURN("packet boundary error");

	if(k->no_packets == 0)
		RETURN("block with no packets");
	
	RETURN(NULL);
}


int32_t block_cksum(sdcard_block_t *b)
{
	int32_t *p = &b->int32[0];
	int32_t *q = &b->int32[SD_BLOCKSIZE/sizeof(int32_t)];
	int64_t acc = 0;
	int64_t mask32 = 0x00000000ffffffff;
	
	while(p < q)
		acc += (int64_t)*p++ & mask32;
		
	return (int32_t)((acc & mask32) + ((acc >> 32) & mask32));
}
