/******************************************************************************
* File Name          : common_highflash.h
* Date First Issued  : 10/15/2014
* Board              : 
* Description        : Fixed area in high flash for application calibrations
*******************************************************************************/
#ifndef __SVN_COMMON_HIGHFLASH0
#define __SVN_COMMON_HIGHFLASH0

#include "common_can.h"

/* 
   Obsolete
   Code for designating the number in the .txt file "//i " lines
   These codes were with original flat-file .txt parameter/calibration.
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


/* 
These define the tables stored in upper flash and reloaded independently of
of the loader, and application programs.
*/

struct FUNC_CANID
{
	uint32_t func;		// Function code
	uint32_t canid;		// Command CAN ID for function
};

struct CRCBLOCK
{
	u32 pstart;	// Start address
	u32 count;	// Byte count
	u32 crc;	// CRC-32 expected
};

#define NUMCANIDS	16	// Max number of CAN IDs
#define	NUMPROGSEGS	8	// Max number of program segments

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
	struct FUNC_CANID func_canid[NUMCANIDS];	// Application CAN ID's
	struct	CRCPROG 	crcprog;
};

#define NUMBEROF4BYTSLOTS (0x0C00 - 8)
struct HIGHFLASHP	// Generic calibration and parameters
{
	u32	crc;		// CRC-32 on this struct
	u32	version;	// Version
	u32 x[NUMBEROF4BYTSLOTS];
};

/* A version 2 of the table layout. */
#define NUMCANIDS2	16	// Max number of CAN IDs
struct FLASHH2	// Command CAN ID table, top reserved flash section
{
	u32	unit_code;		// CAN unit (node) code 
	u32	size;			// Number of entries that follow
  struct FUNC_CANID slot[NUMCANIDS2];	// Application CAN ID's
};

/* A more general labeling of the above. */
/*
COMMAND CAN ID table
  x[0]
    p0 = can unit command CAN id (hex) (used by loader)
    p1 = table size (number of elements that follow)
  x[1-n]
    p0 = function code
    p1 = function command CAN ID (hex) (used by app)
APP CRC table
  x[0]
    p0 = crc for the fo

*/
struct HIGHFLASHPAIRS
{
	uint32_t p0;		// 
	uint32_t p1;		// 
};


/*******************************************************************************/
/* These are obsolete, the the symbols may still lurk.**************************/
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

