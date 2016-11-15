/************************************************************************************
sdtimf.c--List time jumps in SD card
06-05-2014
Read SD card from logger of HD file and list time jumps from the time stamps
*************************************************************************************/
/*
Hack of sdpktf.c

// Example--
cd ~/svn_sensor/PC/SD_read_packet
Laptop:

cd ~/svn_sensor/PC/SD-timechk/trunk
make && sudo ./sdtimf /dev/sdb
make && ./sdtimf ~/SDimage/sdX

*/


#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "packet_extract.h"
#include "packet_print.h"
#include "can_write.h"
#include "packet_search.h"
#include "sdlog.h"

char *poutfile = "SDtest.tim";

int fd;
FILE *fpOut;

/********************************************************************************
main
********************************************************************************/
int main (int argc, char **argv)
{
//	FILE *fp;
	

//	int count;
//	int i;
	int j;
	int ipktend;
	int ret;

//	int readct;
	int err;
	unsigned long long tim_prev;
//	int sw_1st = 1;

	int blkmax;
	int blktop;

	int pktdurctr;
	int blkdurctr;
	int enddurctr;
	int jmpctr;

	off64_t o_pos;

	struct LOGBLK blk;
	struct PKTP pktp;
//	struct PKTP* pp;
	struct CANRCVTIMBUF can;

	#define PKTMAX	128
	struct PKTP pktarray[PKTMAX];

	int zerosw = 0;


	printf ("'sdtimf.c' - 06-06-2014\n");
	if (argc != 2)
	{ 
		printf("Need sd card device: %u, e.g. /dev/sdf\n",argc);	return -1;
	}
	fd = open( *(argv+1), O_RDONLY);
	if (fd < 0) 
	{
		printf ("\nOPEN FAIL: %u %s %d\n",argc,*(argv+1),fd);
		return -1;
	}

	/* Find end of SD card */
	o_pos = lseek64 (fd,(off64_t)0,SEEK_END);
	blkmax = o_pos/512;	// Number of blocks on the SD card
	printf("Input file opened: %s, SEEK_END returned (blocks): %d  SDLOG_EXTRA_BLOCKS: %d\n",*(argv+1),blkmax, SDLOG_EXTRA_BLOCKS);
	blktop = (blkmax - 1 - SDLOG_EXTRA_BLOCKS); // Block index of the last loggable block

	if ( (fpOut = fopen (poutfile,"w")) == NULL)
	{
		printf ("\nOutput file did not open\n"); return -1;
	}	

	blk.blk = 0;	// We start with block zero, of course.
	blkdurctr = 0;	// Number of blocks between time jumps
	pktdurctr = 0;	// Number of packets between time jumps
	enddurctr = 0;	// Progress ticks at end when reading zero
	jmpctr = 0;	// Running count of number of time jumps

	/* Go through the whole SD card (well, at least just to where the logging ends). */
	while (blk.blk <= blktop)
	{
		ret = get1stpkt(fd, &blk, &pktp, &can);	// Get first packet from block
		blkdurctr += 1;

	
		/* Get the pointer and size for each packet within the block (in reverse time order) */
			switch (ret)
			{
			case 1: // All zero
				if ((zerosw != 0) && (++enddurctr <= (1000000 - 1)))break;
				enddurctr = 0;
				printf("All zero block: block index: %9d %08X \n",blk.blk, blk.blk);
				zerosw = 1; tim_prev = -1;
				break;
						
			case -1: // Packet size too big or too small
				printf("Packet size too big or too small: err: %d block index: %d %X PID: %lld \n",err, blk.blk, blk.blk, blk.pid);
				zerosw = 0; tim_prev = -1;
				break;
	
			case 0: // Just fine
				zerosw = 0; ipktend = 0;
				/* Get the pointer and size for each packet within the block (in reverse time order) */
				ipktend = 0; pktarray[ipktend] = pktp; ipktend += 1;
				while (pktp.p > &blk.data[0])
				{
					err = packet_extract_next(&pktp);
					if (err != 0)
					{
						printf("I don't think this should happen: err: %d block index: %d %X PID: %lld \n",err, blk.blk, blk.blk, blk.pid);
					}
					else
					{
						pktarray[ipktend] = pktp; ipktend += 1; if (ipktend > PKTMAX) 
						{ipktend = PKTMAX; printf("Too many packets in one block for the array size\n"); exit(0);}				
					}
				}
					
					/* Go through packets in forward time order */
//printf(" ipktend: %d\n",ipktend);
					for (j = (ipktend - 1); j >= 0; j--)
					{
						packet_convert(&can, &pktarray[j]);	// Move binary SD data into CAN struct
						if (tim_prev != can.U.ull)	// Time stamp change?
						{ // Yes.
							tim_prev += 1;	// We expect a change to be one tick higher
							if (tim_prev != can.U.ull) // Was it one tick higher?
							{ // Time jump
								jmpctr += 1;
								printf ("%3i %8i %9i ",jmpctr, blkdurctr, pktdurctr);
								printf("%9i ",blk.blk);
								printf("%08x %08x %08x %08x ",can.R.id,can.R.dlc,can.R.cd.ui[0],can.R.cd.ui[1]);
								packet_print_date1(pktarray[j].p);
								printf (" %lld %lld",tim_prev,can.U.ull);
								printf(" %lld \n",blk.pid);\
								tim_prev = can.U.ull;
								pktdurctr = 0; blkdurctr = 0;
							}
						}
//printf ("X %2i %8i %9i \n",j, blkdurctr, pktdurctr);
						pktdurctr += 1;
//						packet_print(&pktarray[j]);
					}

				break;
		}
//if (blkdurctr > 10000) 
//return -1;
		blk.blk += 1;
	}
	printf("END:            block index: %9d %08X  PID: %lld",blk.blk, blk.blk,blk.pid);
	return 0;
}

