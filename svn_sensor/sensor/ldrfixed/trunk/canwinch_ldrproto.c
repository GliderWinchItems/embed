/******************************************************************************
* File Name          : canwinch_ldrproto.c
* Date First Issued  : 10/09/2014
* Board              : RxT6
* Description        : Loader protocol work
*******************************************************************************/
/*
Notes on timings:
crc - hardware (word by word) v software (byte by byte)--
hardware crc-32: 1E6 bytes of flash -> 3.45 sec
software crc-32: 1E6 bytes of flash -> 17.405 sec

payload--
8 byte payload, 11b CAN ID + interframe -> 139 CAN bits 
 @ 500,000 bits/sec -> 278 usec
1 byte payload, 11b CAN ID + interframe -> 83 CAN bits
 @ 500,000 bits/sec -> 166 usec

One XL flash block = 2048 bytes = 256 8 byte payloads
256 * 278 usec = 71168 usec
 add ACK msg = 166 usec -> 71,334 -> 71.3 ms per block

Plus unknown amount of write flash time.

*/
#include "canwinch_ldrproto.h"
#include "scb.h"
#include "flash_write.h"
//#include "hwcrc.h"
#include "crc-32.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/printf.h"
#include "common_fixedaddress.h"
#include "../../../../svn_common/trunk/db/gen_db.h"

extern struct CAN_CTLBLOCK* pctl1;

static unsigned int debugPctr = 0;

/* Address pointer for storing incoming data */
u8 *padd;		// Points to any address that is reasonable--working address
u8 *padd_start;		// Points to any address that is reasonable--address received
u32 sw_padd = 0;	// 0 = address needs to be set before storing; 1 = OK to store
u32 err_bogus_cmds = 0;	// Counter for commands not classified
u32 err_bogus_cmds_cmds = 0; // Counter for Command code undefined
u32 err_novalidadd = 0;	// Counter for data msg with no valid address
u8* flash_hi; 		// Highest flash address

#define LARGESTFLBLK	512	// Largest flash block (4 byte words)

union FLUN	// Assure aligment, etc.
{
	unsigned long long  ll[LARGESTFLBLK/2];
	u64	u64[LARGESTFLBLK/2];	// 64b (long long)
	u32	u32[LARGESTFLBLK];	// 32b (words)
	u16	u16[LARGESTFLBLK*2];	// 16b (1/2 words)
	u8	u8[LARGESTFLBLK*4];	// bytes
};

struct FLBLKBUF	// Working pointers and counts accompanying data for block
{
	union FLUN fb;	// Size of largest flash block (2048 bytes)
	u8*	base;		// Flash block base address
	u8*	end;		// Flash block end address + 1
	u8*	p;		// Byte pointer in flash block
	u16	sw;		// Write switch: 0 = skip, 1 = write, 2 = erase and write
};

static struct FLBLKBUF flblkbuff;	// Flash block buffer

u32 flashblocksize = 2048;	// 1024, or 2048, 
u32 sw_flash1sttime = 0;	// First time switch for getting flash block

/* CAN msgs */
static struct CANRCVBUF can_msg_cmd;	// Commmand
static struct CANRCVBUF can_msg_rd;	// Read
static struct CANRCVBUF can_msg_wr;	// Write

/* Switch that shows if we have a program loading underway and should not jump to an app */
int ldr_phase = 0;

/******************************************************************************
 * static void can_msg_put(struct CANRCVBUF* pcan);
 * @brief	: send a CAN msg
 * @param	: pcan = pointer to CAN msg
 ******************************************************************************/
static void can_msg_put(struct CANRCVBUF* pcan)
{
	can_driver_put(pctl1, pcan, 4, 0);
}
/******************************************************************************
 * static void sendcanCMD(u8 cmd);
 * @brief	: send a CAN msg with a command byte
 * @param	: cmd = command code
 ******************************************************************************/
static void sendcanCMD(u8 cmd)
{
	
	can_msg_cmd.dlc = 1;
	can_msg_cmd.cd.uc[0] = cmd;
	can_msg_put(&can_msg_cmd);
	return;
}
/* **************************************************************************************
 * static void flbblkbuff_init(void);
 * @brief	: Initialize flash block buffer struct
 * ************************************************************************************** */
static void flbblkbuff_init(void)
{

	/* Assure start address for block is on an even boundary */
	flblkbuff.base = (u8*)((u32)(padd) & (flashblocksize - 1));

	flblkbuff.p = &flblkbuff.fb.u8[0]; // Pointer into flash block
	flblkbuff.sw = 0;	// need to write, or erase and write, switch

	return;
}
/* **************************************************************************************
 * void canwinch_ldrproto_init(u32 iamunitnumber);
 * @brief	: Initialization for loader
 * @param	: Unit number 
 * ************************************************************************************** */
void canwinch_ldrproto_init(u32 iamunitnumber)
{
	padd = (u8*)(0x08000000);	// Set to something that doesn't give a bad address error

	/* Flash block size */
	flashblocksize = (*(u16*)(0x1FFFF7E0)); // Get size of flash in Kbytes
	if (flashblocksize > 256) 
		flashblocksize = 2048;	// XL series flash block size
	else
		flashblocksize = 1024;	// Med, and High series flash block size

	/* Highest flash address plus one = lowest + (flash size(kbytes) * 1024)  */
	flash_hi = (u8*)(0x08000000 + (u32)((*(u16*)(0x1FFFF7E0)) << 10) );

	flbblkbuff_init(); // Set initial pointer.

printf("FLASH BLK SZE: %d K\n\r",flashblocksize);
printf("FLASH HI ADDR: %08X\n\r",flash_hi);

	/* Set fixed part of CAN msgs */
//	can_msg_cmd.id = iamunitnumber | (CAN_EXTRID_DATA_CMD << CAN_EXTRID_SHIFT); // Command
	can_msg_cmd.id = iamunitnumber; // Command
	can_msg_rd.id  = iamunitnumber | (CAN_EXTRID_DATA_RD  << CAN_EXTRID_SHIFT); // Read
	can_msg_wr.id  = iamunitnumber | (CAN_EXTRID_DATA_WR  << CAN_EXTRID_SHIFT); // Write

printf("CAN ID's\n\r");
printf( "CMD: %08X\n\r",can_msg_cmd.id);
printf( "RD : %08X\n\r",can_msg_rd.id);
printf( "WR : %08X\n\r",can_msg_wr.id);

	return;
}
/* **************************************************************************************
 * u16 mv2(u8* p2);
 * @brief	: Convert 2 bytes into a 1/2 word
 * u32 mv4(u8* p2);
 * @brief	: Convert 4 bytes into a word

 * ************************************************************************************** */
u16 mv2(u8* p2){ return ( *(p2+1)<<8 | *p2 ); }
u32 mv4(u8* p2){ return ( *(p2+3)<<24 | *(p2+2)<<16 | *(p2+1)<<8 | *p2 ); }

/* **************************************************************************************
 * static int do_jump(struct CANRCVBUF* p);
 * @brief	: Check and execute a JUMP command
 * @param	: p = pointer to message buffer
 * ************************************************************************************** */
static int do_jump(struct CANRCVBUF* p)
{
	u32 appjump;

	/* Check for correct number of bytes in payload. */
	if (p->dlc != 4) return -1;

	/* Convert byte array to 4 byte unsigned int */
	 appjump = mv4(&p->cd.u8[0]); 

	(*(  (void (**)(void))appjump)  )();	// Indirect jump via vector address

	return 0;	// We should never get here!
}
/* **************************************************************************************
 * void do_dataread(struct CANRCVBUF* p);
 * @brief	: Do something!
 * @param	: Point to message buffer
 * ************************************************************************************** */
void do_dataread(struct CANRCVBUF* p)
{
	u32 i;
	/* We will assume the bozo asking for read has set a valid address.  If the address
           is not valid there might be a hard fault/invalid address crash and a power cycling \
           to recover would be needed. */
	u32 count = p->dlc & 0xf;	// Get the byte count request
	
	if (count > 8)
	{ // In case we got a bogus byte count
		count = 8;
	}
	for (i = 0; i < count; i++)
	{
		p->cd.uc[i] = *padd++;
	}
		can_msg_put(p);	// Set it up for tranmission
	
	return;
}
/* **************************************************************************************
 * static void copypayload(u8* pc, s8 count);
 * @brief	: Copy payload to memory or registers; do 16b or 32b if possible (registers require this)
 * @param	: pc = pointer to payload start byte
 * @param	: count = dlc after masking
 * ************************************************************************************** */
/* This vexing routine takes bytes from the CAN payload, which may not be address aligned for
16b, 32b, 64b, and moves them to the largest of these.  This requires checking the low order
bits of the address and the number of bytes remaining to be moved to determine which size
(16, 32, 64) can be used.  'table' is used to look up the one to use.
*/
const u8 table[16] = {2,4,4,8,2,2,2,2,2,4,4,4,2,2,2,2};
static void copypayload(u8* pc, s8 count)
{
	u32 x;
	if (count < 1 ) return;
	if (count > 8) count = 8;	// JIC	
	count -= 1; // (count = 0 - 7)

	while (count > 0)	// Loop through the payload
	{
		if ( ((count & 0x1) == 0) || ( ((u32)(padd) & 0x1) != 0) ) // Odd byte count and/or odd address requires byte moves
		{ // Here, if either are odd, then we must move one byte at a time.
			*padd++ = *pc++; count -= 1;
		}
		else
		{ // Here both count and address are even 
			x = ( ( ( (u32)padd << 1) & 0xc) | ((count >> 1) & 0x3) );
			x &= 0x0f; 	// jic!
			switch (table[x])
			{
			case 2:	// Move 1/2 words (2 bytes)
				*(u8*)padd = mv2(pc); padd +=2; pc +=2; count -= 2;
				break;

			case 4: // Move words (4 bytes)
				*(u32*)padd = mv4(pc); padd +=4; pc +=4; count -= 4;
				break;

			case 8: // Move double word (8 bytes)
				*(u32*)padd = mv4(pc); padd +=4; pc +=4; // No need to adjust 'count' as we 'return'
				*(u32*)padd = mv4(pc); padd +=4; pc +=4;
				return;
			}
		}
	}
	return;
}
/* **************************************************************************************
 * int addressOK(u8* pa);
 * @brief	: Check that it is legal
 * @param	: Address pointer
 * @return	: 0 = OK, not zero = bad
 * ************************************************************************************** */
int addressOK(u8* pa)
{
	u32 p;
	if ( ((u32)pa > 0x1FFFE000) && ((u32)pa <= 0x1FFFF80F) ) return 0; // Information block
	if (  (u32)pa < 0x08000000) return -1;	// Return below any flash
	p = *(u32*)0x1FFFF7E0;			// Get size of flash in Kbytes
	p = (p << 10); 				// Multiply by 1024 (K)
	if ((u32)pa > p) return -1;		// Above flash for this part
	return 0;
}
/* **************************************************************************************
 * static void wrblk(struct CANRCVBUF* pcan);
 * @brief	: Write RAM buffer out to flash
 * ************************************************************************************** */
static void wrblk(struct CANRCVBUF* pcan)
{
printf("wrblk: %d %X %X %08X %08X\n\r",debugPctr, padd, pcan->dlc, pcan->cd.ui[0], pcan->cd.ui[1]);USART1_txint_send();

	switch (flblkbuff.sw) 
	{
	case 0:	// Need to write or erase & write?
printf("wrblk: CASE 0: no need to write block\n\r");USART1_txint_send();
		break;	// No need to write the block

	case 2:
	case 3: // Erase block, then write
printf("wrblk: CASE 2: erase block before writing\n\r");USART1_txint_send();
		can_msg_cmd.cd.uc[2] = flash_erase((u16*)flblkbuff.base);

	case 1: // Write, but no need for erase
		can_msg_cmd.cd.uc[1] = flash_write( (u16*)flblkbuff.base, &flblkbuff.fb.u16[0], flashblocksize );
printf("wrblk: CASE 1: writing block\n\r");USART1_txint_send();

if ((can_msg_cmd.cd.uc[1] != 0) || (can_msg_cmd.cd.uc[2] != 0))
 printf("wrblk: pay[1] %X pay[2] %X\n\r",can_msg_cmd.cd.uc[1], can_msg_cmd.cd.uc[2]);USART1_txint_send();
		break;	
	default:
printf("wrblk: default: %d\n\r",flblkbuff.sw);USART1_txint_send();
	}
	return;
}
/* **************************************************************************************
 * static void flashblockinit(void);
 * @brief	: Read in flash block to sram buffer
 * ************************************************************************************** */
static void flashblockinit(void)
{
	u64* pt1;	// Beginning address of flash block
	u64* pt2;	// Beginning address of RAM buff
	u64* ptend;	// End+1 of RAM buff

	/* Load block that padd points to into RAM. */
	pt1 = (u64*)((u32)(padd) & ~(flashblocksize - 1)); 	// Block addr of STM32 memory to be loaded
	flblkbuff.base = (u8*)pt1;				// Pointer to beginning of flash block
	flblkbuff.end = flblkbuff.base + flashblocksize;	// End + 1
	flblkbuff.p = &flblkbuff.fb.u8[0] + (padd - (u8*)pt1); 	// Pointer to where payload begins storing
	flblkbuff.sw = 0;					// Switch for write/erase
	pt2 = &flblkbuff.fb.u64[0];				// RAM buffer for block
	ptend = pt2 + (flashblocksize/sizeof(u64)); 		// End+1 of RAM buffer
//printf("P0: %X %X %X\n\r",pt1, pt2, ptend);USART1_txint_send();
	while (pt2 < ptend) *pt2++ = *pt1++; 			// Copy flash to RAM buffer
//printf("P1: %X %X %X\n\r",pt1, pt2, ptend);USART1_txint_send();
	return;
}
/* **************************************************************************************
 * static void writepayloadf1(u8* pc, s8 count,struct CANRCVBUF* p);
 * @brief	: Load payload into RAM buffer
 * @param	: pc = pointer to payload start byte
 * @param	: count = dlc after masking
 * @param	: p = CAN msg pointer (for printf & debugging)
 * ************************************************************************************** */
static void writepayloadf1(u8* pc, s8 count,struct CANRCVBUF* p)
{
	while (count > 0)	// Loop through the payload
	{	
		/* Is this address for a different block? */
		if (((u32)padd & ~(flashblocksize - 1)) != (u32)flblkbuff.base)
		{ // Here, the load address crossed a flash boundary
printf("padd crossed block boundary: %08X  padd: %08x\n\r",(u32)flblkbuff.base, padd);USART1_txint_send();
			wrblk(p);		// Write RAM buff to flash
			flashblockinit();	// Read in new flash block and init some pointers.
		}

		/* Store one, measly, single, but valuable, byte */
		if (*pc != *padd) // Is new byte different from byte in the flash location?
		{ // Here, yes.  Flag for writing and mabye even erasing
			ldr_phase |= 0x1;	// Show we are going write flash so don't jump to the app unless commanded
			flblkbuff.sw |= 0x1;	// Show that we have to write (but may or may not need erase)
			if ( (*(u16*)((u32)padd & ~0x1)) != 0xffff)	// Is the 1/2 word already in an erased condtion?
				flblkbuff.sw |= 0x2;	// No, show that we have to erase before write
		}
		*flblkbuff.p++ = *pc++;	// Store this golden byte in the ram block buffer
		padd++;			// Advance where the next byte would be placed in flash.
		count -= 1;		// CAN payload byte counter 
	}
	return;
}

/* **************************************************************************************
 * void do_datawrite(u8* pc, s8 count,struct CANRCVBUF* p);
 * @brief	: Move payload to flash RAM buffer, or RAM, or somewhere...
 * @param	: pc = pointer to payload start byte
 * @param	: count = dlc after masking
 * @param	: p = CAN msg pointer (for printf & debugging)
 * ************************************************************************************** */
/* NOTE: The system block is in flash and is not in the flash address range, so copying it as a
memory address will not change it.  (You shouldn't be messing with it anyway!) */
void do_datawrite(u8* pc, s8 count,struct CANRCVBUF* p)
{

	if (count > 8) 			// Return if count out of range
		{sendcanCMD(LDR_NACK);
printf("NACK0: %d %X %d %X %08X %08X\n\r",debugPctr, padd, count, p->dlc, p->cd.ui[0], p->cd.ui[1]);USART1_txint_send();
return;}

	// Return = No valid address in place
	if (sw_padd == 0) { err_novalidadd += 1; sendcanCMD(LDR_NACK);
printf("NAC1K: %d %X %d %X %08X %08X\n\r",debugPctr, padd, count, p->dlc, p->cd.ui[0], p->cd.ui[1]);USART1_txint_send();
return;}	
	
	/* Is the address is within the flash bounds.  */
	if ( ((u32)padd >= 0x08000000) && ((u32)padd < (u32)flash_hi) ) 
	{ // Here, it is flash

		writepayloadf1(pc, count,p);
	}
	else
	{ // Here, not flash address.  Pray that it is a valid address
		copypayload(pc, count);
	}
debugPctr += 1;
	return;
}
/* **************************************************************************************
 * void do_wrblk(struct CANRCVBUF* pcan);
 * @brief	: Write RAM buffer out to flash
 * ************************************************************************************** */
void do_wrblk(struct CANRCVBUF* pcan)
{
	wrblk(pcan);	// Write block to flash

	/* Let PC know the flash write is complete. Send status in payload. */
	can_msg_cmd.dlc = 3;
	can_msg_cmd.cd.uc[0] = LDR_WRBLK;
	can_msg_put(&can_msg_cmd);		// Place in CAN output buffer

	return;
}
/* **************************************************************************************
 * void do_crc(struct CANRCVBUF* p);
 * @brief	: Compute CRC and send it back
 * @param	: Point to message buffer holding the precious command
 * ************************************************************************************** */
void do_crc(struct CANRCVBUF* p)
{
	u32 pstart;
	u32 count;
	u32 crc32;

	/* Check that the payload is the correct size: start|end addresses */
	if ((p->dlc & 0x0f) != 8)
	{ // Send NACK
printf("Send NACK: dlc not 8: %d\n\r",p->dlc);USART1_txint_send();
		 return;
	}
	/* Get start and end addresses from payload */
	count  = (p->cd.ui[0] >> 8);		// Number of bytes to check
	count  &= 0x000fffff; 	// Limit the size
	pstart = p->cd.ui[1];	// Address to start

	
	/* Check that we are dealing with valid start address and count */
	extern u8* __appoffset;	// Origin of application define in .ld file
	if ((pstart < (u32)(&__appoffset)) || ((u32)(&__appoffset + count) > (u32)flash_hi)) 
	{ // Send NACK
printf("Send NACK start addr out of range: %08X %X %08X %08X\n\r",pstart, count,(unsigned int)(&__appoffset + count), flash_hi);USART1_txint_send();
		return;
	}

if ( (pstart < (u32)0x08004000) ||  ( (pstart + count) > (u32)0x08100000) )
   {printf("Hard Coded address check out-of-range:"); return;}

printf("rc_crc32: %08X %08X %08X\n\r",pstart, (pstart+count), count);USART1_txint_send();
	/* Compute the crc and place in msg to be returned. */
	crc32 = rc_crc32((unsigned char*)pstart, count );
printf("Send crc-32: %08X\n\r",crc32);USART1_txint_send();USART1_txint_send();

	// Return msg with crc
	p->dlc = 8;
	p->cd.ui[1] = crc32;
	p->cd.uc[0] = LDR_CRC;
	can_msg_put(p);		// Place in CAN output buffer

	return;
}
/* **************************************************************************************
 * void do_set_addr(struct CANRCVBUF* p);
 * @brief	: Check payload and send response as two bytes (command + ACK/NACK)
 * @param	: Point to message buffer holding the precious command
 * ************************************************************************************** */
void do_set_addr(struct CANRCVBUF* p)
{
	u8* ptmp;

	if ((p->dlc & 0x0f) == 5) // Payload size: cmd byte + 4 byte address
	{ // 
ldr_phase |= 0x1; // Stop ldr.c from jumping to the app.
		ptmp = (u8*)mv4(&p->cd.uc[1]);	// Extract address from payload
		if (addressOK(ptmp) == 0)	// Valid STM32 address?
		{ // Here, yes.  It shouldn't cause a memory fault
			p->cd.uc[1] = LDR_ACK;	// Show it passed all checks
			padd = ptmp;		// Save working pointer
			padd_start = padd;	// Save start
			sw_padd = 1;		// Show it was "recently set"			
			flashblockinit();	//Load block that padd points to into RAM.
			p->cd.uc[1] = LDR_ACK;
debugPctr = 0;
		}
		else
			p->cd.uc[1] = LDR_ADDR_OOB; // Failed the address check					
	}
	else
	{ // Here, dlc size wrong
		sw_padd = 0;	// Don't be storing stuff in bogus addresses
		p->cd.uc[1] = LDR_DLC_ERR;			
	}
	/* Send response */
	p->dlc = 2;
	can_msg_put(p);	// Place in CAN output buffer
	return;
}
/* **************************************************************************************
 * void do_flashsize(struct CANRCVBUF* p);
 * @brief	: Send flashsize
 * @param	: Point to message buffer holding the imperial command
 * ************************************************************************************** */
void do_flashsize(struct CANRCVBUF* p)
{
	p->dlc = 3;	// Command plus short
	p->cd.uc[1] = flashblocksize;	
	p->cd.uc[2] = flashblocksize >> 8;
printf("X: flashblocksize: %X %X %X %X\n\r",flashblocksize, p->cd.uc[0],p->cd.uc[1],p->cd.uc[2]);USART1_txint_send();
	can_msg_put(p);	// Place in CAN output buffer
	return;
}
/* **************************************************************************************
 * void do_rd4(struct CANRCVBUF* p);
 * @brief	: Send 4 bytes from starting address contained in payload
 * @param	: Point to message buffer holding the imperial command
 * ************************************************************************************** */
void do_rd4(struct CANRCVBUF* p)
{
	u8* rdaddr = (u8*)mv4(&p->cd.uc[1]); // Get bytes 1-4 into a word
	p->dlc = 5;	// Command plus char*
	p->cd.uc[1] = *rdaddr++;
	p->cd.uc[2] = *rdaddr++;
	p->cd.uc[3] = *rdaddr++;
	p->cd.uc[4] = *rdaddr;
printf("R4: read addr: %X %X %X %X %X %X\n\r",(u32)rdaddr, p->cd.uc[0],p->cd.uc[1],p->cd.uc[2],p->cd.uc[3],p->cd.uc[4]);USART1_txint_send();
	can_msg_put(p);	// Place in CAN output buffer
	return;
}
/* **************************************************************************************
 * void do_getfromdaddress(struct CANRCVBUF* p, u8* rdaddr);
 * @brief	: Return msg with 4 bytes in payload uc[1-4] from address d
 * @param	: p = pointer to message buffer holding the imperial command
 * @param	: rdaddr = address to use 
 * ************************************************************************************** */
void do_getfromdaddress(struct CANRCVBUF* p, u8* rdaddr)
{
	if (addressOK(rdaddr) != 0)
	{printf("do_getfromdaddress: addr not OK: %08X\n\r",rdaddr);USART1_txint_send(); return;}

	p->dlc = 5;	// Command plus addr
	p->cd.uc[1] = *rdaddr++;
	p->cd.uc[2] = *rdaddr++;
	p->cd.uc[3] = *rdaddr++;
	p->cd.uc[4] = *rdaddr;
printf("GETADDR: read addr: %X %X %X %X %X\n\r", p->cd.uc[0],p->cd.uc[1],p->cd.uc[2],p->cd.uc[3],p->cd.uc[4]);USART1_txint_send();
	can_msg_put(p);	// Place in CAN output buffer
	return;
}
/* **************************************************************************************
 * void do_send4(struct CANRCVBUF* p, u32 n);
 * @brief	: Send 4 bytes
 * @param	: Point to message buffer holding the imperial command
 * @param	: n = 4 byte number
 * ************************************************************************************** */
void do_send4(struct CANRCVBUF* p, u32 n)
{
	p->dlc = 5;	// Command plus char*
	p->cd.uc[1] = n;
	p->cd.uc[2] = n >> 8;
	p->cd.uc[3] = n >> 16;
	p->cd.uc[4] = n >> 24;
printf("send4: %X %X %X %X %X %X\n\r", n, p->cd.uc[0],p->cd.uc[1],p->cd.uc[2],p->cd.uc[3],p->cd.uc[4]);USART1_txint_send();
	can_msg_put(p);	// Place in CAN output buffer
	return;
}
/* **************************************************************************************
 * void do_getflashpaddr(struct CANRCVBUF* p);
 * @brief	: Return msg number of crc blocks, and address to array of blocks
 * @param	: p = pointer to message buffer holding the imperial command
 * ************************************************************************************** */
extern u8* __appoffset;
extern u8* __highflashp;
extern u8* __highflashlayout;

void do_getflashpaddr(struct CANRCVBUF* p)
{
	p->dlc = 8;	// Command plus addr

	extern void* __appjump;	// Defined in ldr.ld file
	u32* ppflashp = (u32*)((u32)((u8*)*&__appjump + 7 + 0));	// Points to "size"
	
	u32 size = *ppflashp;		// Get size
	u32 pflashp = *(ppflashp + 1);	// Get pointer to FLASHP and beginning of app struct

	if (size > 0x000fffff) size = 0x000fffff; // Only 3 bytes allowed for size (and < 1 MB)

	/* Payload [0] = cmd code; [1] - [3] = size; [4] - [7] = FLASHP address */
	u8 tmp = p->cd.uc[0];
	p->cd.ui[0] = (size << 8);	
	p->cd.uc[0] = tmp;
	p->cd.ui[1] = pflashp;

int i;printf("GET FLASHP addr ");
for (i = 0; i < 8; i++) printf(" %X",p->cd.uc[i]); 
printf("\n\r"); USART1_txint_send();

	can_msg_put(p);	// Place in CAN output buffer
	return;
}


/* **************************************************************************************
 * void do_cmd_cmd(struct CANRCVBUF* p);
 * @brief	: Do something!
 * @param	: p = pointer to message buffer
 * ************************************************************************************** */
void do_cmd_cmd(struct CANRCVBUF* p)
{
/*	
	#define LDR_SET_ADDR	1	// 5 Set address pointer (not FLASH) (bytes 2-5):  Respond with last written address.
	#define LDR_SET_ADDR_FL	2	// 5 Set address pointer (FLASH) (bytes 2-5):  Respond with last written address.
	#define LDR_CRC		3	// 8 Get CRC: 2-4 = count; 5-8 = start address 
	#define LDR_ACK		4	// 1 ACK: Positive acknowledge (Get next something)
	#define LDR_NACK	5	// 1 NACK: Negative acknowledge (So? How do we know it is wrong?)
	#define LDR_JMP		6	// 5 Jump: to address supplied (bytes 2-5)
	#define LDR_WRBLK	7	// 1 Done with block: write block with whatever you have.
	#define LDR_RESET	8	// 1 RESET: Execute a software forced RESET
	#define LDR_XON		9	// 1 Resume sending
	#define LDR_XOFF	10	// 1 Stop sending
	#define LDR_FLASHSIZE	11	// 1 Get flash size; bytes 2-3 = flash block size (short)
	#define LDR_ADDR_OOB	12	// 1 Address is out-of-bounds
	#define LDR_DLC_ERR	13	// 1 Unexpected DLC
	#define LDR_FIXEDADDR	14	// 5 Get address of flash with fixed loader info (e.g. unique CAN ID)
	#define LDR_RD4		15	// 5 Read 4 bytes at address (bytes 2-5)
	#define LDR_APPOFFSET	16	// 5 Get address where application begins storing.
	#define LDR_HIGHFLASHH	17	// 5 Get address of beginning of struct with crc check and CAN ID info for app
	#define LDR_HIGHFLASHP	18	// 5 Get address of beginning of struct with app calibrations, parameters, etc.
	#define LDR_ASCII_SW	19	// 2 Switch mode to send printf ASCII in CAN msgs
	#define LDR_ASCII_DAT	20	// 3-8  1=line position;2-8=ASCII chars
	#define LDR_WRVAL_PTR	21	// 2-8 Write: 2-8=bytes to be written via address ptr previous set.
	#define LDR_WRVAL_AI	22	// 8 Write: 2=memory area; 3-4=index; 5-8=one 4 byte value

*/
//printf("Q: %02X\n\r",p->cd.uc[0]); USART1_txint_send(); // Debug: Display command codes coming in
	/* Here, we have a command in the ID field. */
	switch (p->cd.uc[0])	// Command code
	{
	case LDR_SET_ADDR: // Set address pointer (bytes 2-5):  Respond with last written address.
		do_set_addr(p);
		break;

	case LDR_ACK:		// ACK: Positive acknowledge
		break;

	case LDR_NACK:		// NACK: Negative acknowledge
		break;

	case LDR_JMP:		// Jump: to address supplied
		do_jump(p);
		break;

	case LDR_WRBLK:		// Done with block: write block with whatever you have..
		do_wrblk(p);
		break;

	case LDR_RESET:		// RESET: Execute a software forced RESET for this unit only.
		SCB_AIRCR  |= (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
		break;

	case LDR_CRC:		// Get CRC: given count | start address, compute CRC and respond with it.
		do_crc(p);
		break;	
	
	case LDR_XON:		// Resume sending
		break;

	case LDR_XOFF:		// Stop sending: response to our sending LDR_XOFF msg.
		break;

	case LDR_FLASHSIZE:	// Send flash size
		do_flashsize(p);
		break;

	case LDR_FIXEDADDR:	// Send address ahead of fixed address block
		do_getfromdaddress(p, (u8*)0x08000004);
		break;

	case LDR_RD4:		// Send address of fixed param flash area
		do_rd4(p);
		break;

	case LDR_APPOFFSET:	// Send address of where app loads
		do_send4(p, (u32)&__appoffset);
		break;

	case LDR_HIGHFLASHH:	// Send address of high flash area
		do_send4(p, (u32)&__highflashlayout);
		break;

	case LDR_HIGHFLASHP:	// Get address of beginning of crc check info for app
		do_getflashpaddr(p);
		break;

	case LDR_WRVAL_PTR:	// Write: 2-8=bytes to be written via address ptr previous set
		// Write data starting at payload[1], dlc holds byte ct: 1-7
		do_datawrite(&p->cd.uc[1], (0x7f & p->dlc) - 1, p);
		sendcanCMD(LDR_ACK);
		break;

	default:		// Not a defined command
		err_bogus_cmds_cmds += 1;
printf("BOGUS CMD CODE: %X %08X %X  %08X %08X\n\r",p->cd.uc[0], p->id, p->dlc, p->cd.ui[0], p->cd.ui[1]);USART1_txint_send();
		break;
	}
	return;
}
/* **************************************************************************************
 * void canwinch_ldrproto_poll(void);
 * @param	: pctl = pointer control block for CAN module being used
 * @brief	: If msg is for this unit, then do something with it.
 * ************************************************************************************** */
void canwinch_ldrproto_poll(void)
{
	struct CANRCVBUF* pcan;

	/* Check incoming msgs. */
	while ((pcan = can_driver_peek0(pctl1)) != NULL) // RX0 have a msg?
	{ // Here we have a FIFO0 CAN msg
//if ((pcan->id & 0x0000000e) == 0x0000000c) 
//	printf ("X: %08X %d %08X %08X\n\r",pcan->id,pcan->dlc,pcan->cd.ui[0],pcan->cd.ui[1]);
//printf ("A: %08X\n\r",pcan->id);


		/* Select msg if matches our unit CAN ID  */
		if ( (pcan->id & 0x0ffffffe) == fixedaddress.canid_ldr)
		{ // Here CAN ID is for this unit/loader
//printf ("X: %08X %d %08X %08X\n\r",pcan->id,pcan->dlc,pcan->cd.ui[0],pcan->cd.ui[1]);
//$			do_cmd_cmd(pcan);		// Execute command 
		}
		can_driver_toss0(pctl1);	// Release buffer block
	}
	/* Dump FIFO1 msgs. */
	while ((can_driver_peek1(pctl1)) != NULL)	// RX1 have a msg?
		can_driver_toss1(pctl1); 	// Release buffer block
	return;
}


