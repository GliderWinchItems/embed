/************************************************************************************
sdpktrdr.c--SD card packet reader
06-14-2013
*************************************************************************************/
/*
// Example--
cd ~/svn_sensor/PC/SD_read_packet
Laptop:
gcc -Wall sdpktrdr.c packet_extract.c packet_print.c -o sdpktrdr -lm && sudo ./sdpktrdr /dev/sdb
Desktop:
gcc -Wall sdpktrdr.c packet_extract.c packet_print.c -o sdpktrdr -lm && sudo ./sdpktrdr /dev/sdf
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




#define BLOCKSIZE  512
unsigned char BlockBuff[BLOCKSIZE];	// Current sd block
struct PKTP pktp;			// Current pid, packet size, packet pointer



/********************************************************************************
main
********************************************************************************/
int main (int argc, char **argv)
{
//	FILE *fp;
	int fd;
	int blknum;
	int blkmax;
	int count;
	int i;
	off64_t o_pos;
	off64_t o_blk;
	int readct;
	int err;
	unsigned long long pid_prev;
	int sw_1st = 1;

	printf ("'sdpktrdr.c' - 06-14-2013, now 01-27-2014\n");
	if (argc != 2)
	{ 
		printf("Need sd card device: %u, e.g. /dev/sdf\n",argc);	return -1;
	}
//	fp = fopen( *(argv+1), "rb");
	fd = open( *(argv+1), O_RDONLY);
	if (fd < 0) 
	{
		printf ("\nOPEN FAIL: %u %s %d\n",argc,*(argv+1),fd);
		return -1;
	}

	o_pos = lseek64 (fd,(off64_t)0,SEEK_END);
	blkmax = o_pos/512;
	printf("Success: %u %s, SEEK_END returned (blocks) %d \n",argc,*(argv+1),blkmax);

	while (1==1)
	{
		printf("\nEnter: block_number  and count: \n");
		scanf("%u %u",&blknum,&count);
		
		printf ("start at block %u, count %u\n",blknum,count);
		sw_1st = 0;	// Previous block block packet ct 1st time switch

		if ((count > 0) && (blknum < blkmax) )	// Check for a fat fingers entry.
		{ // Here, it is OK.
			for (i = 0; i < count; i++)	// Grind through the number of blocks requested
			{
				o_blk = blknum;	o_blk = o_blk << 9;
				o_pos = lseek64(fd,o_blk,SEEK_SET);	// Byte offset seek
				if (o_pos < 0)	// In case something went awry
				{ // Here, lseek returned an error.
					printf("seek error: %lld for block: %u, %llu\n",(unsigned long long)o_pos, blknum, (unsigned long long)o_blk/512); continue;
				}

				/* Read the block */
				readct = read (fd, &BlockBuff[0], 512);
				if (readct != 512)
				{ // Here, for some reason we didn't get the length expected from an sd card.
					printf("Read was not 512 bytes.  It was %d\n",readct); continue;
				}

				/* Get first packet in the block */
				err = packet_extract_first(&pktp, &BlockBuff[0]);
				if (err != 0)
				{ // Here, finding a packet failed.
//					packet_print_hex(&BlockBuff[0]);
					if ( sdblk_allzero(&BlockBuff[0]) == 0 )
					{
						printf("Block %u is all zeroes\n",blknum);
					}
					else
					{ // Something other than all zeroes
						packet_changelines(&BlockBuff[0]);	// Print change lines
						printf ("\nPacket err: %d, pid %llu, ct %u\n",err,pktp.pid,pktp.ct);
					}
				}
				else
				{ // Here, it looks OK so far.
					if (sw_1st == 0)
					{ // Here, take care of 1st time (skip difference in pid's)
						printf ("block_number %u  pid: %llu\n",blknum,pktp.pid);
						sw_1st = 1;
					}
					else
						printf ("block_number %u  pid: %llu prior block packet count %lld\n",blknum,pktp.pid,(pid_prev-pktp.pid));

					packet_print(&pktp);	// Nice formatted printout.
				}

				/* Go through the other packets in the block */
				while (pktp.p > &BlockBuff[0])
				{
					err = packet_extract_next(&pktp);
					if (err != 0)
					{
						if ( sdblk_allzero(&BlockBuff[0]) == 0 )
						{
							printf("Block %u is all zeroes\n",blknum);
						}
						else
						{ // Something other than all zeroes
							packet_changelines(&BlockBuff[0]);	// Print change lines
							printf ("\nPacket err: %d, pid %llu, ct %u\n",err,pktp.pid,pktp.ct);
						}
					}
					else
					{
						packet_print(&pktp);
					}
				}
				pid_prev = pktp.pid;
				blknum += 1;
			}
		}
		else
		{ // Here, the count and/or block number are out of bounds, (obviously due to fat fingers).
			printf ("Count must be greater than zero and block number less than %d\n",blkmax);
		}
	}
	return 0;
}

