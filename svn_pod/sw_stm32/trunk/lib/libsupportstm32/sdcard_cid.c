/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_cid.c
* Hacker             : deh
* Date First Issued  : 08/29/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Extraction of sd card cid fields 
*******************************************************************************/
#include "sdcard_cid.h"



/**********************************************************************************
 * int sdcard_cid_extract_v1 (char * p, char* cid, unsigned short usFieldBitNumber);
 * @brief	: Extract the bit field into an easy-to-use int from the CSD data (sd v1)
 * @param	: ASCII string return.  Max size including terminator 6 bytes.
 * @param	: Pointer to CSD record (16 bytes)
 * @param	: Bit number of the field of the low order bit
 * @return	: negative = field was ascii; postivie = value in field, 65536 = usFieldBitNumber
**********************************************************************************/
int sdcard_cid_extract_v1 (char * p, char* cid, unsigned short usFieldBitNumber)
{
	int i;

	switch (usFieldBitNumber)
	{
	case 0:	// Always 1
		return ( *(cid+15) & 0x01);
	case 1:	// CRC7
		return ( (*(cid+15) >> 1) & 0x7f );
	case 8:	// MDT - Manufacturing date (3 bcd
		return ( ((*(cid+13) & 0xf) << 8)|(*(cid+14) ) );
	case 55:// PSN - Product Serial number
		return ( (*(cid+9)<<24)|(*(cid+10)<<16)|(*(cid+11)<<8)|(*(cid+12) ));
	case 56:// PRV - Product revision (two BCD)
		return ( *(cid+8) );
	case 64:// PNM - Produce name (5 ascii)
		for (i = 0; i < 5; i++)
			*(p+i) = *(cid+3+i);
		*(p+5) = 0;
		return -1;
	case 104:// OID - OEM/Application ID (two char ascii)
		*p = *(cid+1); *(p+1) = *(cid+2); *(p+2) = 0;
		return -1;
	case 126: // MID - Manufacturer ID
		return  (*(cid));

	}	
		return 65536;
	
}


