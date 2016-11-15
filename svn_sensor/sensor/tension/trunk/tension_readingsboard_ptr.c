/******************************************************************************
* File Name          : tension_readingsboard_ptr.c
* Date First Issued  : 06/11/2016
* Board              :
* Description        : Pointer to reading versus code for program level readings
*******************************************************************************/
/* 
Pointers to readings for the program/board level (and not the function instance
level).  Only one list applies, whereas for function instances the list applies
(i.e. the offsets into a struct) for all instances of that function.  Consequently,
there is no need for a struct that encompasses all the list.

Note however, CAN_CTLBLOCK is a struct that is setup with memory from calloc and
therefore is not a fixed location at compile time.  To handle this a dummy struct 
is setup and and a pointer to the pointer to the control block is placed in the 
2nd element of the list entry.  

Locations in memory that are known at compile time
do not need this sort of mash-up, but since an entry is required NULL is used to 
signal that the first entry pointer is used directly.

*/

#include <stdint.h>
#include "../../../../svn_common/trunk/common_can.h"
#include "tension_readingsboard_ptr.h"
#include "tension_a_functionS.h"
#include "p1_initialization.h"
#include "../../../../svn_common/trunk/db/gen_db.h"
#include "../../../../svn_common/trunk/can_driver.h"

/* Pointer to control block for CAN1 when it was init'd in tension.c 'main'  */
extern struct CAN_CTLBLOCK* pctl0;	// Pointer to control block for CAN1


/* **************************************************************************************
 * struct READINGSBOARDPTR_RET tension_readingsboard_ptr_get(uint16_t code);
 * @brief	: Get the 4 byte value and type code, given the code
 * @param	:  code = code number for reading
 * @return	:  struct: 
 *		:  .val: value as uint32_t
 *		:  .type: number type code (for format and usage)
 *		:  Code not found-- .type = 0  
 * ************************************************************************************** */
struct READINGSBOARDPTR_RET tension_readingsboard_ptr_get(uint16_t code)
{
	struct READINGSBOARDPTR_RET ret = {0,TYP_U32};	// (So far) All have this number type
	
	switch(code)
	{
case PROG_TENSION_READINGS_BOARD_NUM_AD7799:	    ret.val = AD7799_num; 				break; /* Number of AD7799 that successfully initialized */
case PROG_TENSION_READINGS_BOARD_CAN_TXERR:	    ret.val = pctl0->can_errors.can_txerr; 		break; /* Count: total number of msgs returning a TERR flags (including retries) */
case PROG_TENSION_READINGS_BOARD_CAN_TX_BOMBED:	    ret.val = pctl0->can_errors.can_tx_bombed;		break; /* Count: number of times msgs failed due to too many TXERR */
case PROG_TENSION_READINGS_BOARD_CAN_ALST0_ERR:	    ret.val = pctl0->can_errors.can_tx_alst0_err;	break; /* Count: arbitration failure total */
case PROG_TENSION_READINGS_BOARD_CAN_ALST0_NART_ERR:ret.val = pctl0->can_errors.can_tx_alst0_nart_err;	break; /* Count: arbitration failure when NART is on */
case PROG_TENSION_READINGS_BOARD_CAN_MSGOVRFLO:     ret.val = pctl0->can_errors.can_msgovrflow;		break; /* Count: Buffer overflow when adding a msg */
case PROG_TENSION_READINGS_BOARD_CAN_SPURIOUS_INT:  ret.val = pctl0->can_errors.can_spurious_int;	break; /* Count: TSR had no RQCPx bits on (spurious interrupt) */
case PROG_TENSION_READINGS_BOARD_CAN_NO_FLAGGED:    ret.val = pctl0->can_errors.can_no_flagged;		break; /* Count: */
case PROG_TENSION_READINGS_BOARD_CAN_PFOR_BK_ONE:   ret.val = pctl0->can_errors.can_pfor_bk_one;	break; /* Count: Instances that pfor was adjusted in TX interrupt */
case PROG_TENSION_READINGS_BOARD_CAN_PXPRV_FWD_ONE: ret.val = pctl0->can_errors.can_pxprv_fwd_one;	break; /* Count: Instances that pxprv was adjusted in 'for' loop */
case PROG_TENSION_READINGS_BOARD_CAN_RX0ERR_CT:	    ret.val = pctl0->can_errors.can_rx0err;		break; /* Count: FIFO 0 overrun */
case PROG_TENSION_READINGS_BOARD_CAN_RX1ERR_CT:     ret.val = pctl0->can_errors.can_rx1err;		break; /* Count: FIFO 1 overrun */
case PROG_TENSION_READINGS_BOARD_CAN_CP1CP2: 	    ret.val = pctl0->can_errors.can_cp1cp2;		break; /* Count: (RQCP1 | RQCP2) unexpectedly ON */
case PROG_TENSION_READINGS_BOARD_TXINT_EMPTYLIST:   ret.val = pctl0->can_errors.txint_emptylist;	break; /* Count: TX interrupt with pending list empty */
case PROG_TENSION_READINGS_BOARD_CAN1_BOGUS_CT:     ret.val = pctl0->bogusct;				break; /* Count of bogus CAN1 IDs rejected */
	default: ret.type = 0;	// Return code-not-found
	}
	return ret;
}


