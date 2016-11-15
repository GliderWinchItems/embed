#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include "sdcard.h"

int fd;
int byte_size;

int sdcard_init(char cid_buf[SDCARD_CID_SIZE], char csd_buf[SDCARD_CSD_SIZE])
{
	char buf[SDCARD_BLOCKSIZE];
	int i;
	
	byte_size = lseek(fd, 0, SEEK_END);
	
	printf("# sdcard_csd_memory_size returns %d\n", sdcard_csd_memory_size());

	if(byte_size != (sdcard_csd_memory_size(0) * SDCARD_BLOCKSIZE))
	{
		printf("## Creating simulation file of %d sectors\n",
				sdcard_csd_memory_size(0));
				
		memset(buf, '\0', SDCARD_BLOCKSIZE);
				
		for(i=0; i<sdcard_csd_memory_size(0); i+=1)
			write(fd, buf, SDCARD_BLOCKSIZE);
	}
	
	return 0;
}

int sdcard_read(int blocknum, char buf[SDCARD_BLOCKSIZE])
{
	lseek(fd, blocknum * SDCARD_BLOCKSIZE, SEEK_SET);
	read(fd, buf, SDCARD_BLOCKSIZE);
	return 0;
}

int sdcard_write(int blocknum, char buf[SDCARD_BLOCKSIZE])
{
	lseek(fd, blocknum * SDCARD_BLOCKSIZE, SEEK_SET);
	write(fd, buf, SDCARD_BLOCKSIZE);
	return 0;
}

int sdcard_csd_memory_size(char csd_buf[SDCARD_CSD_SIZE])
{
	return byte_size / SDCARD_BLOCKSIZE;
}

