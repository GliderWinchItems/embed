/************************************************************************************
sdpktf.c--SD card packet reader to file
05-18-2014
Read SD card from logger and output ascii/hex lines (which can be piped to a file)
*************************************************************************************/
/*
Hack of sdpktrdr to implement date/time search and output ascii/hex lines

// Example--
cd ~/svn_sensor/PC/SD_read_packet
Laptop:

cd ~/svn_sensor/PC/SD-read/trunk
make && sudo ./sdpktf /dev/sdb
make && sudo ./sdpktf ~/SDimageSD4

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



#define BLOCKSIZE  512
unsigned char BlockBuff[BLOCKSIZE];	// Current sd block
struct PKTP pktp;			// Current pid, packet size, packet pointer
struct PKTP pktp1;			// Current pid, packet size, packet pointer
#define PKTMAX	128
struct PKTP pktarray[PKTMAX];

int blknum;
int blkmax;
off64_t o_pos;
off64_t o_end;
off64_t o_blk;
int readablock_err;
int fd;
FILE *fpOut;
/********************************************************************************
main
********************************************************************************/
int main (int argc, char **argv)
{
//	FILE *fp;
	

	int count;
	int i;
	int j;
	int ipktend;

	int readct;
	int err;
	unsigned long long pid_prev;
	int sw_1st = 1;

	printf ("'sdpktf.c' - 05-18-2014\n");
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

	/* Start date/time */
	int opt = 0; int yy; int mm; int dd; int hh; int nn; int ss;
	while (opt == 0)
	{
		printf("Note: the hour you enter should be one lower if daylight time\n");
		printf("Enter yy mm dd hh nn (year (00 - 99) month (1-12)  day of month (1 - 31)  hour (0 - 23) minute (0 - 59) second (0 - 59)\nyy mm dd hh mm ss\n");
		scanf("%u %u %u %u %u %u",&yy, &mm, &dd, &hh, &nn, &ss);
		if ((yy < 0) || (yy > 99)) {printf("year    out of range: %u\n",yy); continue;}
		if ((mm < 1) || (mm > 12)) {printf("month   out of range: %u\n",mm); continue;}
		if ((dd < 1) || (dd > 31)) {printf("day     out of range: %u\n",dd); continue;}
		if ((hh < 0) || (hh > 23)) {printf("hour    out of range: %u\n",hh); continue;}
		if ((nn < 0) || (nn > 59)) {printf("minute  out of range: %u\n",nn); continue;}
		if ((ss < 0) || (ss > 59)) {printf("second  out of range: %u\n",ss); continue;}
		opt = 1;
	}
	
	struct tm t;
	time_t start_t;

	/* Fill tm struct with values extracted from gps */
	t.tm_sec =	 ss;
	t.tm_min = 	 nn;
	t.tm_hour =	 hh;
	t.tm_mday =	 dd;
	t.tm_mon =	 mm-1;
	t.tm_year =	 yy + (2000 - 1900);
	t.tm_isdst = 0;		// Be sure daylight time is off
	start_t =  mktime (&t);	// Convert to time_t format

	printf("Search for start date/time: %s",ctime (&start_t));
	
	/* Find end of SD card */
	o_pos = lseek64(fd,(off64_t)0,SEEK_END);
	o_end = o_pos;		// Save end of SD position
	blkmax = o_pos/512;	// Number of blocks on SD card
	printf("Success: %u %s, SEEK_END returned (blocks) %d \n",argc,*(argv+1),blkmax);

	if ( (fpOut = fopen ("SDtest.can","w")) == NULL)
	{
		printf ("\nOutput file did not open\n"); return -1;
	}	

	int highest_nonzero_blk;
	int highest_pid;
	unsigned long long pidx,pidE,pid0;
	int blk_rotate;	// Number to rotate blk index for binary search of date/time

	// NOTE: 'getaPID' returns (PID + 1), so PID = 0 conveys an error.
	pidE = getaPID(fd, (blkmax - 1 - SDLOG_EXTRA_BLOCKS)); // Last block logging writes
	pid0 = getaPID(fd, 0);
	if ((pidE == 0) || (pid0 == 0)) {printf("Error reading SD card\n"); return -1;}
	if (pidE == 1)
	{ // Here high end PID is zero.  Non-wrap situation
		if (pid0 == 1)
		{ // Here.  Beginning & End blocks PIDs are zero
			printf("NO DATA ON CARD: Beginning and End PIDs are both zero\n"); 
//			return -1;
		}
	}

	highest_nonzero_blk = search_zero_end(fd, blkmax);
 	printf ("Search zero result: highest_nonzero_blk: %d end: %d incr: %d odd: %d \n\n",highest_nonzero_blk, (blkmax - SDLOG_EXTRA_BLOCKS), ((blkmax-1)>>1) + ((blkmax-1)&1),(blkmax & 1));


	highest_pid = search_highest_pid (fd, blkmax);
	if (highest_pid == 0)
	{
		printf("Error finding the highest PID\n");
	}
	/* Number of blocks to make the block number (index) of the block with the highest PID
           become the last block of the SD card */
	blk_rotate = ((blkmax - SDLOG_EXTRA_BLOCKS) - highest_pid); 
	pidx = getaPID(fd, highest_pid);
printf ("highest_pid (blk #): %d rotate: %d pid: %llX\n",highest_pid, blk_rotate,(pidx-1));
	
	/* See if requested time is between the lowest and highest times on the SD card */
	struct LOGBLK logblk;
	time_t t_low;
	logblk.blk = highest_pid;
	time_t t_high = get1stpkttime(fd, &logblk);
	printf ("\nThe highest and lowest times found on this SD card---\n");
	printf ("Highest: block#:%9d SD time: %s",logblk.blk, ctime (&t_high));
	pidx = getaPID(fd, (blkmax - 1 - SDLOG_EXTRA_BLOCKS));
	if (pidx > 1)
	{ // Here a wraparound situation
		highest_pid += 1; if (highest_pid >= (blkmax - SDLOG_EXTRA_BLOCKS)) highest_pid -= (blkmax - SDLOG_EXTRA_BLOCKS);
		logblk.blk = highest_pid;
		printf("SD card data has wrapped around\n");
	}
	else
	{ // Here, SD not yet filled
		logblk.blk = 0;
	}
	t_low = get1stpkttime(fd, &logblk);
	printf ("Lowest : block#:%9d SD time: %s", logblk.blk, ctime (&t_low));
	
	if (start_t < t_low)
	{
		printf ("Requested time is less than SD lowest time\n"); //return -1;
	}
	if (start_t > t_high)
	{
		printf ("Requested time is greater than SD highest time\n");// return -2;
	}
	printf("Requested time: %s\n",ctime(&start_t));

	
//	struct SEARCHRESULT search_datetime (int fd, time_t datetime, int rotate, int blkmax)
	struct SEARCHRESULT srx;
	srx = search_datetime (fd, start_t, blk_rotate, blkmax);
	/* Show search return code. */
	printf("Date/time search return code: ");
	switch (srx.type)
	{
	case  1: printf("EXACT MATCH\n"); break;
	case  0: printf("OK\n"); break;
	case -1: printf("ERROR getting PID\n"); break;
	case -2: printf("NOT FOUND\n");break;
	}
	
	printf("srx: type %2d blk %8d time_t: %9d %s",srx.type, srx.blk, (int)srx.t, ctime(&srx.t));


	printf("\nOut file opened\n");	

	/* Go for it */
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
					pktp1 = pktp; // Save beginning of last packet in block
					if (sw_1st == 0)
					{ // Here, take care of 1st time (skip difference in pid's)
						printf ("block_number %u  pid: %llu\n",blknum,pktp.pid);
						sw_1st = 1;
					}
//					else
						printf ("block_number %u  pid: %llu prior block packet count %lld\n",blknum,pktp.pid,(pid_prev-pktp.pid));

//					packet_print(&pktp);	// Nice formatted printout.
				
//printf ("##############################################################################################################\n");

					/* Get the pointer and size for each packet within the block (in reverse time order) */
					ipktend = 0; pktarray[ipktend] = pktp; ipktend += 1;
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
							pktarray[ipktend] = pktp; ipktend += 1; if (ipktend > PKTMAX) {ipktend = PKTMAX; printf("ERROR1\n"); exit(0);}
//							packet_print(&pktp);
							
						}
					}
//printf ("Number of packets in this block: %d\n",ipktend);
					/* Print out in forward time order */
					for (j = (ipktend - 1); j >= 0; j--)
					{
						packet_print(&pktarray[j]);
//						can_write(fpOut, &pktarray[j]);
					}
					pid_prev = pktp.pid;
				}
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

