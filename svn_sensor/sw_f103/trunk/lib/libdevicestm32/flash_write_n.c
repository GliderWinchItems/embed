/******************************************************************************
* File Name          : flash_write_n.c
* Date First Issued  : 07/28/2013
* Board              : ../svn_sensor/hw/trunk/eagle/f103R/RxT6
* Description        : write 'n' bytes to flash (crosses block boundaries)
*******************************************************************************/

#include "flash_write_n.h"

// Debugging w serial port
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/printf.h"

/******************************************************************************
 * int flash_write_n(u8* pflash, u8* psrc, u32 n);
 *  @brief 	: Write 'n' bytes to flash
 *  @param	: pstart = pointer to flash 
 *  @param	: psrc = pointer to source bytes
 *  @param	: n = number of bytes
 *  @return	: zero = success; not zero = failed
*******************************************************************************/
int flash_write_n(u8* pflash, u8* psrc, u32 n)
{
	u32 flashblocksize;
	u8* flashblockbase;
	u64* pb64;
	u64* pf64;
	u64* pe64;
	u8* ps8;
	u8* pe8;
	u32 flashincrement;
	union BLOCK {
		u8   u8[8];		
		u16 u16[4];
		u32 u32[2];
		u64 u64;
	}block[2048/8];	// Allow for largest flash block
		
	int ret1 = 0;
	int ret2 = 0;
	int err1 = 0;
	int err2 = 0;
	int sw;

	/* Flash block size */
	flashblocksize = (*(u16*)(0x1FFFF7E0)); // Get size of flash in Kbytes
	if (flashblocksize > 256) 
		flashblocksize = 2048;	// XL series flash block size
	else
		flashblocksize = 1024;	// Med, and High series flash block size

	flashblockbase = (u8*)((u32)pflash & ~(flashblocksize - 1)); // Get block base address
//printf("flashblockbase: %08x, size: %d pflash: %08X\n\r",(u32)flashblockbase, flashblocksize, (u32)pflash);	USART1_txint_send();
	while (n > 0 )
	{
		/* Copy existing block into SRAM buffer using long longs */
		pb64 = &block[0].u64;			// SRAM buffer beginning
		pe64 = &block[flashblocksize/8].u64;	// SRAM buffer end pointer
		pf64 = (u64*)flashblockbase;		// flash pointer
//printf("pb64: %08X pe64: %08X pf64: %08X %d\n\r",(u32)pb64, (u32)pe64, (u32)pf64,n);USART1_txint_send();
		while (pb64 < pe64) *pb64++ = *pf64++;

		/* Point to where to start within the SRAM buffer */
		ps8 = &block[0].u8[0] + (pflash - (u8*)flashblockbase);
		sw = 0;	// Switch for erase/write/skip

		/* Copy source data into SRAM buffer, noting differences. */
		pe8 = &block[flashblocksize/8].u8[0];	// SRAM buffer end pointer
		flashincrement = pe8 - ps8;

//printf("ps8:  %08X pe8:  %08X psrc: %08X, %d\n\r",(u32) ps8, (u32)pe8, (u32)psrc, n);USART1_txint_send();

		while ( (ps8 < pe8) && (n > 0) ) // Stop: end of block, or no more input bytes
		{
			if (*ps8 != *psrc)	// Will new data cause a change?
			{ // Here a change in flash will be required
				// Is the half-word for this byte already erased?
				if ( *( (u16*)((u32)ps8 & ~0x1) )  == (u16)0xffff)
					sw |= 0x1;	// Yes, no erase of block needed
				else
					sw |= 0x2;	// No, block erase required
				*ps8 = *psrc;	// Update SRAM buffer
			}
			ps8++; psrc++; n -= 1;	// Adv pointers and dec byte count
		}
//printf("ps8:  %08X pe8:  %08X psrc: %08X, %d\n\r",(u32) ps8, (u32)pe8, (u32)psrc, n);USART1_txint_send();
		/* Skip, or write, or erase then write. */
		switch (sw) 
		{
		case 0:	// Need to write or erase & write?
printf("flash_write_n: skip block\n\r");	USART1_txint_send();	
			break;	// No need to write the block
		case 1:
		case 2:
		case 3: // Erase block, then write
			ret1 = flash_erase((u16*)flashblockbase);
			// Fall through after erase
//		case 1: // Write, but no need for block erase
			ret2 = flash_write( (u16*)flashblockbase, &block[0].u16[0], flashblocksize/2 );
			break;	
		}
	
		/* Update pointers */
		flashblockbase += flashblocksize;
		pflash += flashincrement;
//printf("flashblockbase: %08x, size: %d pflash: %08X\n\r",(u32)flashblockbase, flashblocksize, (u32)pflash);	USART1_txint_send();	
		if (ret1 != 0) err1 += 1;	// Count erase errors
		if (ret2 != 0) err2 += 1;	// Count write errors
if (ret2 != 0) {printf("write err: %d\n\r",ret2);USART1_txint_send();}
	}
		
	return ( (err1 << 16) | (err2 & 0xffff) );
}

