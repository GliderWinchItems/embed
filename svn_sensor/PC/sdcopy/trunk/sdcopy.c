/************************************************************************************
sdcopy.c--SD card packet reader to file
06-18-2014
Copy SD card to a HD file
*************************************************************************************/
/*
Hack of sdpktf to implement date/time search and output ascii/hex lines

// Example--
cd ~/svn_sensor/PC/sdcopy/trunk
make && sudo ./sdcopy /dev/sdf $HOME/winch/instrumentation
 arguments: 
 1.  input device or file path/name
 2.  path for output (name is generated)

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
#include <errno.h>



#include "packet_extract.h"
#include "packet_print.h"
#include "can_write.h"
#include "packet_search.h"
#include "sdlog.h"

int readwriteblock(int fd2, int fd1, unsigned char* p, int blk);

#define BLOCKSIZE  512
unsigned char BlockBuff[BLOCKSIZE];	// Current sd block
struct PKTP pktp;			// Current pid, packet size, packet pointer
struct PKTP pktp1;			// Current pid, packet size, packet pointer
#define PKTMAX	128
struct PKTP pktarray[PKTMAX];

char* vvpath = "echo $HOME/winch/instrumentation/"; // Path for output file

int blknum;
int blkmax;
int blkmin;
off64_t o_pos;
off64_t o_end;
off64_t o_blk;
int readablock_err;
int fd1;
int fd2;
/********************************************************************************
main
********************************************************************************/
int main (int argc, char **argv)
{
//	FILE *fp;
	

	int i;

	printf ("\n'sdcopy.c' - 06-18-2014\n");
	if (argc != 3)
	{ 
		printf("Need sd card device and output file path: e.g. /dev/sdf /home/user/winch/instrumentation\n");	return -1;
	}
//	fp = fopen( *(argv+1), "rb");
	fd1 = open( *(argv+1), O_RDONLY);
	if (fd1 < 0) 
	{
		printf ("\nSD CARD OPEN FAIL: %u %s %d\n",argc,*(argv+1),fd1);
		return -1;
	}
	
	/* Find end of SD card */
	o_pos = lseek64(fd1,(off64_t)0,SEEK_END);
	o_end = o_pos;		// Save end of SD position
	blkmax = o_pos/512;	// Number of blocks on SD card
	printf("Success: %u %s, SEEK_END returned (blocks) %d \n",argc,*(argv+1),blkmax);


	int highest_nonzero_blk;
	int blknum_highest_pid;
	unsigned long long pidx,pidE,pid0;
	int blk_rotate;	// Number to rotate blk index for binary search of date/time

	// NOTE: 'getaPID' returns (PID + 1), so PID = 0 conveys an error.
	pidE = getaPID(fd1, (blkmax - 1 - SDLOG_EXTRA_BLOCKS)); // Last block logging writes

	blkmin = 0;

	/* The following is for recovering data if the 1st block has a zero PID, but later ones are OK. */
#define BADBEGINNINGSIZE 8	// Number of blocks we will allow at beginning to have zero PID's
	for (blkmin = 0; blkmin < BADBEGINNINGSIZE; blkmin++)
	{
		pid0 = getaPID(fd1, blkmin);
		if (pid0 == 0)  {printf("Error reading SD card\n"); return -1;}
		if (pid0 != 0) break;
	}
	if (blkmin >= BADBEGINNINGSIZE)
	{
		printf("THE FIRST %d BLOCKS HAVE ZERO pid's.  So sorry, it looks like the card doesn't have usable data\n", BADBEGINNINGSIZE);
		return -1;
	}
	if (blkmin != 0)
		printf("BLOCK # %d is the first block that is not-zero...maybe data useable, maybe not.\n",blkmin);

	/* Check the high end PID */
	if (pidE == 0)  {printf("Error reading SD card\n"); return -1;}
	if (pidE == 1)
	{ // Here high end PID is zero.  Non-wrap situation
		printf("THE HIGH END HAS A ZERO pid, SO THE DATA HAS NOT WRAPPED AROUND\n");
	}

	highest_nonzero_blk = search_zero_end(fd1, blkmax, blkmin);
 	printf ("SEARCH FOR END OF DATA: highest_nonzero_blk: %d last data block: %d\n\n",highest_nonzero_blk, (blkmax - SDLOG_EXTRA_BLOCKS));

	blknum_highest_pid = search_highest_pid (fd1, blkmax);
	if (blknum_highest_pid == 0)
	{
		printf("Error finding the highest PID\n");
	}
	/* Number of blocks to make the block number (index) of the block with the highest PID
           become the last block of the SD card */
	blk_rotate = ((blkmax - SDLOG_EXTRA_BLOCKS) - blknum_highest_pid); 
	pidx = getaPID(fd1, blknum_highest_pid);
	printf ("The block number of the highest_pid: %d  rotate: %d pid: %llu %llX\n",blknum_highest_pid, blk_rotate,(pidx-1), (pidx-1));
	
	/* Date search */
	struct LOGBLK logblk;
	time_t t_low;
	logblk.blk = blknum_highest_pid;
	time_t t_high = get1stpkttime(fd1, &logblk);
	printf ("\nThe highest and lowest times found on this SD card---\n");
	printf ("Highest: block#:%9d SD time: %s",logblk.blk, ctime(&t_high));
	pidx = getaPID(fd1, (blkmax - 1 - SDLOG_EXTRA_BLOCKS));
	if (pidx > 1)
	{ // Here a wraparound situation
		blknum_highest_pid += 1; if (blknum_highest_pid >= (blkmax - SDLOG_EXTRA_BLOCKS)) blknum_highest_pid -= (blkmax - SDLOG_EXTRA_BLOCKS);
		logblk.blk = blknum_highest_pid;
		printf("SD card data has wrapped around\n");
	}
	else
	{ // Here, SD not yet filled
		logblk.blk = 0;
	}
	t_low = get1stpkttime(fd1, &logblk);
	printf ("Lowest : block#:%9d SD time: %s", logblk.blk, ctime(&t_low));
	
	/* Output file name setup */
	char vv[256];
	char ww[256];
	char *pc;
	strcpy(vv, *(argv+2));		// Setup path
	strcat(vv,"/");
	strcat(vv,ctime(&t_high)); 	// Append date/time
	pc = strchr(vv,'\n');		// Remove '\n' in the date/time
	if (pc != NULL) *pc = 0;
	while ( (pc=strchr(vv,':')) != NULL) *pc = '.'; // Replace ':' that causes script problems later
	while ( (pc=strchr(vv,' ')) != NULL) *pc = '_'; // Same thing for spaces
	strcpy(ww,vv);			// Save for later tar'ing
	strcat(vv,".RAW");		// Give a nice extension name

	fd2 = open(vv, O_RDWR|O_CREAT, 0444);	// Open with permissions for everybody to read
	if (fd2 < 0)
	{
		i = errno;
		printf ("\nOutput file did not open: %s   file descriptor: %d error number: %d\n",vv, fd2, errno);
		printf("Error description is : %s\n\n",strerror(i));
		 return -1;
	}	
	printf("\nOUTPUT FILE NAME: ====>%s<====\n\n", vv);

	/* Setup the file name so that after exit the script can find it. */
	char zz[256];
	strcpy (zz,"echo ");
	strcat (zz,ww);
	strcat (zz," > cpname"); // File path/name is stored in 'cpname' file
	system (zz);

//return 0;

	lseek64(fd1,(off64_t)0,SEEK_SET);

	/* Copy data blocks */
	system("date");
	time_t t_prev = 0;
	time_t t_now = time(0); 
	printf("Number of blocks to copy: %u\n",highest_nonzero_blk);
/* -------------------------------------------------------------------------------------------------------- */
	while (blknum < (highest_nonzero_blk+1))
	{
		readwriteblock(fd2, fd1, BlockBuff, blknum);
		blknum += 1;

		if ( (blknum & ((1<<11)-1)) == 0 )
		{
			printf("progress: %4.0f%% %9d %9d  \r", ((float)blknum*100.0)/(float)highest_nonzero_blk, blknum, highest_nonzero_blk);
		}
	}
	printf("           DONE                                                                 \n");

	/* Copy extra block region. */
	lseek64(fd1,(o_end - (2048*512)),SEEK_SET);	// Skip to extra blocks
	for (i = 0; i < SDLOG_EXTRA_BLOCKS; i++)
	{
		readwriteblock(fd2, fd1, BlockBuff, blknum);
		blknum += 1;
	}
	t_prev = time(0);
	system("date");
	double blkps = (double)highest_nonzero_blk/(double)(t_prev-t_now);
	printf("Copy time (secs): %u  blocks/sec: %6.0f Mbyte/sec: %7.1f\n",(unsigned int)(t_prev-t_now), blkps, (blkps * 512)/1E6);
	
	return 0;
}
/******************************************************************************
 * int readwriteblock(int fd2, int fd1, char* p);
 * @brief	: Read and write one block
 * @param	: fd2 = file descriptor--output
 * @param	: fd1 = file descriptor--input
 * @prarm	: p = pointer to block buffer
 * @return	: 0 = OK; -1 = failed miserably
 ******************************************************************************/
int readwriteblock(int fd2, int fd1, unsigned char* p, int blk)
{	
			
	int readct = read (fd1, p, 512);
	if (readct != 512)
	{ // Here, for some reason we didn't get the length expected from an sd card.
		printf("read was not 512 bytes.  It was %d at block number %d\n",readct,blk); return -1;
	}
	readct = write (fd2, p, 512);
	if (readct != 512)
	{ // Here, for some reason we didn't get the length expected from an sd card.
		printf("write was not 512 bytes.  It was %d at block number %d\n",readct,blk); return -2;
	}
	return 0;
}
