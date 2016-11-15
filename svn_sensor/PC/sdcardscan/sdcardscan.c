#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

int sd_open(char *fn);
void sd_close(void);
int sd_read(int blocknum, void *buf);
int sd_write(int blocknum, void *buf);

void buf_fill(int blocknum, void *buf, int pattern_id);
int buf_test(int blocknum, void *buf);

uint8_t buf[512];

int main(int argc, char *argv[])
{
	int size;
	int argv2;
	int i, j;

	if(argc != 3)
		return 100;
		
	size = sd_open("./sdcard.dat");
	
	argv2 = atoi(argv[2]);
	if(argv2 < 0  ||  argv2 > size)
		return 101;

	printf("%d blocks\n", size);

	switch(*argv[1])
	{
	case 'a':
		for(i=0; i<argv2; i+=1)
		{
			buf_fill(i, buf, 2);
			sd_write(i, buf);
		}
		break;
		
	case 'b':
		for(i=0; i<argv2; i+=1)
		{
			sd_read(i, buf);
			j = buf_test(i, buf);
			if(j != 2)
				printf("%d %d/", i, j);
		}
		break;		
	}

	sd_close();

	printf("Huzzah!\n");
	return 0;
}

/**************************************************************************/
static const uint32_t rand_seed = 0xdeadbeef;	/* My favorite 32-bit hex constant */
static const uint32_t rand_incr = 982451653;	/* The 50-million-th Mersenne prime */

void buf_fill(int blocknum, void *buf, int pattern_id)
{
	uint32_t *b = (uint32_t *)buf + 0;
	uint32_t *e = (uint32_t *)buf + (512/4);
	uint32_t x = rand_seed + (blocknum * 512) * rand_incr;
	
	switch(pattern_id)
	{
	case 0:
		while(b < e)
			*b++ = 0;
		break;

	case 1:
		while(b < e)
			*b++ = ~0;
		break;
		
	case 2:
		while(b < e)
		{
			*b++ = x;
			x += rand_seed;
		}
		break;
			
	case 3:
		while(b < e)
		{
			*b++ = ~x;
			x += rand_seed;
		}
		break;
	}
}

int buf_test(int blocknum, void *buf)
{
	uint32_t *b = (uint32_t *)buf + 0;
	uint32_t *e = (uint32_t *)buf + (512/4);
	uint32_t x = rand_seed + (blocknum * (512/4)) * rand_incr;
	uint32_t y = *b;
	
	if(y == 0)
	{
		while(b < e)
			if(*b++ != 0)
				return ~0xf + 0;
		return 0;
	}
	else if(y == ~0)
	{
		while(b < e)
			if(*b++ != ~0)
				return ~0xf + 1;
		return 1;
	}
	else if(y == x)
	{
		while(b < e)
		{
			if(*b++ != x)
				return ~0xf + 2;
			x += rand_incr;
		}
		return 2;
	}
	else if(y == ~x)
	{
		while(b < e)
		{
			if(*b++ != ~x)
				return ~0xf + 3;
			x += rand_incr;
		}
		return 3;
	}
	
	return ~0xf + 4;
}

/**************************************************************************/
int fd;

int sd_open(char *fn)
{
	off_t tmp;
	
	fd = open(fn, O_RDWR + O_CREAT);
	if(fd < 0)
	{
		printf("open err is %d\n", fd);
		exit(1);
	}
	
	tmp = lseek(fd, 0, SEEK_END);
	if(tmp < 0)
	{
		printf("open/lseek err is %d\n", (int)tmp);
		exit(2);
	}
	
	return (int)(tmp / 512);
}

void sd_close(void)
{
	int err = close(fd);
	
	if(err < 0)
	{
		printf("close err is %d\n", err);
		exit(3);
	}
}

int sd_read(int blocknum, void *buf)
{
	off_t tmp0, tmp1;
	int i;
	
	tmp0 = (off_t)blocknum * 512;
	tmp1 = lseek(fd, tmp0, SEEK_SET);
	if(tmp1 != tmp0)
	{
		printf("read/lseek err is %d\n", (int)tmp1);
		exit(4);
	}
		
	i = read(fd, buf, 512);
	if(i != 512)
	{
		printf("read err is %d\n", i);
		exit(5);
	}
	
	return 0;
}

int sd_write(int blocknum, void *buf)
{
	off_t tmp0, tmp1;
	int i;
	
	tmp0 = (off_t)blocknum * 512;
	tmp1 = lseek(fd, tmp0, SEEK_SET);
	if(tmp1 != tmp0)
	{
		printf("write/lseek err is %d\n", (int)tmp1);
		exit(4);
	}
		
	i = write(fd, buf, 512);
	if(i != 512)
	{
		printf("write err is %d\n", i);
		exit(5);
	}
	
	return 0;
}

