#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sdcardio.h"
#include "sdcard_utils.h"

sdcard_block_t my_block, mbm1, mbm2, mbm3;
int32_t num_blocks;

sdcard_isok_t my_ok_stuff;

void sdcard_scan(char *card_filename)
{
	int i;
	int print_flag;

	num_blocks = sdcard_open(card_filename);
	
	printf("sdcard == \"%s\", num_blocks == %d\n", card_filename, num_blocks);
	
	for(i=0; i<num_blocks; i+=1)
	{
		memcpy(&mbm3, &mbm2, sizeof(mbm3));
		memcpy(&mbm2, &mbm1, sizeof(mbm2));
		memcpy(&mbm1, &my_block, sizeof(mbm1));
		sdcard_read(i, (char*)&my_block);

		block_isok(&my_block, &my_ok_stuff);

		print_flag = (1==0);
		if(i == 0)						print_flag = (1==1);
		if(i == (num_blocks-1))			print_flag = (1==1);
		if(my_ok_stuff.err_msg != NULL)	print_flag = (1==1);


if(1)	if(print_flag && (my_ok_stuff.last_pid != 0 || my_ok_stuff.this_pid != 0))
		{
			printf("## blk:%8d, last_pid:%9lld, this_pid:%9lld, no_pkts:%3d -- %s\n",
				i, my_ok_stuff.last_pid, my_ok_stuff.this_pid, 
				my_ok_stuff.no_packets, my_ok_stuff.err_msg);
		
if(0)		{
				if(i >= 3)
					block_dump(&mbm3, i-3);
				if(i >= 2)
					block_dump(&mbm2, i-2);
				if(i >= 1)
					block_dump(&mbm1, i-1);
				block_dump(&my_block, i-0);
			}
		}
	}

	printf("## blk:%8d, last_pid:%9lld, this_pid:%9lld, no_pkts:%3d -- %s\n",
		i, my_ok_stuff.last_pid, my_ok_stuff.this_pid, 
		my_ok_stuff.no_packets, my_ok_stuff.err_msg);
}
