/************************************************************************************
sdutil.c--List time jumps in SD card
06-22-2014
SD card utility: zero card, check time gaps, hi/lo times on card.
*************************************************************************************/
/*
Hack of sdtimf.c

// Example--
cd ~/svn_sensor/PC/sdutil/trunk
make && sudo ./sdutil [default /dev/sdb]
make && ./sdutil /dev/sdf [specify input file name]

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

void printmenu(void);
int getcommand(char c);
void check_entire_card_zero(int blkmax);
int readblock(int fd, char* p, int blk);
int readwriteblock(int fd2, int fd1, unsigned char* p, int blk);
void writezeros(int blkmax);

char *poutfile = "SDtest.tim";
char *pfiledefault = "/dev/sdb"; // Default input: SD card

#define CMDCT	5	// Number of command options
char *pcmd[CMDCT] = { \
" t - time gap check\n",\
" w - check if entire card zero\n",\
" y - zero card from block zero up through last data block\n",\
" z - zero card from beginning to end\n",\
" x - cancel operation\n"\
};
#define CMD_T	0	
#define CMD_W	1
#define CMD_Y	2	
#define CMD_Z	3	
#define CMD_X	4	

#define BLOCKSIZE  512
unsigned char BlockBuff[BLOCKSIZE];	// Current sd block
int fd;
int fdin;

/********************************************************************************
main
********************************************************************************/
int main (int argc, char **argv)
{
//	FILE *fp;
	

	int cmdidx;
//	int i;
	int j;
	int ipktend;
	int ret;

	char jc;

//	int readct;
	int err;
	unsigned long long tim_prev;
//	int sw_1st = 1;

	int blkmax;

	int pktdurctr;
	int blkdurctr;
	int enddurctr;
	int jmpctr;

	off64_t o_pos;

	struct LOGBLK logblk;

//	struct tm t;
//	time_t start_t;
	time_t t_low;
	time_t t_high;

	char vv[192];
//	char ww[256];
	char *pc;

	char kb[256];

	int highest_nonzero_blk;
//	int highest_pid;
	unsigned long long pidE,pid0;

	struct LOGBLK blk;
	struct PKTP pktp;
//	struct PKTP* pp;
	struct CANRCVTIMBUF can;
	struct CANRCVTIMBUF can_save;

	#define PKTMAX	128
	struct PKTP pktarray[PKTMAX];

	int zerosw = 0;

	char filename[256];	// Input file path/name

	printf ("\n'sdutil.c' - 06-22-2014  (WOW: one day after the summer solstice!)\n");

	// Setup input file
	strcpy(filename, pfiledefault);	// Setup default input file name
	if (argc > 2){printf("Number arguments on command line: %d  Only one argument allowed, e.g. ./sdutil /dev/sdf\n",argc); return -1;}
	if (argc == 2) strncpy(filename,*(argv+1), 128);
	fd = open(filename, O_RDWR|O_CREAT, 0444);
	if (fd < 0)  {printf ("\nOPEN FAIL (did you put 'sudo' in front of ./sdutil?): %u :%s: %d\n",argc,filename,fd); return -1;}

	/* Find end of SD card */
	o_pos = lseek64 (fd,(off64_t)0,SEEK_END);
	blkmax = o_pos/512;	// Number of blocks on the SD card
	printf("Input file opened: %s, SEEK_END returned (blocks): %d  SDLOG_EXTRA_BLOCKS: %d\n",*(argv+1),blkmax, SDLOG_EXTRA_BLOCKS);

	/* Find end of SD card */
	o_pos = lseek64(fd,(off64_t)0,SEEK_END);
	blkmax = o_pos/512;	// Number of blocks on SD card
	printf("Success: %u %s, SEEK_END returned (blocks) %d \n",argc,*(argv+1),blkmax);

	/* Make keyboard input non-blocking */
	const char* pstdin = "/proc/self/fd/0";
	fdin = open(pstdin, O_RDONLY | O_NONBLOCK);

   while (1==1)
   {
	// NOTE: 'getaPID' returns (PID + 1), so PID = 0 conveys an error.
	pidE = getaPID(fd, (blkmax - 1 - SDLOG_EXTRA_BLOCKS)); // Last block logging writes
	pid0 = getaPID(fd, 0);
	if ((pidE == 0) || (pid0 == 0)) {printf("Error reading SD card: returned the PID block[0] = %llu block[%u] = %llu\n",pid0,(blkmax - 1 - SDLOG_EXTRA_BLOCKS),pidE); return -1;}
	if (pidE == 1)
	{ // Here high end PID is zero.  Non-wrap situation
		if (pid0 == 1)
		{ // Here.  Beginning & End blocks PIDs are zero
			printf("NO DATA ON CARD: Beginning and End PIDs are both zero\n"); 
//			return -1;
		}
	}

	highest_nonzero_blk = search_zero_end(fd, blkmax);
	for(j = 0; j < 64; j++)printf("-");printf("\n");
 	printf ("\nFile: %s\nSearch zero result: highest_nonzero_blk: %d end: %d incr: %d odd: %d \n\n",filename, highest_nonzero_blk, (blkmax - SDLOG_EXTRA_BLOCKS), ((blkmax-1)>>1) + ((blkmax-1)&1),(blkmax & 1));


	logblk.blk = highest_nonzero_blk;
	t_high = get1stpkttime(fd, &logblk);
	printf ("Highest: block#:%9d SD time: %s", logblk.blk, ctime(&t_high));
	logblk.blk = 0;
	t_low = get1stpkttime(fd, &logblk);
	printf ("Lowest : block#:%9d SD time: %s", logblk.blk, ctime(&t_low));

//	rewind(fd);

	strcpy(vv,ctime(&t_high));
	pc = strchr(vv,'\n');	// Remove '\n'
	if (pc != NULL) *pc = 0;
	while ( (pc=strchr(vv,':')) != NULL) *pc = '.';
	while ( (pc=strchr(vv,' ')) != NULL) *pc = '_';

	printmenu();
	do
	{
		scanf("%c",&jc);
		cmdidx = getcommand(jc);
		if (cmdidx < 0) {if(jc != '\n')printf("command char not recognized: %c\n",jc);}
	} while (cmdidx < 0);
	
	switch(cmdidx)
	{
	case CMD_Z:
		writezeros(blkmax);
		break;
	case CMD_Y:
		writezeros(highest_nonzero_blk);
		break;
	case CMD_W:
		check_entire_card_zero(blkmax);
		break;
	case CMD_T:	

		blk.blk = 0;	// We start with block zero, of course.
		blkdurctr = 0;	// Number of blocks between time jumps
		pktdurctr = 0;	// Number of packets between time jumps
		enddurctr = 0;	// Progress ticks at end when reading zero
		jmpctr = 0;	// Running count of number of time jumps

		/* Go through the whole SD card (well, at least just to where the logging ends). */
		printf("Checking for TIME GAPS, from block: %d through block: %d\n",blk.blk, (highest_nonzero_blk+1));
		printf("Type char x to cancel at any time\n");
		printf(" ct   blocks   packets block idx  CAN id     dlc   pay[0-4] pay[5-8]     start date/time tick               end date/time tick         tim_prev   can.U.ull  pid \n");
		int loopsw = 0;
		while ((blk.blk <= (highest_nonzero_blk+1)) && (loopsw == 0))
		{
			ret = get1stpkt(fd, &blk, &pktp, &can);	// Get first packet from block
			blkdurctr += 1;

			/* Break loop if 'x' is hit */
			if (read(fdin, &kb, 256) > 0){if (kb[0] == 'x') loopsw = 1;}
	
			/* Get the pointer and size for each packet within the block (in reverse time order) */
			switch (ret)
			{
			case 1: // All zero
				if ((zerosw != 0) && (++enddurctr <= (1000000 - 1)))break;
				enddurctr = 0;
				printf("All zero block: block index: %9d(dec) %08X(hex) \n",blk.blk, blk.blk);
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
						pktarray[ipktend] = pktp; 
						ipktend += 1; 
						if (ipktend > PKTMAX) 
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
							packet_print_date1(&can_save.U.ull); printf("  ");
							packet_print_date1(pktarray[j].p);
							printf (" %lld %lld",tim_prev,can.U.ull);
							printf(" %lld \n",blk.pid);\
							tim_prev = can.U.ull;
							pktdurctr = 0; blkdurctr = 0;
						}
						else
							packet_convert(&can_save, &pktarray[0]); // Save CAN struct that has last time stamp
					}
//printf ("X %2i %8i %9i \n",j, blkdurctr, pktdurctr);
					pktdurctr += 1;
//					packet_print(&pktarray[j]);
				}
				break;
			} // end switch
			blk.blk += 1;
		} // end while (blknum...
	} // end switch(cmdidx)
//	printf("END:            block index: %9d %08X  PID: %lld",blk.blk, blk.blk,blk.pid);
   } // end while (1==1)
	printf ("UNOBTAINABLE VERY END\n");
	return 0;
}
/********************************************************************************
 * void printmenu(void);
 * @brief	: print the command menu
********************************************************************************/

void printmenu(void)
{
	int i;
	printf("\nCommand options--\n");
	for (i = 0; i < CMDCT; i++)
		printf("%s",pcmd[i]);
	printf("\n");
	return;
}
/********************************************************************************
 * int getcommand(char c);
 * @brief	: get a command from the keyboard
 * @return	: command index
********************************************************************************/
int getcommand(char c)
{
	int i;
	for (i = 0; i < CMDCT; i++)
	{
		if (c == *(pcmd[i] + 1) ) break;
	}
	if (i < CMDCT) return i;

	return -1;
}
/********************************************************************************
 * void check_entire_card_zero(int blkmax);
 * @brief	: Examine each block to check that it is all zero
 * @param	: blkmax = highest block number on card
********************************************************************************/
void check_entire_card_zero(int blkmax)
{
	int i,j;
	int readct;
	int any;
	int zerosw = 0;
	int loopsw = 0; 

	char kb[16];

	off64_t o_blk;

	time_t t_prev = 0;
	time_t t_now = time(0); 

	printf("Check entire card to be zero'd.  Number blocks: %d\n",blkmax);
	system("date");

	for ( i = 0; (i < blkmax) && (loopsw == 0); i++)
	{
		o_blk = i;	o_blk = o_blk << 9;
		lseek64(fd,o_blk,SEEK_SET);	// Byte offset seek
		readct = read (fd, BlockBuff, BLOCKSIZE);
		if (readct != BLOCKSIZE)
		{ // Here, for some reason we didn't get the length expected from an sd card.
			printf("read was not 512 bytes.  It was %d at block index %d\n",readct,i);
			continue;
		}
		any = 0;
		for (j = 0; j < BLOCKSIZE; j++)
		{
			if (BlockBuff[j] != 0) 
			{
				any = 1; break;
			}
		}
		if (zerosw == 0)
		{
			if (any != 0)
			{
				printf("Not zero block starting at:      %8d\n",i); 
				zerosw = 1;
			}
		}
		else
		{
			if (any == 0)
			{
				printf("Zero's starting at block number: %8d\n",i);
				zerosw = 0;
			} 
		}

		if ( (i & ((1<<11)-1)) == 0 )
			printf("progress %% %4.0f  \r", 100.0 * ( (float)(i) / (float)(blkmax) ) );

		/* Break loop if 'x' is hit */
		if (read(fdin, &kb, 16) > 0){if (kb[0] == 'x') loopsw = 1;}
	}
	t_prev = time(0);
	printf("  DONE                              \n");
	double blkps = (double)blkmax/(double)(t_prev-t_now);
	printf("Copy time (secs): %u  blocks/sec: %6.0f Mbyte/sec: %7.1f\n",(unsigned int)(t_prev-t_now), blkps, (blkps * 512)/1E6);
	return;
}
/******************************************************************************
 * int readblock(int fd, char* p, int blk);
 * @brief	: Read one block
 * @param	: fd = file descriptor--input
 * @prarm	: p = pointer to block buffer
 * @return	: 0 = OK; -1 = failed miserably
 ******************************************************************************/
int readblock(int fd, char* p, int blk)
{	
			
	int readct = read (fd, p, BLOCKSIZE);
	if (readct != BLOCKSIZE)
	{ // Here, for some reason we didn't get the length expected from an sd card.
		printf("read was not 512 bytes.  It was %d at block index %d\n",readct,blk); return -1;
	}

	return 0;
}

/********************************************************************************
 * void writezeros(int blkmax);
 * @brief	: write blocks of zero
 * @param	: blkmax = highest block number on card
********************************************************************************/
void writezeros(int blkmax)
{
	int i;
	char buff[BLOCKSIZE];
	int writect;

	int loopsw = 0; 
	char kb[256];
	time_t t_prev = 0;
	time_t t_now = time(0); 

	off64_t o_blk;

	int sw = 0;
	while (sw == 0)
	{
		printf("Do you really want to zero this card?  Enter yes or no\n");
		scanf("%s",&kb[0]);
		if ((strcmp(kb,"yes")) == 0) sw = 1;
		if ((strcmp(kb,"no")) == 0) return;
	}
	

	printf("Number blocks to zero: %d\n",blkmax);
	system("date");

	for (i = 0; i < BLOCKSIZE; i++) buff[i] = 0; // Setup buffer of zeroes

	for (i = 0; (i < blkmax) && (loopsw == 0); i++)
	{
		o_blk = i;	o_blk = o_blk << 9;
		lseek64(fd,o_blk,SEEK_SET);	// Byte offset seek
		writect = write (fd, buff, 512);
		if (writect != 512)
		{ // Here, for some reason we didn't get the length expected from an sd card.
			printf("write was not 512 bytes.  It was %d at block index %d\n",writect,i);
		}
		if ( (i & ((1<<11)-1)) == 0 )
			printf("progress %% %4.0f  \r", 100.0 * ( (float)(i) / (float)(blkmax) ) );

		/* Break loop if 'x' is hit */
		if (read(fdin, &kb, 16) > 0){if (kb[0] == 'x') loopsw = 1;}
	}
	printf("  DONE                              \n");
	t_prev = time(0);
	double blkps = (double)blkmax/(double)(t_prev-t_now);
	printf("Copy time (secs): %u  blocks/sec: %6.0f Mbyte/sec: %7.1f\n",(unsigned int)(t_prev-t_now), blkps, (blkps * 512)/1E6);
	return;
}

