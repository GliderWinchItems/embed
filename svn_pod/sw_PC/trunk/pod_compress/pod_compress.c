#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <strings.h>
#include <time.h>
#include "zlib.h"

#define	MYSIZE	512
char ibuf[MYSIZE];
char obuf[MYSIZE];

voidpf my_alloc(voidpf opaque, uInt items, uInt size);
uLong alloc_so_far(void);

main(int argc, char *argv[])
{
	z_stream *zp;
	FILE *in;
	int deflate_level;
	int i;
	clock_t t0, t1;
	uLong m0, m1;
	
	if(argc != 2)
		exit(1);

	for(deflate_level=1; deflate_level<=9; deflate_level+=1)
	{
		in = fopen(argv[1], "rb");
		if(!in)
			exit(2);
	
		zp = calloc(1, sizeof(z_stream));
		zp->zalloc = my_alloc;
		
		t0 = clock();
		m0 = alloc_so_far();
		deflateInit(zp, deflate_level);
		zp->avail_in = 0;
		
		zp->next_out = obuf;
		zp->avail_out = sizeof(obuf);
		
		for(i=0; ; i+=1)
		{
			if(zp->avail_in == 0)
			{
				zp->next_in = ibuf;
				zp->avail_in = fread(ibuf, 1, sizeof(ibuf), in);
				if(zp->avail_in < sizeof(ibuf))
					break;
			}
			
			deflate(zp, Z_NO_FLUSH);
			
			zp->next_out = obuf;				/* Toss output */
			zp->avail_out = sizeof(obuf);
		}
	
		deflate(zp, Z_FINISH);
		deflateEnd(zp);
		t1 = clock();
		m1 = alloc_so_far();
		free(zp);
		fclose(in);
		
		printf("# total_in is %lu, total_out is %lu,"
		        " deflate_level is %d, ratio is %4.1f%%,"
		        " duration is %6.3f sec., mem is %ld bytes\n",
				zp->total_in, zp->total_out,
				deflate_level, 100.0*(float)zp->total_out/(float)zp->total_in,
				(float)(t1-t0)/CLOCKS_PER_SEC, m1-m0);
	}
}

static uLong so_far = 0;

voidpf my_alloc(voidpf opaque, uInt items, uInt size)
{
	so_far += (uLong)items * (uLong) size;
	return calloc(items, size);
}

uLong alloc_so_far(void)
{
	return so_far;
}
