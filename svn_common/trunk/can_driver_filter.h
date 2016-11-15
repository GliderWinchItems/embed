/******************************************************************************
* File Name          : can_driver_filter.h
* Date First Issued  : 05-30-2015
* Board              : F103 or F4
* Description        : CAN routines common for F103 and F4 CAN drivers: filter
*******************************************************************************/
/* 
*/

#ifndef __CANDRIVER_FILTER
#define __CANDRIVER_FILTER

#include "../../../../svn_common/trunk/common_can.h"

/* Bit combinations for all filter types */
//Bits:          FIFO scale mode
#define CANFILT_FIFO0_16b_MSK_ID	0	// 
#define CANFILT_FIFO0_16b_ID_ID		1	// 
#define CANFILT_FIFO0_32b_MSK_ID	2	// 
#define CANFILT_FIFO0_32b_ID_ID		3	// 
#define CANFILT_FIFO1_16b_MSK_ID	4	// 
#define CANFILT_FIFO1_16b_ID_ID		5	// 
#define CANFILT_FIFO1_32b_MSK_ID	6	// 
#define CANFILT_FIFO1_32b_ID_ID		7	// 

#define CANFILT_FIFO0	0
#define CANFILT_FIFO1	1

#define CANFILT_ODD	1
#define CANFILT_EVEN	0

struct CANFILTERPARAM
{
/* The following two 32b words are inserted into a filter bank */
	u32	id;		// Left justified CAN id
	u32	idmsk;		// CAN id (left justified) or mask
	u8	filttype;	// Type code (see above)
	u8	filtbank; 	// Filter bank number (0 - 27)
/* The following element 'odd' is only applicable for 16b mode.
   In 16b mode the 32b word 'id' is placed in the even or odd position in the
      two 32b filter bank.  
   In 16b mode the 32b word 'id' holds both the mask and id or two ids. */
	u8	odd;		// 16b: 0 = even, 1 = odd. 32b: not applicable
};

/******************************************************************************/
int can_driver_filter_deactivate_one(u8 filtbank);
/* @brief 	: Deactivate one filter
 * @param	: filtbank = ( 0 to < 28d)
 * @param	: filttype = 
 * @return	:  0 = success; 
 *		: -1 = filter number out of range
*******************************************************************************/
void can_driver_filter_deactivate_all(void);
/* @brief 	: Change CAN2 filter number boundary default (15) to 'filtbank'
 * @param	: filtbank = boundary number ( 0 to < 28d)
*******************************************************************************/
int can_driver_filter_setCAN2boundary(u8 filtbank);
/* @brief 	: Change CAN2 filter number boundary default (15) to 'filtbank'
 * @param	: filtbank = boundary number ( 0 to < 28d)
 * @return	:  0 = success; -1 = filter number out of range
*******************************************************************************/
int can_driver_filter_insert(struct CANFILTERPARAM* p);
/* @brief 	: Insert a filter AND set it active
 * @param	: p = pointer to struct with filter parameters
 * @return	:  0 = success
 *		: -1 = filter number out of range
 *		: -2 = filttype code out of range
*******************************************************************************/
int can_driver_filter_add_two_32b_id(u32 id1, u32 id2, u32 fifo, u32 banknum);
/* @brief 	: Insert two CAN IDs into filter bank and designate FIFO
 * @param	: id1 = first CAN ID of pair for this filter bank
 * @param	: id2 = second CAN ID of pair for this filter bank
 * @param	: fifo = 0|1 for fifo0|fifo1
 * @param	: banknum = filter bank number
 * @return	:  0 = success; 
 *		: -1 = filter edit failed
*******************************************************************************/
int can_driver_filter_add_param_tbl(u32* p, u32 CANnum, u32 size, u32 dummy);
/* @brief 	: Add CAN ID's (32b) to hardware filter table from parameter list
 * @param	: p = pointer to array with list of CAN IDs to be received
 * @param	: CANnum = 0 for CAN1; 1 for CAN2
 * @param	: size = size of table
 * @param	: dummy = CAN ID that is a non-table entry, i.e. skip
 * @return	:  0 = success; 
 *		: -1 = filter edit failed
*******************************************************************************/
int can_driver_filter_add_one_32b_id(u32 CANnum, u32 id1, u32 fifo);
/* @brief 	: Add one CAN ID into next available slot and designate FIFO
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
void can_driver_filter_setbanknum(u32 CANnum, u32 setoe, u32 banknum);
/* @brief 	: Set the current bank number 
 * @param	: CANnum = 0 for CAN1; 1 for CAN2
 * @param	: oe: 0 = even, 1 = odd (for 32b slot pairs)
 * @param	: banknum = bank number 0-14/28)
*******************************************************************************/
u32 can_driver_filter_getbanknum(u32 CANnum);
/* @brief 	: Get the current bank number & oe
 * @param	: CANnum = 0 for CAN1; 1 for CAN2
 * @return	: high byte = bank number; low byte = odd/even
*******************************************************************************/



#endif
