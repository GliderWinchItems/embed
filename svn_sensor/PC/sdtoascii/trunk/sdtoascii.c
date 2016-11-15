/************************************************************************************
sdtoascii.c--SD card packet reader to file
06-20-2014
Read SD card from logger and output ascii/hex lines (which can be piped to a file)
*************************************************************************************/
/*
Hack of ../../SD-read/trunk/sdpkf.c to implement date/time search and output ascii/hex lines

// Example--
cd ~/svn_sensor/PC/sdtoascii/trunk
make && sudo ./sdtoascii  [default to /dev/sdb]
make && sudo ./sdtoascii /dev/sdb
make && sudo ./sdtoascii ~/SDimageSD4
make && sudo ./sdtoascii -[options] <path/file>

options:
-g gateway format
-t time search

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

void printhelp(void);
void progressprint(int *ct);

char *pfiledefault = "/dev/sdb"; // Default input: SD card

#define BLOCKSIZE  512
unsigned char BlockBuff[BLOCKSIZE];	// Current sd block
struct PKTP pktp;			// Current pid, packet size, packet pointer
struct PKTP pktp1;			// Current pid, packet size, packet pointer
#define PKTMAX	128
struct PKTP pktarray[PKTMAX];

int blknum;
int blkmax;
int blkend;

off64_t o_pos;
off64_t o_end;
off64_t o_blk;

int readablock_err;
int fd1;
int fd2;

/* Command line switches */
#define SWCT	5	// Number of switches to search
#define SW_G	sw[0]	// Gateway output format
#define SW_T	sw[1]	// Specify start end date/times
#define SW_B	sw[2]	// Specify start block number and count
#define SW_P	sw[3]	// Print to terminal in block format
#define SW_H	sw[4]	// Help
char cmd[SWCT] = {'g','t','b','p','h'};
int   sw[SWCT];		// 0 = switch is off, 1 = switch is on

/********************************************************************************
 * int looklikeaswitch(char *pfile, char *p, int *sw);
 * @brief	: Does this string look like a command line switch?
 * @param	: pointer to file name
 * @param	: pointer to argument
 * @param	: pointer to sw array
 * @return	:  1 = OK
 *		:  0 = first char not '-' so is must not be a switch
 *		: -1 = first char not '-' and next char not 't', 'g', 'b', etc.
 *		: -2 = help
********************************************************************************/
int looklikeaswitch(char *pfile, char *p, int *sw)
{
	int k,m,n;
	int any = 0;

	if (*p == '-')
	{ // First char of an argument means it is a switch
		k = strlen(p);
		for (m = 1; m < k; m++)
		{
			any = 0;
			for (n = 0; n < SWCT; n++)
			{
				if ( *(p+m) == cmd[n] )
				{ 
					*(sw+n) = 1; any += 1;
				}
			}
			if (any == 0)
			{
				printf("switch option not found: %c %s\n",*(p+m),p);
				printhelp();
				return -1;
			}
		}
		if (SW_H == 1) {printhelp(); return -2;}
		return 1;
	}
	strncpy (pfile,p, 256);
	return 0;	// 1st char not '-', so it is not a switch
}
/********************************************************************************
main
********************************************************************************/
int main (int argc, char **argv)
{
	int progress = 0;

	char filename[256];	// Input file path/name

	struct LOGBLK logblk;

	char vv[192];
	char ww[256];
	char *pc;

	struct tm t;
	time_t start_t;
	time_t t_low;
	time_t t_high;


	FILE *fpOut;

	int highest_nonzero_blk;
	int highest_pid;
	unsigned long long pidx,pidE,pid0;

	int count;
	int i;
	int j;
	int ipktend;

	int readct;
	int err;
	unsigned long long pid_prev;
	int sw_1st = 1;

	printf ("\n'sdtoascii.c' - 06-21-2014 (summer solstice)\n\n");
	strcpy(filename, pfiledefault);	// Setup default input file name
	if (argc > 4)
	{
		printf("Too many arguments: %d\n", argc); return -1;
	}
	if (argc > 1)
	{ 
		for (i = 1; i < argc; i++)
		{
			if ((j=looklikeaswitch(filename, *(argv+i), sw)) < 0) return -1;
		}
	}

	printf("Input file name: %s\n",filename);
	/* Open the input file which is likely to be the SD card. */
	fd1 = open( filename, O_RDONLY);
	if (fd1 < 0) 
	{
		printf ("\nOPEN FAIL: %u %s %d\n",argc,filename,fd1);
		return -1;
	}

	/* Command line switch: Enter block start and count */
	if (SW_B == 1)
	{ 
		printf("\nEnter: block_number  and count: \n");
		scanf("%u %u",&blknum,&count);		
		printf ("start at block %u, count %u\n",blknum,count);
		blkend = (blknum + count); // End and start block numbers
	}

	/* Command line switch: Enter start date/tiome */
	if (SW_T == 1)
	{ /* Get Start date/time */
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
	}
	
	/* Find end of SD card */
	o_pos = lseek64(fd1,(off64_t)0,SEEK_END);
	o_end = o_pos;		// Save end of SD position
	blkmax = o_pos/512;	// Number of blocks on SD card
	printf("Success: %u %s, SEEK_END returned (blocks) %d \n",argc,*(argv+1),blkmax);



	int blk_rotate;	// Number to rotate blk index for binary search of date/time

	// NOTE: 'getaPID' returns (PID + 1), so PID = 0 conveys an error.
	pidE = getaPID(fd1, (blkmax - 1 - SDLOG_EXTRA_BLOCKS)); // Last block logging writes
	pid0 = getaPID(fd1, 0);
	if ((pidE == 0) || (pid0 == 0)) {printf("Error reading SD card: returned the PID block[0] = %llu block[%u] = %llu\n",pid0,(blkmax - 1 - SDLOG_EXTRA_BLOCKS),pidE); return -1;}
	if (pidE == 1)
	{ // Here high end PID is zero.  Non-wrap situation
		if (pid0 == 1)
		{ // Here.  Beginning & End blocks PIDs are zero
			printf("NO DATA ON CARD: Beginning and End PIDs are both zero\n"); 
//			return -1;
		}
	}

	highest_nonzero_blk = search_zero_end(fd1, blkmax);
 	printf ("Search zero result: highest_nonzero_blk: %d end: %d incr: %d odd: %d \n\n",highest_nonzero_blk, (blkmax - SDLOG_EXTRA_BLOCKS), ((blkmax-1)>>1) + ((blkmax-1)&1),(blkmax & 1));


	highest_pid = search_highest_pid (fd1, blkmax);
	if (highest_pid == 0)
	{
		printf("Error finding the highest PID\n");
	}
	/* Number of blocks to make the block number (index) of the block with the highest PID
           become the last block of the SD card */
	blk_rotate = ((blkmax - SDLOG_EXTRA_BLOCKS) - highest_pid); 
	pidx = getaPID(fd1, highest_pid);
printf ("highest_pid (blk #): %d rotate: %d pid: %llX\n",highest_pid, blk_rotate,(pidx-1));
	
	if (SW_T == 1)
	{
		/* See if requested time is between the lowest and highest times on the SD card */
		logblk.blk = highest_pid;
		t_high = get1stpkttime(fd1, &logblk);
		printf ("\nThe highest and lowest times found on this SD card---\n");
		printf ("Highest: block#:%9d SD time: %s",logblk.blk, ctime (&t_high));
		pidx = getaPID(fd1, (blkmax - 1 - SDLOG_EXTRA_BLOCKS));
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


		t_low = get1stpkttime(fd1, &logblk);
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

		struct SEARCHRESULT srx;
		srx = search_datetime (fd1, start_t, blk_rotate, blkmax);
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
	}
	

	logblk.blk = highest_pid;
	t_high = get1stpkttime(fd1, &logblk);
	printf ("Highest: block#:%9d SD time: %s", logblk.blk, ctime(&t_high));
	logblk.blk = 0;
	t_low = get1stpkttime(fd1, &logblk);
	printf ("Lowest : block#:%9d SD time: %s", logblk.blk, ctime(&t_low));

	strcpy(vv,ctime(&t_high));
	pc = strchr(vv,'\n');	// Remove '\n'
	if (pc != NULL) *pc = 0;
	while ( (pc=strchr(vv,':')) != NULL) *pc = '.';
	while ( (pc=strchr(vv,' ')) != NULL) *pc = '_';

	/* Add extension to delineate name from the one that would be used in sdcopy. */
	if (SW_G == 1)
		strcat(vv,".GAT");
	else
		strcat(vv,".WCH");	

	strcpy(ww,"../../../../winch/instrumentation/");
	strncat(ww,vv,192);
	strcpy(vv,ww);

	if ( (fpOut = fopen (vv,"w")) == NULL)
	{
		printf ("\nOutput file did not open: %s\n",vv); return -1;
	}
	
	printf("\nOUTPUT FILE NAME: ====>%s<====\n\n", vv);	

	/* Start and end blocks for when the whole card is being converted. */
	if ((SW_T == 0) && (SW_B == 0)) {blkend = highest_nonzero_blk; blknum = 0;}

	printf("Converting: start block = %d  through block = %d\n",blknum, blkend);
	
/* ----------------------------------------------------------------------------------------------------- */
	while (blknum < blkend)
	{

		sw_1st = 0;	// Previous block block packet ct 1st time switch
		{ // Here, it is OK.
			{
				o_blk = blknum;	o_blk = o_blk << 9;
				o_pos = lseek64(fd1,o_blk,SEEK_SET);	// Byte offset seek
				if (o_pos < 0)	// In case something went awry
				{ // Here, lseek returned an error.
					printf("seek error: %lld for block: %u, %llu\n",(unsigned long long)o_pos, blknum, (unsigned long long)o_blk/512); continue;
				}

				/* Read the block */
				readct = read (fd1, &BlockBuff[0], 512);
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
						if (SW_P == 1)printf("Block %u is all zeroes\n",blknum);
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
						if (SW_P == 1) printf ("block_number %u  pid: %llu\n",blknum,pktp.pid);
						sw_1st = 1;
					}
//					else
						if (SW_P == 1) printf ("block_number %u  pid: %llu prior block packet count %lld\n",blknum,pktp.pid,(pid_prev-pktp.pid));

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
								if (SW_P == 1) printf("Block %u is all zeroes\n",blknum);
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
						if (SW_P == 1)
							packet_print(&pktarray[j]);
						if (SW_G == 1) // Gateway versus winch format
							{can_write(fpOut, &pktarray[j]); progressprint(&progress);}
						else
							{can_write2(fpOut, &pktarray[j]);progressprint(&progress);}
					}
					pid_prev = pktp.pid;
				}
				blknum += 1;
			}
		}
	}
	printf("\n=== DONE ====\n");
	return 0;
}
/********************************************************************************
 * void printhelp(void);
 * @brief	: Command line switch -h
********************************************************************************/
void printhelp(void)
{
	printf("HELP: usage: sdtoascii [options] [path/file for input]\noptions:\n");
	printf(" -g : gateway output format (default - winch (.WCH) format)\n");
	printf(" -t : Specify start:end date/times(not implemented)\n");
	printf(" -b : Specify start block number and count\n");
	printf(" -p : Print old block format (primitive debugging)\n");
	printf(" -h : This help listing\n");

	printf(" Example\nsudo ./sdtoascii -g /dev/sdf [gateway format] [input from SD card on /dev/sdf]\n");
	return;
}
/********************************************************************************
 * void progressprint(int *ct);
 * @brief	: Progress
********************************************************************************/
static int ct_prev = 10000;
void progressprint(int *pct)
{
	*pct += 1;
	if (*pct >= ct_prev)
	{
		printf("progress: packetcount %9d\r",*pct);
		ct_prev = *pct + 10000;
	}
	return;
}

