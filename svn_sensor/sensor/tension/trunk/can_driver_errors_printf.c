/******************************************************************************
* File Name          : can_driver_errors_printf.c
* Date First Issued  : 09/06/2016
* Board              : f103
* Description        : Print the error counts for the can_driver
*******************************************************************************/
#include "can_driver_errors_printf.h"
#include "libusartstm32/usartallproto.h"
/* Error counts for monitoring.
struct CANWINCHPODCOMMONERRORS
{
	u32 can_txerr; 		// Count: total number of msgs returning a TERR flags (including retries)
	u32 can_tx_bombed;	// Count: number of times msgs failed due to too many TXERR
	u32 can_tx_alst0_err; 	// Count: arbitration failure total
	u32 can_tx_alst0_nart_err;// Count: arbitration failure when NART is on
	u32 can_msgovrflow;	// Count: Buffer overflow when adding a msg
	u32 can_spurious_int;	// Count: TSR had no RQCPx bits on (spurious interrupt)
	u32 can_no_flagged;	// Count: can_no_flagged
	u32 can_pfor_bk_one;	// Count: Instances that pfor was adjusted in TX interrupt
	u32 can_pxprv_fwd_one;	// Count: Instances that pxprv was adjusted in 'for' loop
	u32 can_rx0err;		// Count: FIFO 0 overrun
	u32 can_rx1err;		// Count: FIFO 1 overrun
	u32 can_cp1cp2;		// Count: (RQCP1 | RQCP2) unexpectedly ON
	u32 error_fifo1ctr;	// Count: 'systickphasing' unexpected 'default' in switch
	u32 nosyncmsgctr;	// Count: 'systickphasing',lost sync msgs 
	u32 txint_emptylist;	// Count: TX interrupt with pending list empty
	u32 disble_ints_ct;	// Count: while in disable ints looped
};
*/
// Since the counts are "running counts" we keep the previous count and report the difference. */
static struct CANWINCHPODCOMMONERRORS can1_errors_prev;
static struct CANWINCHPODCOMMONERRORS can2_errors_prev;

extern struct CAN_CTLBLOCK* pctl0; // CAN1 control block pointer (tension.c)

/* **************************************************************************************
 * void can_driver_errors_printf(struct CAN_CTLBLOCK* pctl);
 * @brief	: Print the values since last printout
 * @param	: pctl = pointer to CAN 1 or 2 control block 
 * ************************************************************************************** */
void can_driver_errors_printf(struct CAN_CTLBLOCK* pctl)
{	
	struct CANWINCHPODCOMMONERRORS* pcce = &pctl->can_errors;

	struct CANWINCHPODCOMMONERRORS* pprev = &can1_errors_prev;
	if (pctl != pctl0)
	{
		 pprev = &can2_errors_prev;
		printf("CAN2 error counts since last listing\n\r");
	}
	else
	{
		printf("CAN1 error counts since last listing\n\r");
	}

 printf("%6d %s\n\r",(int)(pcce->can_txerr - pprev->can_txerr),"can_txerr\ttotal number of msgs returning a TERR flags (including retries)");
 printf("%6d %s\n\r",(int)(pcce->can_tx_bombed - pprev->can_tx_bombed),"can_tx_bombed\tnumber of times msgs failed due to too many TXERR");
 printf("%6d %s\n\r",(int)(pcce->can_tx_alst0_err - pprev->can_tx_alst0_err),"can_tx_alst0_err\tarbitration failure total");
 printf("%6d %s\n\r",(int)(pcce->can_tx_alst0_nart_err - pprev->can_tx_alst0_nart_err),"can_tx_alst0_nart_err\tarbitration failure when NART is on");
 printf("%6d %s\n\r",(int)(pcce->can_msgovrflow - pprev->can_msgovrflow),"can_msgovrflow\tBuffer overflow when adding a msg");
 printf("%6d %s\n\r",(int)(pcce->can_spurious_int - pprev->can_spurious_int),"can_spurious_int\tTSR had no RQCPx bits on (spurious interrupt)");
 printf("%6d %s\n\r",(int)(pcce->can_no_flagged - pprev->can_no_flagged),"can_no_flagged\tcan_no_flagged");
 printf("%6d %s\n\r",(int)(pcce->can_pfor_bk_one - pprev->can_pfor_bk_one),"can_pfor_bk_one\tInstances that pfor was adjusted in TX interrupt");
 printf("%6d %s\n\r",(int)(pcce->can_pxprv_fwd_one - pprev->can_pxprv_fwd_one),"can_pxprv_fwd_one\tInstances that pxprv was adjusted in 'for' loop");
 printf("%6d %s\n\r",(int)(pcce->can_rx0err - pprev->can_rx0err),"can_rx0err\tFIFO 0 overrun");
 printf("%6d %s\n\r",(int)(pcce->can_rx1err - pprev->can_rx1err),"can_rx1err\tFIFO 1 overrun");
 printf("%6d %s\n\r",(int)(pcce->can_cp1cp2 - pprev->can_cp1cp2),"can_cp1cp2\t(RQCP1 | RQCP2) unexpectedly ON");
 printf("%6d %s\n\r",(int)(pcce->error_fifo1ctr - pprev->error_fifo1ctr),"error_fifo1ctr\t'systickphasing' unexpected 'default' in switch");
 printf("%6d %s\n\r",(int)(pcce->nosyncmsgctr - pprev->nosyncmsgctr),"nosyncmsgctr\t'systickphasing',lost sync msgs ");
 printf("%6d %s\n\r",(int)(pcce->txint_emptylist - pprev->txint_emptylist),"txint_emptylist\tTX interrupt with pending list empty");
 printf("%6d %s\n\r",(int)(pcce->disable_ints_ct - pprev->disable_ints_ct),"disble_ints_ct\twhile in disable ints looped");
 
pprev->can_txerr 		= pcce->can_txerr;
pprev->can_tx_bombed 		= pcce->can_tx_bombed;
pprev->can_tx_alst0_err 	= pcce->can_tx_alst0_err;
pprev->can_tx_alst0_nart_err 	= pcce->can_tx_alst0_nart_err;
pprev->can_msgovrflow 		= pcce->can_msgovrflow;
pprev->can_spurious_int 	= pcce->can_spurious_int;
pprev->can_no_flagged 		= pcce->can_no_flagged;
pprev->can_pfor_bk_one		= pcce->can_pfor_bk_one;
pprev->can_pxprv_fwd_one 	= pcce->can_pxprv_fwd_one;
pprev->can_rx0err 		= pcce->can_rx0err;
pprev->can_rx1err 		= pcce->can_rx1err;
pprev->can_cp1cp2 		= pcce->can_cp1cp2;
pprev->error_fifo1ctr 		= pcce->error_fifo1ctr;
pprev->nosyncmsgctr 		= pcce->nosyncmsgctr;
pprev->txint_emptylist 		= pcce->txint_emptylist;
pprev->disable_ints_ct 		= pcce->disable_ints_ct; // Make counts absolute, i.e. did it every happen monitoring.
	
}
/* **************************************************************************************
 * void can_driver_errortotal_printf(struct CAN_CTLBLOCK* pctl);
 * @brief	: Print accumulated values
 * @param	: pctl = pointer to CAN 1 or 2 control block 
 * ************************************************************************************** */
void can_driver_errortotal_printf(struct CAN_CTLBLOCK* pctl)
{	
	struct CANWINCHPODCOMMONERRORS* pcce = &pctl->can_errors;

	if (pctl != pctl0)
		printf("CAN2 total error ct\n\r");
	else
		printf("CAN1 total error ct\n\r");

 printf("%6d %s\n\r",(int)(pcce->can_txerr),"can_txerr");
 printf("%6d %s\n\r",(int)(pcce->can_tx_bombed),"can_tx_bombed");
 printf("%6d %s\n\r",(int)(pcce->can_tx_alst0_err),"can_tx_alst0_err");
 printf("%6d %s\n\r",(int)(pcce->can_tx_alst0_nart_err),"can_tx_alst0_nart_err");
 printf("%6d %s\n\r",(int)(pcce->can_msgovrflow),"can_msgovrflow");
 printf("%6d %s\n\r",(int)(pcce->can_spurious_int),"can_spurious_int");
 printf("%6d %s\n\r",(int)(pcce->can_no_flagged),"can_no_flagged");
 printf("%6d %s\n\r",(int)(pcce->can_pfor_bk_one),"can_pfor_bk_one");
 printf("%6d %s\n\r",(int)(pcce->can_pxprv_fwd_one),"can_pxprv_fwd_one");
 printf("%6d %s\n\r",(int)(pcce->can_rx0err),"can_rx0err");
 printf("%6d %s\n\r",(int)(pcce->can_rx1err),"can_rx1err");
 printf("%6d %s\n\r",(int)(pcce->can_cp1cp2),"can_cp1cp2");
 printf("%6d %s\n\r",(int)(pcce->error_fifo1ctr),"error_fifo1ctr");
 printf("%6d %s\n\r",(int)(pcce->nosyncmsgctr),"nosyncmsgctr");
 printf("%6d %s\n\r",(int)(pcce->txint_emptylist),"txint_emptylist");
 printf("%6d %s\n\r",(int)(pcce->disable_ints_ct),"disble_ints_ct");
 	
}
