/******************************************************************************
* File Name          : flash_write.c
* Date First Issued  : 07/28/2013
* Board              : ../svn_sensor/hw/trunk/eagle/f103R/RxT6
* Description        : flash write: small bits of code that execute in ram
*******************************************************************************/
/*


*/
#include "flash.h"
/******************************************************************************
 * int flash_write_ram(u16 *pflash, u16* pfrom, int count);
 *  @brief 	: Write "count" u16 (1/2 words) to flash from ram
 *  @param	: pflash = 1/2 word pointer to address in flash
 *  @param	: pfrom = 1/2 word pointer to address with source data
 *  @param	: count = number of 1/2 words to write
 *  @return	: zero = success; not zero = failed
*******************************************************************************/
/*     The main Flash memory programming sequence in standard mode is as follows:
      ●    Check that no main Flash memory operation is ongoing by checking the BSY bit in the
           FLASH_SR register.
      ●    Set the PG bit in the FLASH_CR register.
      ●    Perform the data write (half-word) at the desired address.
      ●    Wait for the BSY bit to be reset.
      ●    Read the programmed value and verify.
Note: The registers are not accessible in write mode when the BSY bit of the FLASH_SR register
      is set.
*/
int flash_write_ram(u16 *pflash, u16 *pfrom, int count)
{
	while (count > 0)
	{
		while ((FLASH_SR & 0x1) != 0);
		FLASH_CR |= 1;	// Set program bit
		*pflash++ = *pfrom++;
		while ((FLASH_SR & 0x1) != 0);
		FLASH_CR |= 1;	// Set program bit
		*pflash++ = *pfrom++;
		count--;
	}
	
	return 0;	
}

/******************************************************************************
 *  int flash_unlock(u32 address);
 *  @brief 	: Perform unlock sequence
 *  @return	: 0 = unlocked; not zero = failed and locked until next reset.
*******************************************************************************/
#define FLASH_RDPRT_KEY 0x00A5	// Protection code
#define FLASH_KEY1  0x45670123	// Unlock 1st
#define FLASH_KEY2  0xCDEF89AB	// Unlock 2nd

int flash_unlock(void)
{
	FLASH_KEYR = 0x45670123;
	FLASH_KEYR = 0xCDEF89AB;
	return (FLASH_CR & FLASH_LOCK);
}
int flash_unlock2(void)
{
	FLASH_KEYR2 = 0x45670123;
	FLASH_KEYR2 = 0xCDEF89AB;
	return (FLASH_CR2 & FLASH_LOCK);
}

/******************************************************************************
 * int flash_write(u16 *pflash, u16 *pfrom, int count);
 *  @brief 	: Write "count" u16 (1/2 words) to flash
 *  @param	: pflash = 1/2 word pointer to address in flash
 *  @param	: pfrom = 1/2 word pointer to address with source data
 *  @param	: count = number of 1/2 words to write
 *  @return	: 
 *           0 = success
 *          -1 = address greater than 1 MB
 *          -2 = unlock sequence failed for upper bank
 *          -3 = address below start of ram.
 *          -4 = unlock sequence failed for lower bank
 *          -5 = error at some point in the writes, flash_err has the bits
*******************************************************************************/
u32 flash_err;
int flash_write(u16 *pflash, u16 *pfrom, int count)
{
	int i;
	flash_err = 0;
	for (i = 0; i < count; i++)
	{
		if (pflash >= (u16*)0x08080000)
		{ // Here maybe 2nd bank
			if (pflash > (u16*)0x080FFFFe)  return -1;
			while ((FLASH_SR2 & 0x1) != 0);	// Wait for busy to go away
			if (flash_unlock2() != 0) return -2;
			FLASH_SR2 = (FLASH_EOP | FLASH_WRPRTERR | FLASH_PGERR); // Clear any error bits
			FLASH_CR2 = FLASH_PG;		// Set program bit
			*pflash++ = *pfrom++;		// Program the 1/2 word
			while ((FLASH_SR2 & 0x1) != 0);	// Wait for busy to go away
			flash_err |= FLASH_SR2;			
		}
		else
		{ // Here maybe 1st bank
			if (pflash < (u16*)0x0800000)  return -3;
			while ((FLASH_SR & 0x1) != 0);	// Wait for busy to go away
			if (flash_unlock() != 0) return -4;
			FLASH_SR = (FLASH_EOP | FLASH_WRPRTERR | FLASH_PGERR); // Clear any error bits
			FLASH_CR = FLASH_PG;		// Set program bit
			*pflash++ = *pfrom++;		// Program the 1/2 word
			while ((FLASH_SR & 0x1) != 0);	// Wait for busy to go away
			flash_err |= FLASH_SR;			
		}
	}	
	if ( (flash_err & (FLASH_WRPRTERR | FLASH_PGERR)) != 0) return -5;	
	return 0;
}
/******************************************************************************
 * int flash_erase(u16* pflash);
 *  @brief 	: Erase one page
 *  @param	: pflash = 1/2 word pointer to address in flash
 *  @param	: pfrom = 1/2 word pointer to address with source data
 *  @return	: 
 *           0 = success
 *          -1 = address greater than 1 MB
 *          -2 = unlock sequence failed for upper bank
 *          -3 = address below start of ram.
 *          -4 = unlock sequence failed for lower bank
 *          -5 = error at some point in the writes, flash_err has the bits
*******************************************************************************/
/*
● Check that no Flash memory operation is ongoing by checking the BSY bit in the
  FLASH_CR register
● Set the PER bit in the FLASH_CR register
● Program the FLASH_AR register to select a page to erase
● Set the STRT bit in the FLASH_CR register
● Wait for the BSY bit to be reset
● Read the erased page and verify
*/
int flash_erase(u16 *pflash)
{
	flash_err = 0;
	if (pflash >= (u16*)0x08080000)
	{ // Here maybe 2nd bank
		if (pflash > (u16*)0x080FFFFe)  return -1;
		while ((FLASH_SR2 & 0x1) != 0);	// Wait for busy to go away
		if (flash_unlock2() != 0) return -2;
		FLASH_CR2 = FLASH_PER;		// Set Page Erase function
		FLASH_AR2 = (u32)pflash;	// Set page address
		FLASH_CR2 |= FLASH_STRT;	// Start erase
		while ((FLASH_SR2 & 0x1) != 0);	// Wait for busy to go away
		FLASH_CR2 &= ~FLASH_PER;	// Remove PER bit
		flash_err |= FLASH_SR2;
	}
	else
	{ // Here maybe 1st bank
		if (pflash < (u16*)0x0800000)  return -3;
		while ((FLASH_SR & 0x1) != 0);	// Wait for busy to go away
		if (flash_unlock() != 0) return -4;
		FLASH_CR = FLASH_PER;		// Set Page Erase function
		FLASH_AR = (u32)pflash;		// Set page address
		FLASH_CR |= FLASH_STRT;		// Start erase
		while ((FLASH_SR & 0x1) != 0);	// Wait for busy to go away
		FLASH_CR &= ~FLASH_PER;		// Remove PER bit
		flash_err |= FLASH_SR;
	}
	if ( (flash_err & (FLASH_WRPRTERR | FLASH_PGERR)) != 0) return -5;
	return 0;
}

