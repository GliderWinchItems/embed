/******************************************************************************/
#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>

#include "sdcardio.h"

#define	ERRBASE			(-1000)

static int sdcard_fd = 0;
static int32_t size_in_blocks;

int32_t sdcard_open(char *filename)
{
	off64_t size_in_bytes;

	if(sdcard_fd != 0) return ERRBASE-1;

	sdcard_fd = open(filename, O_RDWR);
	
	if(sdcard_fd < 0)
		return sdcard_fd;
		
	size_in_bytes = lseek64(sdcard_fd, (off64_t)0, SEEK_END);
	if(size_in_bytes <= 0)
		return ERRBASE-2;
		
	size_in_blocks = (int32_t)(size_in_bytes / SD_BLOCKSIZE);

	return size_in_blocks;
}

int sdcard_read(int32_t blockno, char *buf)
{
	int rv;
	
	if(sdcard_fd == 0) return ERRBASE-10;

	if(blockno < 0 || blockno >= size_in_blocks || buf == NULL)
		return ERRBASE-11;
	
	rv = lseek64(sdcard_fd, (off64_t)blockno*SD_BLOCKSIZE, SEEK_SET);
	if(rv < 0)
		return ERRBASE-12;
		
	rv = read(sdcard_fd, buf, SD_BLOCKSIZE);
	if(rv < 0) return ERRBASE-13;

	return 0;
}

int sdcard_write(int32_t blockno, char *buf)
{
	int rv;
	
	if(sdcard_fd == 0) return ERRBASE-20;

	if(blockno < 0 || blockno >= size_in_blocks || buf == NULL)
		return ERRBASE-21;
	
	rv = lseek64(sdcard_fd, (off64_t)blockno*SD_BLOCKSIZE, SEEK_SET);
	if(rv < 0)
		return ERRBASE-22;
		
	rv = write(sdcard_fd, buf, SD_BLOCKSIZE);
	if(rv < 0)
		return ERRBASE-23;

	return 0;
}

int sdcard_close(void)
{
	if(sdcard_fd == 0) return ERRBASE-30;

	close(sdcard_fd);
	return 0;
}
