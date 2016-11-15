/******************************************************************************
* File Name          : packet_search.c
* Date First Issued  : 06/01/2014
* Board              : PC
* Description        : Search packets
*******************************************************************************/

#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "packet_search.h"
#include "packet_print.h"

#include "sdlog.h"


/******************************************************************************
 * int readablock(int fd, struct LOGBLK* pblk);
 * @brief	: Read block
 * @param	: fd = file descriptor for read/write
 * @param	: Pointer to buffer with requested block number set
 * @return	:  1 = msg time < search time;
 *		:  0 = msg time >= search time;
 *		: -1 = Read was not 512 bytes
 *		: -2 = Block was all zeros
 *		: -3 = Extraction error, and not all zeros.
 ******************************************************************************/
int readablock(int fd, struct LOGBLK* pblk)
{
	int readct;
	off64_t o_blk;
	off64_t o_pos;

	/* Position for reading. */
	o_blk = pblk->blk;	o_blk = o_blk << 9; 	// Convert block number to byte offset
	o_pos = lseek64(fd,o_blk,SEEK_SET);		// Byte offset seek
	if (o_pos < 0)	// In case something went awry
	{ // Here, lseek returned an error.
		printf("seek error: %lld for block: %u, %llu\n",(unsigned long long)o_pos, pblk->blk, (unsigned long long)o_blk/512); return -1;
	}

	/* Read the block */
	readct = read (fd, (char*)&pblk->data[0], 512);
	if (readct != 512)
	{ // Here, for some reason we didn't get the length expected from an sd card.
		printf("Read was not 512 bytes.  It was %d, pblk %d\n",readct, pblk->blk); return -1 ;
	}

	return 0;	// Success
}
/******************************************************************************
 * unsigned long long getaPID(int fd, int blk);
 * @brief	: Read block and extract PID
 * @param	: fd = file descriptor
 * @param	: blk = block number 
 * @return	:  n = PID + 1
 *		:  0 = error
 ******************************************************************************/
unsigned long long getaPID(int fd, int blk)
{
	struct LOGBLK logblk;
	logblk.blk = blk;
	int ret = readablock(fd, &logblk);
	if (ret != 0) return 0;
	return (logblk.pid + 1);
}

/******************************************************************************
 * int search_zero_end (int fd, int blkmax, int blkmin);
 * @brief	: Find highest non-zero pid block number
 * @param	: fd = file descriptor
 * @param	: blkmax = number of blocks on SD card 
 * @param	: blkmin = lowest non-zero block (should be zero, but maybe not)
 * @return	:  n = block number (0 - n) if last non-zero pid block
 *		:  0 = error or no data
 ******************************************************************************/
int search_zero_end (int fd, int blkmax, int blkmin)
{
	int imax = (blkmax - 1 - SDLOG_EXTRA_BLOCKS);	// index of last highest blk written
	int imin = blkmin;
	int imid = 0;
	int imid_prev = -1;

	unsigned long long pid = getaPID(fd, 0);	// Get PID of the first block
	if (pid <= 1)	return 0;  	// No data on 1st block, or error

	pid = getaPID(fd, imax);		// Get PID of the last block

	/* Quick wrap-around check. */
	if (pid >  1) return (imax);	// Return: Wraparound--Last position has non-zero PID
	if (pid == 0) return   0;   	// Return: error getting PID


//int ct = 0;
	/* Find block with the last non-zero PID. */
	// Note: Here, the highest block has a zero PID.
	while (imin < imax)	// Loop
	{
		imid = (imax + imin)/2;
		if (imid == imid_prev) return imid;
		imid_prev = imid;

		pid = getaPID(fd, imid);
		if (pid == 0) return 0; // Return: error getting PID
		if (pid == 1)		// Was PID zero? (PID is offset by 1)
			imax = imid;	// Yes
		else
			imin = imid;	// No
// printf("zero ct: %d imax: %d imin: %d imid: %d pid: %llX \n",++ct,imax,imin,imid, (pid-1));
		
	}
	return (imid);			// Return last non-zero PID
}			
/******************************************************************************
 * int search_highest_pid (int fd, int blkmax);
 * @brief	: Find highest pid block number
 * @param	: fd = file descriptor
 * @param	: blkmax = number of blocks on SD card
 * @return	:  n = block number (0 - (blkmax - 1 - SDLOG_EXTRA_BLOCKS) of block with highest PID
 *		:  0 = error or some sort of unhappiness
 ******************************************************************************/

int search_highest_pid (int fd, int blkmax)
{
	int imax = (blkmax - 1 - SDLOG_EXTRA_BLOCKS);	// index of last highest blk written
	int imin = 0;
	int imid = 0;
	int imid_prev = -1;
	int pid_prev;

	unsigned long long pid = getaPID(fd, 0);	// Get PID of the first block
	if (pid <= 1)	return 0;  	// No data on 1st block, or error

	pid_prev = getaPID(fd, imax);		// Get PID of the last block

	/* Quick wrap-around check. */
	if (pid_prev >  1) return (imax);	// Return: Wraparound--Last position has non-zero PID
	if (pid_prev == 0) return   0;   	// Return: error getting PID
	pid_prev = getaPID(fd, 0);

//int ct = 0;
	/* Find block with the last non-zero PID. */
	// Note: Here, the highest block has a zero PID.
	while (imin < imax)	// Loop
	{
		imid = (imax + imin)/2;
		if (imid == imid_prev) return imid;
		imid_prev = imid;

		pid = getaPID(fd, imid);
		if (pid == 0) return 0; // Return: error getting PID
		if (pid < pid_prev)		// Was PID zero? (PID is offset by 1)
			imax = imid;	// Yes
		else
			imin = imid;	// No
// printf("pid ct: %d imax: %d imin: %d imid: %d pid: %llX \n",++ct,imax,imin,imid, (pid-1));
		
	}
	return (imid);			// Return last non-zero PID
}
/******************************************************************************
 * int search_rotate(int blk, int rotate, int blkmax);
 * @brief	: Find highest pid block number
 * @param	: blk = block number to be rotated (0 - n)
 * @prarm	: rotate = number of blks to make highest PID blk the last
 * @param	: blkmax = number of blocks on SD card
 * @return	: rotated block number
 ******************************************************************************/			
static int search_rotate(int blk, int rotate, int blkmax)
{
	int blkr = (blk + rotate);
	if (blkr >= (blkmax - SDLOG_EXTRA_BLOCKS)) blkr -= (blkmax - SDLOG_EXTRA_BLOCKS);
	return blkr;
}
/******************************************************************************
 * time_t get1stpkttime(int fd, struct LOGBLK* pblk);
 * @param	: fd = file descriptor
 * @param	: pblk = pointer to struct with blk number and time
 * @return	: t = Linux time (secs) for first (going backwards) packet
 *        	: 1 = error findng first packet within the block
 *		: 0 = read block error
 * Given "logical" block number (meaning highest logical block number is always
 * the block number of (N-1) where N is the number of blocks on the SD card),
 * find the physical block number.  'rotate' is the difference between the highest
 * block number with the highest PID, and the end of the SD card.
 ******************************************************************************/
time_t get1stpkttime(int fd, struct LOGBLK* pblk)
{	
	/* Read the block */
	int ret = readablock(fd, pblk);
	if (ret != 0) return 0;

	/* Find the first packet in the block. */
	struct PKTP pktp;
	ret = packet_extract_first(&pktp, (unsigned char*)pblk);
	if (ret != 0) return 1;

	/* Convert packet to CAN struct. */
	struct CANRCVTIMBUF can;
	packet_convert(&can, &pktp);

	time_t t = can.U.ull >> 6;	// Time in whole seconds
	t += PODTIMEEPOCH;		// Adjust for shifted epoch

	/* Return the linux time (secs). */
	return t;	
}

/******************************************************************************
 * struct SEARCHRESULT search_datetime (int fd, time_t datetime, int rotate, int blkmax);
 * @brief	: Find block number for datetime that matches (whole seconds)
 * @param	: fd = file descriptor
 * @param	: datetime = linux time (secs)
 * @prarm	: rotate = number of blks to make highest PID blk the last
 * @param	: blkmax = Number of blocks number on SD card
 * @return	: TWOINTS
 *		:   n1 = block number of something
 *		:   n2:  1 = exact match
 *		:	 0 = OK
 *		:	-1 = error getting PID
 *		:       -2 = not found
 ******************************************************************************/
struct SEARCHRESULT search_datetime (int fd, time_t datetime, int rotate, int blkmax)
{
	int imax = (blkmax - 1 - SDLOG_EXTRA_BLOCKS);	// Block number index of highest block
	int imin = 0;
	int imid = 0;
	int imid_prev = -1;
	struct SEARCHRESULT sr = {0,0,0};

	/* Rotated blk numbers */
	int rmax = search_rotate(imax, rotate, blkmax);

	struct LOGBLK logblk;

	logblk.blk = rmax;

//logblk.blk = rmax;
//printf("rot: imid: %d rotate: %d blkmax: %d logblk.blk: %d\n", imid, rotate, blkmax, logblk.blk);
//sr.t = get1stpkttime(fd, &logblk); 
//printf("%d %s", rmax, ctime(&sr.t) );
//return sr;

int ct = 0;
	while (imin < imax)	// Loop
	{
		imid = (imax + imin)/2;
		if (imid == imid_prev) {sr.blk = imid; sr.type = 0; return sr;}
		imid_prev = imid;

		logblk.blk = search_rotate(imid, rotate, blkmax);
		sr.t = get1stpkttime(fd, &logblk);

		if (sr.t == 0) 		{sr.blk = -1;          sr.type = -1; return sr;} // Error getting PID
		if (sr.t == datetime)	{sr.blk =  logblk.blk; sr.type =  1; return sr;} // Exact match
		if (sr.t >  datetime)	// Compare (rotated) packet time to requested time
			imax = imid;	// Midpt time is too high
		else
			imin = imid;	// Midpt time is too low
printf("tim ct: %3d rmax: %7d imin: %7d imid: %7d imax: %7d rmid: %7d logblk.blk %7d %s",++ct,search_rotate(imax, rotate, blkmax),imin,imid,imax, search_rotate(imid, rotate, blkmax),logblk.blk,ctime(&sr.t));
	}
	sr.blk = 0; sr.type = -2; return sr;
}			

