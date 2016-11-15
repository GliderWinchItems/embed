/******************************************************************************
* File Name          : common_highflash.h
* Date First Issued  : 10/15/2014
* Board              : 
* Description        : Fixed area in high flash for application calibrations
*******************************************************************************/
#ifndef __HIGHFLASH
#define __HIGHFLASH

#include "libopencm3/cm3/common.h"

/* Code for designating the number in the .txt file "//i " lines
//        number type codes
//        0 - u32 (unsigned int)
//        1 - s32 (int)
//        2 - u64 (unsigned long long)
//        3 - s64 (long long)
//        4 - float
//        5 - double
//        6 - string of chars (zero termination added after 1st double space, or
//            tab.  It fills "blocks" of 4 bytes.)
//        7 - string with CAN ID name
//
*/
#define CALTYPE_U32		0
#define CALTYPE_S32		1
#define CALTYPE_U64		2
#define CALTYPE_S64		3
#define CALTYPE_FLOAT		4
#define CALTYPE_DOUBLE		5
#define CALTYPE_STRING		6
#define CALTYPE_CANID		7
#define CALTYPE_LAST		7	// Highest number in list


struct CRCBLOCK
{
	u32 pstart;	// Start address
	u32 count;	// Byte count
	u32 crc;	// CRC-32 expected
};

#define NUMCANIDS	96
struct APPCANID
{
	u32 crc;	// CRC-32 on whole mess
	u32 version;		// Version number
	u32 numcanids;	// Number of CAN ID's
	u32 canid[NUMCANIDS];	// Application CAN ID's
};

#define	NUMPROGSEGS	8
struct CRCPROG
{
	u32 crc;		// CRC-32 on this struct
	u32 version;		// Version number
	u32 numprogsegs;	// Number of program segments
	struct CRCBLOCK crcblk[NUMPROGSEGS]; // Prog segments to check CRC
};



/* Use the following to get size, and address relative positioning. */
struct HIGHFLASHH	// APP CAN ID and program CRC checks
{
	u32	crc;		// CRC-32 on this struct
	u32	version;	// Version
	/* One instance of each struct comprises the high flash layout. */
	struct	APPCANID 	appcanid;
	struct	CRCPROG 	crcprog;
};

#define NUMBEROF4BYTSLOTS (0x0C00 - 8)
struct HIGHFLASHP	// Generic calibration and parameters
{
	u32	crc;		// CRC-32 on this struct
	u32	version;	// Version
	u32 x[NUMBEROF4BYTSLOTS];
};


/*******************************************************************************/
struct APPCANID*   gc_hiflash_appcanid(u32 version);
struct CRCPROG*    gc_hiflash_crcprog(u32 version);
struct BOARDCALIB* gc_hiflash_boardcalib(u32 version);
struct APPPARAM*   gc_hiflash_appparam(u32 version);
struct HIGHFLASHLAYOUT* gc_hiflash_highflash(u32 version);
/* @brief 	: Check crc and return pointer to struct
 * @param	: version = version number expected
 * @return	: NULL = crc failed or version did not match, otherwise pointer 
*******************************************************************************/

#endif 

