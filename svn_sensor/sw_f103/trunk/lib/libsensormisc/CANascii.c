/******************************************************************************
* File Name          : CANascii.c
* Date First Issued  : 11/17/2014
* Board              : STM32F103
* Description        : Send 'putc' chars to CAN msgs 
*******************************************************************************/
/*
This routine depends on vector.c having a jump address to a jump ahead of a table that
has the uniquie CAN id stored.

*/
#include "CANascii.h"
#include "db/gen_db.h"
#include "libusartstm32/usartallproto.h"

#define CANRCVBUFSIZE	32
static struct CANRCVBUF canascii[CANRCVBUFSIZE];	// CAN message circular buffer
static int canbufIDXm = 0;		// Index for removing data from buffer
static int canbufIDXi = 0;		// Index for adding data

static u8* pcas1 = &canascii[0].cd.uc[2];	// Pointer to payload loading
static int canputc_sw = 0;		// 0 = Don't send ascii CAN msgs
static u32 canid_ldr;

/******************************************************************************
 * static int adv_index(int idx, int size)
 * @brief	: Format and print date time in readable form
 * @param	: idx = incoming index
 * @param	: size = number of items in FIFO
 * return	: index advanced by one
 ******************************************************************************/
static int adv_index(int idx, int size)
{
	int localidx = idx;
	localidx += 1; if (localidx >= size) localidx = 0;
	return localidx;
}
/***** ######## UNDER INTERRUPT ###########  **********************************
 * struct CANRCVBUF* CANascii_send(void);
 * @brief	: Enter from systickLOpriority3X_ptr pointer
 * @return	: Zero = no more msgs; not zero = msgs remain to be sent
 ******************************************************************************/
extern struct CAN_CTLBLOCK* pctl1;

struct CANRCVBUF* CANascii_send(void)
{
	struct CANRCVBUF* pcan = CANascii_get();
	if (pcan == 0 ) return 0;
	can_driver_put(pctl1, pcan, 1, 0);
	return pcan;
}
/******************************************************************************
 * void CANascii_init(void);
 * @brief	: Initialize
 ******************************************************************************/
void CANascii_init(void)
{
	int i;
	/* Get CAN ID from ldr.c (unique unit CAN ID) and set data field code. */
	u32* vectjmp = (u32*)0x08000004;	// Loader reset vector address
	u32* tblptr = (u32*)((u8*)(*(u32*)vectjmp) + 7);
	canid_ldr = *tblptr;
//	canid_ldr = canid_ldr | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);	// Unit loader Command
printf(  "LDR CAN IDcmd: %08X\n\r", canid_ldr );	USART1_txint_send(); // Hapless Op msg
	/* Fill buffer with CAN ID and command code */
	for (i = 0; i < CANRCVBUFSIZE; i++)
	{
		canascii[i].id = canid_ldr;		// CAN ID
		canascii[i].cd.uc[0] = LDR_ASCII_DAT;	// Command code
	}
	pcas1 = &canascii[0].cd.uc[2];	// Pointer to payload loading
//	systickLOpriority3X_ptr = &CANascii_send;	// Callbacks to send buffered msgs
	return;
}
/******************************************************************************
 * void CANascii_poll(struct CANRCVBUF* pcan);
 * @brief	: Retreive CAN msgs from 
 * @param	: Pointer to CAN msg
 ******************************************************************************/
void CANascii_poll(struct CANRCVBUF* pcan)
{
	// Is this for us?
	if (pcan->id == canid_ldr)
	{ // Here, yes.  Is it the SWITCH command?
		if (pcan->cd.uc[0] == LDR_ASCII_SW) // Switch msg?
		{ // Here, CAN msg has switch ON/OFF setting.
			canputc_sw = pcan->cd.uc[1];	// Set switch
		}
printf("SW msg %08X %02X %02X\n\r",pcan->id, pcan->cd.uc[0], pcan->cd.uc[1]);USART1_txint_send();
	}
	return;
}
/******************************************************************************
 * void CANascii_putc(char c);
 * @brief	: Put chars from putc into circular CAN msg buffer
 ******************************************************************************/
void CANascii_putc(char c)
{
	if (canputc_sw != 0xA5) return;	// Return if not in "send" mode?
//	if (c == '\r') return;		// Ignore these
	*pcas1++ = c;			// Store char in payload
//USART1_txint_putc(c);
	if ((c == '\n') || (pcas1 > &canascii[canbufIDXi].cd.uc[7]))
	{ // Here, newline, or payload full]
//USART1_txint_send();
		canascii[canbufIDXi].dlc = pcas1 - &canascii[canbufIDXi].cd.uc[0]; // Payload size
		canbufIDXi = adv_index(canbufIDXi, CANRCVBUFSIZE); // Advance index in buffer
		pcas1 = &canascii[canbufIDXi].cd.uc[1];	// Reset payload ptr for next msg
	}
	return;
}
/***** ######## UNDER INTERRUPT ###########  **********************************
 * struct CANRCVBUF* CANascii_get(void);
 * @brief	: Get pointer CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
struct CANRCVBUF* CANascii_get(void)
{
	struct CANRCVBUF *p;
	if (canbufIDXi == canbufIDXm) return 0;	// Return showing no new data
	p = &canascii[canbufIDXm];	// Get return pointer value
	canbufIDXm = adv_index(canbufIDXm, CANRCVBUFSIZE);
	return p;			// Return pointer to buffer
}

