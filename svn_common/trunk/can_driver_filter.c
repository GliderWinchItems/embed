/******************************************************************************
* File Name          : can_driver_filter.c
* Date First Issued  : 05-30-2015
* Board              : F103 or F4
* Description        : CAN routines common for F103 and F4 CAN drivers: filter
*******************************************************************************/
/*
NOTE: When changing filters CAN rcv is disabled.


*/
#include <stdint.h>

#ifdef STM32F3

#include "../../../git_discoveryf3/lib/libopencm3/stm32/gpio.h"
#include "../../../git_discoveryf3/lib/libopencm3/stm32/rcc.h"
#include "../../../git_discoveryf3/lib/libopencm3/stm32/can.h"
#include "../../../git_discoveryf3/lib/libmiscf3/f3DISCpinconfig.h"
#include "../../../git_discoveryf3/lib/libopencm3/stm32/f3/nvic.h"

#else

#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/can.h"
#include "../../../../svn_common/trunk/common_can.h"

#endif

#include "can_driver_filter.h"

#define DUMMY 0xfffffffc	// 32b CAN ID that is not used

/* Maintain the largest filter register than has been loaded. */
static u32 filtnum1 = 0;	// CAN1 (default start number)
static u32 filtnum2 = 14;	// CAN2 (default start number 'F4)
static u32 filtcan2sb = 14;	// Start Bank number for CAN2

/* 32b ID-ID filter banks are in pairs, so keep track of *next* available odd/even slot. */
static u32 oe = 0;	// odd/even CAN ID for register pair--0 = even, 1 = odd.

/******************************************************************************
 * int can_driver_filter_deactivate_one(u8 filtbank);
 * @brief 	: Deactivate one filter
 * @param	: filtbank = ( 0 to < 28d)
 * @param	: filttype = 
 * @return	:  0 = success; 
 *		: -1 = filter number out of range
*******************************************************************************/
int can_driver_filter_deactivate_one(u8 filtbank)
{
	if ((filtbank > 27) != 0) return -1;

	/* CAN filter master register */
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 1) CAN_FMR(CAN1)  |= CAN_FMR_FINIT;	// Set initialization mode ON for setting up filter banks
	CAN_FA1R(CAN1) &= ~(1 << filtbank);
	CAN_FA1R(CAN1) |=  (1 << filtbank);
	/* Remove filter registers from initialization mode */
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 0) CAN_FMR(CAN1)  &= ~CAN_FMR_FINIT;	// FINIT = 0; Set initialization mode off for filter banks p 665
	return 0;
}

/******************************************************************************
 * void can_driver_filter_deactivate_all(void);
 * @brief 	: 
 * @param	: filtbank = boundary number ( 0 to < 28d)
*******************************************************************************/
void can_driver_filter_deactivate_all(void)
{
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 1) CAN_FMR(CAN1)  |= CAN_FMR_FINIT;	// Set initialization mode ON for setting up filter banks
	CAN_FA1R(CAN1) = 0;
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 0) CAN_FMR(CAN1)  &= ~CAN_FMR_FINIT;	// FINIT = 0; Set initialization mode off for filter banks p 665
	return;
}

/******************************************************************************
 * int can_driver_filter_setCAN2boundary(u8 filtbank);
 * @brief 	: Change CAN2 filter number boundary default (15) to 'filtbank'
 * @param	: filtbank = boundary number ( 0 to < 28d)
 * @return	:  0 = success; -1 = filter number out of range
*******************************************************************************/
/* 
For F103 (not connectivity series) the boundary has no meaning and the number of
  filter slots is 14.
For F3 & F4 the boundary sets the filter number where filters apply to CAN2.  The 
  boundary is default 15.  
Setting to 0 means no filters assigned to CAN1.
Setting to 28d means all filters assigned to CAN1.

*/
int can_driver_filter_setCAN2boundary(u8 filtbank)
{
	if ((filtbank > 28) != 0) return -1;

	/* CAN filter master register p 665 */
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 1) CAN_FMR(CAN1)  |= CAN_FMR_FINIT;	// Set initialization mode ON for setting up filter banks

	CAN_FMR(CAN1)  &= ~(31 << 8);		// Clear existing boundary
	CAN_FMR(CAN1)  |= ~( (filtbank & 31) << 8); // Insert new boundary
	filtnum2 = filtbank;		// Working bank number
	filtcan2sb = filtbank;		// Update CAN2 start number

	/* Remove filter registers from initialization mode */
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 0) CAN_FMR(CAN1)  &= ~CAN_FMR_FINIT;	// FINIT = 0; Set initialization mode off for filter banks p 665
	return 0;
}
/******************************************************************************
 * int can_driver_filter_insert(struct CANFILTERPARAM* p);
 * @brief 	: Insert a filter AND set it active
 * @param	: p = pointer to struct with filter parameters
 * @return	:  0 = success
 *		: -1 = filter number out of range
 *		: -2 = filttype code out of range
*******************************************************************************/
/*
NOTE: Caller has to keep track of filter numbers.  This routine inserts the arguments
  without regard to prior use and therefore could overwrite 

*/
int can_driver_filter_insert(struct CANFILTERPARAM* p)
{
	if (p->filtbank > 27) return -1;	// Out of range
	if (p->filttype >  7) return -2;	// Out of range

	/* Extract bits */
	u8 fifo  = (p->filttype >> 2) & 1;	// FIFO 0|1
	u8 scale = (p->filttype >> 1) & 1; 	// 16b|32b mode
	u8 mode  = (p->filttype >> 0) & 1;	// mask-id or id-id

	/* CAN filter master register */
	// Set initialization mode ON for setting up filter banks
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 1) CAN_FMR(CAN1)  |= CAN_FMR_FINIT;	
	
	/* Set assigned filter to FIFO 1 or 2. */
	CAN_FFA1R(CAN1) &= ~(   1 << p->filtbank);	// Clear old bit
	CAN_FFA1R(CAN1) |=  (fifo << p->filtbank);	// Set new bit

	/* Set 16 or 32 bit mode */
	CAN_FS1R(CAN1) &= ~(    1 << p->filtbank);	// Clear old bit
	CAN_FS1R(CAN1) |=  (scale << p->filtbank);	// Set new bit

	/* Set mode (id mask) or (id id) */
	CAN_FM1R(CAN1) &= ~(   1 << p->filtbank);	// Clear old bit
	CAN_FM1R(CAN1) |=  (mode << p->filtbank);	// Set new bit

	/* Insert ID and ID/MASK into filter bank. */
	if (scale == 0)
	{ // Here, 16b dual filtering
		if (p->odd != CANFILT_ODD)
		{ // Here, use even 32b register
			CAN_FiR1(CAN1, p->filtbank) = (p->id << 16) | (p->id & 0xffff);
			CAN_FiR2(CAN1, p->filtbank) = CAN_NEVERUSEID; // Mask all
		}
		else
		{ // Here, use odd 32b register (assume even has been set)
			CAN_FiR2(CAN1, p->filtbank) = (p->id << 16) | (p->id & 0xffff);
		}
	}
	else
	{ // Here, 32b filtering
		CAN_FiR1(CAN1, p->filtbank) = p->id;		
		CAN_FiR2(CAN1, p->filtbank) = p->idmsk;		
	}

	/* Activate filter bank */
	CAN_FA1R(CAN1) |=  (1 << p->filtbank);

	/* Remove filter registers from initialization mode */
	// FINIT = 0; Set initialization mode off for filter banks
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 0) CAN_FMR(CAN1)  &= ~CAN_FMR_FINIT;

	if (p->filtbank > filtnum1) filtnum1 = p->filtbank;

	return 0;
}
/*******************************************************************************
 * int can_driver_filter_add_one_32b_id(u32 CANnum, u32 id1, u32 fifo);
 * @brief 	: Add one CAN ID into next available slot and designate FIFO
 * @param	: CANnum = 0 for CAN1; 1 for CAN2
 * @param	: id1 = CAN ID
 * @param	: fifo = 0|1 for fifo0|fifo1
 * @param	: banknum = filter bank number
 * @return	:  0 = success; 
 *		: -1 = filter edit failed; 
 *		: -2 = CANnum out of bounds
 *		: -3 = fifo out of bounds
 * NOTE: FIFO 0|1 applies to pairs of CAN IDs for the "list mode" (ID-ID)
*******************************************************************************/
int can_driver_filter_add_one_32b_id(u32 CANnum, u32 id1, u32 fifo)
{
	u32 filtnum;

	if (CANnum > 1) return -2; // Be sure index is reasonable
	if (CANnum == 0) filtnum = filtnum1;
	if (CANnum == 1) filtnum = filtnum2; // CAN2 starts as an offset

	// Set initialization mode ON for setting up filter banks
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 1) CAN_FMR(CAN1)  |= CAN_FMR_FINIT;		

	if (oe == 0)
	{ // Load first slot and set 2nd to dummy
		CAN_FiR1(CAN1, filtnum) = id1;
		CAN_FiR2(CAN1, filtnum) = DUMMY;
	}
	else
	{ // Load 2nd slot
		CAN_FiR2(CAN1, filtnum) = id1;
	}

	/* Set assigned filter to FIFO 1 or 2. */
	if (fifo > 1) return -3;
	CAN_FFA1R(CAN1) &= ~(1 << filtnum);	// Clear old bit
	CAN_FFA1R(CAN1) |=  (fifo << filtnum);	// Set new bit

	/* Set 32 bit mode */
	CAN_FS1R(CAN1) |= (1 << filtnum);

	/* Set mode to (id id) */
	CAN_FM1R(CAN1) |= (1 << filtnum);

	/* Activate filter bank */
	CAN_FA1R(CAN1) |= (1 << filtnum);

	/* Next bank number when 2nd slot filled. */
	if (oe != 0)
	{ // Here, 2nd slot was filled, so advance bank number
		oe = 0;
		filtnum += 1;	// Next filter bank
		if (CANnum == 0) filtnum1 = filtnum; // Update
		if (CANnum == 1) filtnum2 = filtnum;
	}
	else
	{ // Here, 1st slot was filled and 2nd has a dummy
		oe = 1;	// 2nd slot next
	}

	/* Remove filter registers from initialization mode */
	// FINIT = 0; Set initialization mode off for filter banks
	while ((CAN_FMR(CAN1) & CAN_FMR_FINIT) != 0) CAN_FMR(CAN1)  &= ~CAN_FMR_FINIT;
	
	return 0;			
}
/*******************************************************************************
 * int can_driver_filter_add_two_32b_id(u32 id1, u32 id2, u32 fifo, u32 banknum);
 * @brief 	: Insert two CAN IDs into filter bank and designate FIFO
 * @param	: id1 = first CAN ID of pair for this filter bank
 * @param	: id2 = second CAN ID of pair for this filter bank
 * @param	: fifo = 0|1 for fifo0|fifo1
 * @param	: banknum = filter bank number
 * @return	:  0 = success; 
 *		: -1 = filter edit failed
*******************************************************************************/
int can_driver_filter_add_two_32b_id(u32 id1, u32 id2, u32 fifo, u32 banknum)
{
	/* Default definition for pair of CAN IDs */
	struct CANFILTERPARAM filt = {DUMMY, DUMMY, 0, 2, 0};
	int ret;

	filt.id = id1;
	filt.idmsk = id2;
	filt.filttype = CANFILT_FIFO0_32b_ID_ID | ((fifo & 0x1) << 2);
	if (banknum > 28) banknum = 28;
	filt.filtbank = banknum;
	ret  = can_driver_filter_insert((struct CANFILTERPARAM*)&filt);
	return ret;
}
/*******************************************************************************
 * int can_driver_filter_add_param_tbl(u32* p, u32 CANnum, u32 size, u32 dummy);
 * @brief 	: Add CAN ID's (32b) to hardware filter table from parameter list
 * @param	: p = pointer to array with list of CAN IDs to be received
 * @param	: CANnum = 0 for CAN1; 1 for CAN2
 * @param	: size = size of table
 * @param	: dummy = CAN ID that is a non-table entry, i.e. skip
 * @return	:  0 = success; 
 *		: -1 = filter edit failed
*******************************************************************************/
int can_driver_filter_add_param_tbl(u32* p, u32 CANnum, u32 size, u32 dummy)
{
	unsigned int i;
	int ret = 0;	// return error code

	for (i = 0; i < size; i++)
	{
		if (*p != dummy) // Skip dummy entries in array
		{
			ret |= can_driver_filter_add_one_32b_id(CANnum, *p, 0);
		}
		p++;
	}
	return ret;
}
/*******************************************************************************
 * void can_driver_filter_setbanknum(u32 CANnum, u32 setoe, u32 banknum);
 * @brief 	: Set the current bank number 
 * @param	: CANnum = 0 for CAN1; 1 for CAN2
 * @param	: oe: 0 = even, 1 = odd (for 32b slot pairs)
 * @param	: banknum = bank number 0-14/28)
*******************************************************************************/
void can_driver_filter_setbanknum(u32 CANnum, u32 setoe, u32 banknum)
{
	if (CANnum == 0) filtnum1 = banknum; // Update
	if (CANnum == 1) filtnum2 = banknum;
	oe = setoe & 1;
	return;
}
/*******************************************************************************
 * u32 can_driver_filter_getbanknum(u32 CANnum);
 * @brief 	: Get the current bank number & oe
 * @param	: CANnum = 0 for CAN1; 1 for CAN2
 * @return	: high byte = bank number; low byte = odd/even
*******************************************************************************/
u32 can_driver_filter_getbanknum(u32 CANnum)
{
	u32 filtnum = 0;
	if (CANnum == 0) filtnum = filtnum1;
	if (CANnum == 1) filtnum = filtnum2; // CAN2 starts as an offset
	return ((filtnum << 8) | oe);
}

