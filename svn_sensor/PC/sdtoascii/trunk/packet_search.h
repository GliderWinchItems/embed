/******************************************************************************
* File Name          : packet_search.h
* Date First Issued  : 06/01/2014
* Board              : PC
* Description        : Search packets
*******************************************************************************/

#ifndef __PACKET_SEARCH
#define __PACKET_SEARCH

#include "common_can.h"
#include "packet_extract.h"
#include <time.h>

struct SEARCHRESULT
{
	int	blk;
	int	type;
	time_t t;
};

/******************************************************************************/
int readablock(int fd, struct LOGBLK* pblk);
/* @brief	: Read block
 * @param	: fd = file descriptor for read/write
 * @param	: Pointer to buffer with requested block number set
 * @return	:  1 = msg time < search time;
 *		:  0 = msg time >= search time;
 *		: -1 = Read was not 512 bytes
 *		: -2 = Block was all zeros
 *		: -3 = Extraction error, and not all zeros.
 ******************************************************************************/
unsigned long long getaPID(int fd, int blk);
/* @brief	: Read block and extract PID
 * @param	: fd = file descriptor
 * @param	: blk = block number 
 * @return	:  n = PID + 1
 *		:  0 = error
 ******************************************************************************/
int search_zero_end (int fd, int blkmax);
/* @brief	: Find highest non-zero pid block number
 * @param	: fd = file descriptor
 * @param	: blk: block number 
 * @return	:  n = block number
 *		:  0 = error or no data
 ******************************************************************************/
int search_highest_pid (int fd, int blkmax);
/* @brief	: Find highest pid block number
 * @param	: fd = file descriptor
 * @param	: blk: block number of non-zero highest PID
 * @return	:  n = block number
 *		:  0 = error or no data
 ******************************************************************************/
struct SEARCHRESULT search_datetime (int fd, time_t datetime, int rotate, int blkmax);
/* @brief	: Find block number for datetime that matches (whole seconds)
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
time_t get1stpkttime(int fd, struct LOGBLK* pblk);
/* @param	: fd = file descriptor
 * @param	: pblk = pointer to struct with blk number and time
 * @return	: t = Linux time (secs) for first (going backwards) packet
 *        	: 1 = error findng first packet within the block
 *		: 0 = read block error
 * Given "logical" block number (meaning highest logical block number is always
 * the block number of (N-1) where N is the number of blocks on the SD card),
 * find the physical block number.  'rotate' is the difference between the highest
 * block number with the highest PID, and the end of the SD card.
 ******************************************************************************/


#endif

