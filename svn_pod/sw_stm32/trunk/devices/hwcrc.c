/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : HWCRC.c
* Generator          : deh
* Date First Issued  : 01/14/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Hardware CRC
*******************************************************************************/
/* NOTE:
Page 57 Ref Manual.

Useful links:
https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=%2Fpublic%2FSTe2ecommunities%2Fmcu%2FLists%2Fcortex_mx_stm32%2FCRC%20calculation%20in%20software&FolderCTID=0x01200200770978C69A1141439FE559EB459D7580009C4E14902C3CDE46A77F0FFD06506F5B&currentviews=2545

https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=https%3a%2f%2fmy.st.com%2fpublic%2fSTe2ecommunities%2fmcu%2fLists%2fcortex_mx_stm32%2fCC%2b%2b%20Example%20for%20CRC%20Calculation&FolderCTID=0x01200200770978C69A1141439FE559EB459D7580009C4E14902C3CDE46A77F0FFD06506F5B&currentviews=2282

http://www.ross.net/crc/download/crc_v3.txt

*/

#include "libopenstm32/crc.h"
#include "libopenstm32/rcc.h"

/******************************************************************************
 * unsigned int hwcrcgen(unsigned int * p, unsigned int  count);
 * @brief	: Compute a CRC using the hardware CRC feature
 * @param	: p: Pointer to array to be crc'ed
 * @param	: count: (==> INT <==) to be CRC'ed
 * @return	: CRC 
 ******************************************************************************/
unsigned int hwcrcgen(unsigned int * p, unsigned int count)
{
	unsigned int i;

	/* Be sure the clocking for the CRC hardware is enabled */
	RCC_AHBENR |= RCC_AHBENR_CRCEN;	// Enable CRC hardware if not enabled

	/* Reset CRC to 0xFFFFFFFF */
	CRC_CR = 1;	// Some hardware guy said this is bright idea.  Why not just write 0xFFFFFFFF?  

	/* Build CRC 4 bytes at a time */
	for ( i = 0; i < count; i += 1)
	{
		CRC_DR = *p++;		// Step to next int
	}

	/* Return computed CRC */
	return CRC_DR;
}
	

