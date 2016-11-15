/******************************************************************************
* File Name          : can_driver.c
* Date First Issued  : 05-28-2015
* Board              : F103 or F4
* Description        : CAN routines common for F103 and F4 CAN drivers
*******************************************************************************/
/*
06/02/2016 - Add rejection of loading bogus CAN ids.

06/14/2015 rev 720: can.driver.[ch] replaced with can.driverR.[ch] and 
  old can.driver[ch] deleted from svn.

This routine implements a common pool of buffer blocks for TX, RX0, and RX1 for
CAN1 and CAN2.  There is a linked list of "free" blocks which are removed from
this list and added to the CAN1 TX, RX0, or RX1, or CAN2 TX, RX0, RX1 lists.

RX lists are used a FIFOs.  TX msgs are added and ordered by CAN priority so 
that the next CAN arbitration always is attempted with the highest CAN priority 
msg.

Startup--
1) Initialize and reserve buffer memory--e.g.
   pctl1 = canwinch_setup_F103_pod(180);
2) test return for errors
   if (pctl1 == NULL) and
   if (pctl1->ret < 0)
3) enable interrupts-- e.g. 
   can_driver_enable_interrupts();


NOTE: vector.c needs the following interrupt entry 
 labels in the appropriate interrupt vector locations--
(the following are the default F4 names in vector.c, the F103 names need changing.)
void CAN1_TX_IRQHandler
void CAN1_RX0_IRQHandler
void CAN1_RX1_IRQHandler

void CAN2_TX_IRQHandler
void CAN2_RX0_IRQHandler
void CAN2_RX1_IRQHandler
*/
// 
#define WEAK __attribute__ ((weak))
void WEAK CAN1_TX_IRQHandler(void);
void WEAK CAN1_RX0_IRQHandler(void);
void WEAK CAN1_RX1_IRQHandler(void);

void WEAK CAN2_TX_IRQHandler(void);
void WEAK CAN2_RX0_IRQHandler(void);
void WEAK CAN2_RX1_IRQHandler(void);

#include <malloc.h>

#ifdef STM32F3
  #include "../../../git_discoveryf3/lib/libopencm3/stm32/can.h"
  #include "../../../git_discoveryf3/lib/libusartstm32f3/nvicdirect.h"
#elif STM32F4
 #include "../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/stm32/can.h"
 #include "nvicdirect.h"
#elif STM32F103
 #include "../../../svn_pod/sw_stm32/trunk/lib/libopenstm32/can.h"
 #define CAN_RFR0			0x000
 #define CAN_RFR1			0x004
 #define CAN_RFxR(can_base, rfr)		MMIO32(can_base + rfr + 0x00C)
 #include "nvicdirect.h"
#else
 # error "candriver.c: stm32 family not defined."
#endif

#include "can_driver.h"
#include "../../../../svn_common/trunk/can_driver_port.h"
#include "DTW_counter.h"

/* subroutine declarations */
static void loadmbx2(struct CAN_CTLBLOCK* pctl);
static void moveremove2(struct CAN_CTLBLOCK* pctl);

/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern u32	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern u32	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

#define CAN_TIMEOUT	6000000		// Break 'while' loop count during CAN initialization

 struct CAN_POOLBLOCK  frii;	// Always present block, i.e. list pointer head
static struct CAN_CTLBLOCK* pctl0;	// Pointer to control block for CAN1
static struct CAN_CTLBLOCK* pctl1;	// Pointer to control block for CAN2
 u32 buffct = 0;	// Number of msg blocks buffered (total)
 int friict = 0;	// Number of msg blocks on the free list

/*---------------------------------------------------------------------------------------------
 * static void disable_ints(u32* p[2]);
 * @brief	: disable CAN1 and CAN2 TX, RX0, RX1 interrupts
 * @param	: p[2] = pointer to save
 ----------------------------------------------------------------------------------------------*/
static void disable_ints(u32 p[])
{
//   __attribute__((__unused__)) int rdbk ; // Readback dummy
	p[0] = CAN_IER(CAN1) & 0x7F;	// Save current IER
	p[1] = CAN_IER(CAN2) & 0x7F;
	CAN_IER(CAN1) &= ~0x7F;	// Disable TX, RX0, RX1, and errors
	CAN_IER(CAN2) &= ~0x7F;
	/* Note: the while assures that the disabling is complete before returning. */
	// The counting is for monitoring if the looping ever takes place
	while((CAN_IER(CAN1) & 0x13) != 0) pctl0->can_errors.disable_ints_ct += 1;
	while((CAN_IER(CAN2) & 0x13) != 0) pctl1->can_errors.disable_ints_ct += 1;
	return;
}
/*---------------------------------------------------------------------------------------------
 * static void disable_TXints(u32* p[2]);
 * @brief	: disable CAN1 and CAN2 TX interrupts;' leave RX interrupts alone
 * @param	: p[2] = pointer to save
 ----------------------------------------------------------------------------------------------*/
static void disable_TXints(u32 p[])
{
	p[0] = CAN_IER(CAN1) & 0x7F;	// Save current IER CAN interrlupts
	p[1] = CAN_IER(CAN2) & 0x7F;
	CAN_IER(CAN1) &= ~0x01;	// Disable TX
	CAN_IER(CAN2) &= ~0x01;
	/* Note: the while assures that the disabling is complete before returning. */
	// The counting is for monitoring if the looping ever takes place
	while((CAN_IER(CAN1) & 0x01) != 0) pctl0->can_errors.disable_ints_ct += 1;
	while((CAN_IER(CAN2) & 0x01) != 0) pctl1->can_errors.disable_ints_ct += 1;
	return;
}
/*---------------------------------------------------------------------------------------------
 * static void reenable_ints(u32* p[2]);
 * @brief	: reenable CAN1 and CAN2 TX, RX0, RX1 interrupts
 * @param	: p1 = pointer to CAN1 IER before disable
 * @param	: p2 = pointer to CAN2 IER before disable
 ----------------------------------------------------------------------------------------------*/
static void reenable_ints(u32 p[])
{
	CAN_IER(CAN1) |= p[0];
	CAN_IER(CAN2) |= p[1];
	return;
}
/*---------------------------------------------------------------------------------------------
 * static u8 bitcvt(u8 bit);
 * @brief	: convert a non-zero byte into a 1
 * @param	: input bit
 * @return	: bit
 ----------------------------------------------------------------------------------------------*/
static u8 bitcvt(u8 bit)
{
	if (bit != 0) return 1;
	return 0;
}
/******************************************************************************
 * struct CAN_CTLBLOCK* can_driver_init(struct CAN_PARAMS2 *p, const struct CAN_INIT pinit);
 * @brief 	: Setup CAN pins and hardware
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @param	: pinit = pointer to msg buffer counts for this CAN
 * @return	: Pointer to control block for this CAN
 *		:  NULL = cannum not 1 or 2, calloc of control block failed
 *		:  Pointer->ret = return code
 *		:   0 success
 *		:  -1 cannum: CAN number not 1 or 2
 *		:  -2 calloc of linked list failed
 *		:  -6 CAN initialization mode timed out
 *		:  -7 Leave initialization mode timed out
 *		: -12 port pin setup CAN1
 *		: -13 port pin setup CAN2
*******************************************************************************/
struct CANWINCHPODCOMMONERRORS* pcan_errors; // Debug: pointer to CAN errors struct

struct CAN_CTLBLOCK* can_driver_init(struct CAN_PARAMS2 *p, const struct CAN_INIT* pinit)
{
	int ret;
	u32 can_timeout;
	struct CAN_CTLBLOCK*  pctl;
	struct CAN_POOLBLOCK* plst;
	struct CAN_POOLBLOCK* ptmp;

	/* Get base address of CAN module. */
	// We depend on F4, F103, and F373 being the same (which they are, so far)
	if ((p->cannum != 1) && (p->cannum != 2)) return NULL;

	/* Get control block for this CAN module. */
	pctl = (struct CAN_CTLBLOCK*)calloc(1, sizeof(struct CAN_CTLBLOCK));
	if (pctl == NULL) return NULL;
	if (p->cannum == 1) {pctl0 = pctl; pctl->vcan = CAN1;}
	if (p->cannum == 2) {pctl1 = pctl; pctl->vcan = CAN2;}
	
	/* Now that we have control block in memory, use it to return errors. 
	   by setting the error code in pctl->ret. */

	/* One time only--get pool of msg blocks for "everybody." */
	// Note: if the first caller doesn't setup of the buffers, maybe the 2nd one will
	// in which case the buffers get setup then.  If neither sets them up the the
	// enable interrupts call will return -1 and not enable interrupts.
	if ((buffct == 0)  && (pinit->numbuff != 0)) // Have we calloc'ed the buffer space?
	{ // Here, no.
		buffct = pinit->numbuff;// Serves as a OTO switch and saves total blk count
		friict = buffct;	// Initial running count of free list msg  block inventory

		/* Get CAN xmit linked list and init 'plinknext'. */	
		ptmp = (struct CAN_POOLBLOCK*)calloc(pinit->numbuff, sizeof(struct CAN_POOLBLOCK));
		if ( ptmp == NULL){pctl->ret = -2; return pctl;} // Get buff failed

		/* Initialize links in the free list. */
		plst = &frii;
		for (can_timeout = 0; can_timeout < pinit->numbuff; can_timeout++)
		{
			plst->plinknext = ptmp;
			plst = ptmp++;
		} 
	}

	/* Save msg count limits for this CAN. */
	pctl->txbct     = 0;	// Zero'd by calloc
	pctl->ptrs0.bct = 0;	// Zero'd by calloc
	pctl->ptrs1.bct = 0;	// Zero'd by calloc
	pctl->txbmx     = pinit->txmax;		// Max blocks this TX can commandeer
	pctl->ptrs0.bmx = pinit->rx0max;	// Max blocks this RX0 can commandeer
	pctl->ptrs1.bmx = pinit->rx1max;	// Max blocks this RX1 can commandeer

	/* Set tail pointer to an empty list. */
	// calloc set these to zero so we comment them out
//	pctl->ptrs0.ptail = NULL; // RX0
//	pctl->ptrs1.ptail = NULL; // RX1

	/* Setup CAN port pins and enable module clocking. */
	ret = can_driver_port(p->port, p->cannum);
	if (ret != 0) {pctl->ret = ret - 10; return pctl;} // failed

	/* ---------- Put CAN module into Initialization mode -------------------- */
	/* Request initialization p 632, 648.  DEBUG freeze bit on */
	CAN_MCR(pctl->vcan) &= CAN_MCR_DBF;	// Clear DBF since it is set coming out RESET
	CAN_MCR(pctl->vcan) = ((bitcvt(p->dbf) << 16) | (CAN_MCR_INRQ) );

	/* The initialization request (above) causes the INAK bit to be set.  This bit goes off when 11 consecutive
	   recessive bits have been received p 633 */
	can_timeout = CAN_TIMEOUT;	// Counter to break loop for timeout
	while ( ((CAN_MSR(pctl->vcan) & CAN_MSR_INAK) == 0 ) && (can_timeout-- > 0) ); // Wait until initialization mode starts or times out
	if (can_timeout <= 0 ) {pctl->ret = -6; return pctl;}	// Timed out

	/* Compute Baud Rate Prescalar (+1). */
	u32 brp = (pclk1_freq / (1 + p->tbs1 + p->tbs2) ) / p->baudrate;

	/* Setup Bit Timing Register. */
	CAN_BTR(pctl->vcan)  =  (bitcvt(p->silm)        << 31);	// Silent mode bit
	CAN_BTR(pctl->vcan) |=  (bitcvt(p->lbkm)        << 30);	// Loopback mode bit
	CAN_BTR(pctl->vcan) |=  (((p->sjw  - 1) & 0x03) << 24);	// Resynchronization jump width
	CAN_BTR(pctl->vcan) |=  (((p->tbs2 - 1) & 0x07) << 20);	// Time segment 2
	CAN_BTR(pctl->vcan) |=  (((p->tbs1 - 1) & 0x0F) << 16);	// Time segment 1
	CAN_BTR(pctl->vcan) |=  (((brp-1)      & 0x3FF) <<  0);	// Baud rate prescalar

	/* Master Control Register */
	CAN_MCR(pctl->vcan) &= ~(0xfe);			// Clear bits except for INAQ
	CAN_MCR(pctl->vcan) |=  (bitcvt(p->ttcm) << 7);	// Time triggered communication mode
	CAN_MCR(pctl->vcan) |=  (bitcvt(p->abom) << 6);	// Automatic bus-off management
	CAN_MCR(pctl->vcan) |=  (bitcvt(p->awum) << 5);	// Auto WakeUp Mode
	CAN_MCR(pctl->vcan) |=  (1 << 4);		// No Automatic ReTry (0 = retry; non-zero = xmit only once)

	/* Leave initialization mode */
	CAN_MCR(pctl->vcan) &= ~CAN_MCR_INRQ;
	can_timeout = CAN_TIMEOUT;	// Counter to break loop for timeout
	while ( ((CAN_MSR(pctl->vcan) & CAN_MSR_INAK) != 0 ) && (can_timeout-- > 0) );	// Wait until initialization mode starts or times out
	if (can_timeout == 0 ){pctl->ret = -7; return pctl;}	// Timed out

	/* Set and enable interrupt controller for CAN interrupts: TX, RX0, RX1 */
	NVICIPR (p->tx.vectnum, p->tx.vectpriority);	// Set interrupt priority
	NVICISER(p->tx.vectnum);			// Enable interrupt controller

	NVICIPR (p->rx0.vectnum, p->rx0.vectpriority);	// Set interrupt priority
	NVICISER(p->rx0.vectnum);			// Enable interrupt controller

	NVICIPR (p->rx1.vectnum, p->rx1.vectpriority);	// Set interrupt priority
	NVICISER(p->rx1.vectnum);			// Enable interrupt controller

	/* If mailbox is not empty, then enabling interrupts will result in an immediate
           interrupt, and the TX_IRQHandler will be dealing with an empty pending list! */
	while ( (CAN_TSR(pctl->vcan) & CAN_TSR_TME0) == 0)         // Wait for transmit mailbox 0 to be empty
		CAN_TSR(pctl->vcan)  = 0X1;

pcan_errors = &pctl->can_errors;

	/* CAN (CAN1 or CAN2) was intialized.  Enable by calling 'can_driver_enable_interrupts'
	   below *after both* CAN1 and CAN2 intialized (if both are being used). */
//	pctl->ret = 0;	// Set success code (which 'calloc' already set to zero)
	return pctl;	// Return pointer to control block
}
/******************************************************************************
 * int can_driver_enable_interrupts(void);
 * @brief	: Enable interrupts after all CAN modules have been initialized
 * @return	: 0 = OK; -1 = no buffers were set up
 ******************************************************************************/
int can_driver_enable_interrupts(void)
{
	if (buffct == 0) return -1;
	CAN_IER(CAN1) |= 0x13;
	CAN_IER(CAN2) |= 0x13;
	return 0;	
}
/******************************************************************************
 * static void can_driver_toss(volatile struct CAN_RCV_PTRS* ptrs);
 * @brief	: Get pointer to high priority CAN msg buffer (FIFO 0 or 1)
 * @return	: struct with pointer to can msg struct, ptr = zero if no new data
 ******************************************************************************/
u32 debugB = 0;
u32 debugC = 0;
static void can_driver_toss(volatile struct CAN_RCV_PTRS* ptrs)
{
	volatile struct CAN_POOLBLOCK* pmov; // 'volatile' to match plinknext

	u32 save[2];
	disable_ints(save);
	// Remove block from received list
	ptrs->bct -= 1;		// Decrement RX pending list
	friict += 1;		// Increment free list inventory
	pmov = ptrs->ptail->plinknext;	// pmov pts to head
	if (pmov == pmov->plinknext)
	{ // Removing last block for list
		ptrs->ptail = NULL;
debugB += 1;
	}
	else
	{ // More than one block on the list
		ptrs->ptail->plinknext = pmov->plinknext; // Set new head ptr
debugC += 1;
	}
	pmov->plinknext = frii.plinknext;
	frii.plinknext  = pmov;
	reenable_ints(save);
	return;
}
/******************************************************************************
 * void can_driver_toss0(struct CAN_CTLBLOCK* pctl);
 * void can_driver_toss1(struct CAN_CTLBLOCK* pctl);
 * @brief	: Release msg buffer back to free list
 ******************************************************************************/
void can_driver_toss0(struct CAN_CTLBLOCK* pctl)
{
	can_driver_toss(&pctl->ptrs0);
	return;
}
void can_driver_toss1(struct CAN_CTLBLOCK* pctl)
{
	can_driver_toss(&pctl->ptrs1);
	return; 
}
/******************************************************************************
 * struct CANRCVBUF* can_driver_peek0(struct CAN_CTLBLOCK* pctl);
 * struct CANRCVBUF* can_driver_peek1(struct CAN_CTLBLOCK* pctl);
 * @brief	: Get pointer to a received msg CAN msg buffer.
 * @return	: struct with pointer to earliest buffer, ptr = zero if no data
 ******************************************************************************/
u32 debugA = 0;
struct CANRCVBUF* can_driver_peek0(struct CAN_CTLBLOCK* pctl)
{
	/* Return if no msgs in this list */
	if (pctl->ptrs0.ptail == NULL) return NULL;
	return (struct CANRCVBUF*)&pctl->ptrs0.ptail->plinknext->can; // volatile discard warning
}
struct CANRCVBUF* can_driver_peek1(struct CAN_CTLBLOCK* pctl)
{
	/* Return if no msgs in this list */
	if (pctl->ptrs1.ptail == NULL) return NULL;
debugA += 1;
	return (struct CANRCVBUF*)&pctl->ptrs1.ptail->plinknext->can; // volatile discard warning
}
/******************************************************************************
 * int can_driver_put(struct CAN_CTLBLOCK* pctl, struct CANRCVBUF *pcan, u8 maxretryct, u8 bits);
 * @brief	: Get a free slot and add msg ('ext' = extended version)
 * @param	: pctl = pointer to control block for this CAN modules
 * @param	: pcan = pointer to msg: id, dlc, data (common_can.h)
 * @param	: maxretryct =  0 = use TERRMAXCOUNT; not zero = use this value.
 * @param	: bits = Use these bits to set some conditions (see .h file)
 * @return	:  0 = OK; 
 *		: -1 = Buffer overrun (no free slots for the new msg)
 *		: -2 = Bogus CAN id rejected
 ******************************************************************************/


int can_driver_put(struct CAN_CTLBLOCK* pctl, struct CANRCVBUF *pcan, u8 maxretryct, u8 bits)
{
	volatile struct CAN_POOLBLOCK* pnew;
	struct CAN_POOLBLOCK* pfor; 	// Loop pointer for the 'for’ loop.

	u32 save[2];	// IER register saved during disable of interrupts

	/* Reject CAN msg if CAN id is "bogus". */
	// If 11b is specified && bits in extended address are present it is bogus
	if (((pcan->id & CAN_IDE) == 0) && ((pcan->id & CAN_EXTENDED_MASK) != 0))
	{
		pctl->bogusct += 1;
		return -2;
	}

	/* Get a free block from the free list. */
	disable_ints(save);	// TX, RX0, RX1 interrupt might move a msg to the free list.
	pnew = frii.plinknext;
	if ((pnew == NULL) || (pctl->txbct >= pctl->txbmx))
	{ // Here, either no free list blocks OR this TX reached its limit
		reenable_ints(save);
		pctl->can_errors.can_msgovrflow += 1;	// Count overflows
		return -1;	// Return failure: no space & screwed
	}	
	frii.plinknext = pnew->plinknext;
	friict -= 1; // Keep running count of free list inventory
	reenable_ints(save);
	/* 'pnew' now points to the block that is free (and not linked). */

	/* Build struct/block for addition to the pending list. */
	// retryct    xb[0]	// Counter for number of retries for TERR errors
	// maxretryct xb[1]	// Maximum number of TERR retry counts
	// bits	      xb[2]		// Use these bits to set some conditions (see below)
	// nosend     xb[3]	// Do not send: 0 = send; 1 = do NOT send on CAN bus (internal use only)
	pnew->can     = *pcan;	// Copy CAN msg.
	pnew->x.xb[1] = maxretryct;	// Maximum number of TERR retry counts
	pnew->x.xb[2] = bits;	// Use these bits to set some conditions (see .h file)
	pnew->x.xb[3] = 0;	// not used for now
	pnew->x.xb[0] = 0;	// Retry counter for TERRs

	/* Find location to insert new msg.  Lower value CAN ids are higher priority, 
           and when the CAN id msg to be inserted has the same CAN id as the 'pfor' one
           already in the list, then place the new one further down so that msgs with 
           the same CAN id do not get their order of transmission altered. */
 	disable_TXints(save);

	for (pfor = &pctl->pend; pfor->plinknext != NULL; pfor = pfor->plinknext)
	{
		if (pnew->can.id < (pfor->plinknext)->can.id) // Pay attention: "value" vs "priority"
			break;
	}

	/* Add new msg to pending list. (TX interrupt is still disabled) */
	pnew->plinknext = pfor->plinknext; 	// Insert new msg into 
	pfor->plinknext = pnew;			//   pending list.
	pfor =  NULL;	// Signal TX interrupt that search loop is not active.
	if (pctl->pxprv == NULL) // Is sending complete?
	{ // pxprv == NULL means CAN mailbox did not get loaded, so CAN is idle.
		loadmbx2(pctl); // Start sending
	}
	else
	{ // CAN sending is in progress.
		if ((pctl->pxprv)->plinknext == pnew) // Does pxprv need adjustment?
		{ // Here yes. We inserted a msg between 'pxprv' and 'pxprv->linknext'
			pctl->pxprv = pnew;	// Update 'pxprv' so that it still points to msg TX using.
			pctl->can_errors.can_pxprv_fwd_one += 1;	// Count: Instances that pxprv was adjusted in 'for' loop
		}
	}
	pctl->txbct += 1;	// Increment running ct of pending list
	reenable_ints(save);

	return 0;	// Success!
}
/*---------------------------------------------------------------------------------------------
 * static void loadmbx2(struct CAN_CTLBLOCK* pctl)
 * @brief	: Load mailbox
 ----------------------------------------------------------------------------------------------*/
static void loadmbx2(struct CAN_CTLBLOCK* pctl)
{
	volatile struct CAN_POOLBLOCK* p = pctl->pend.plinknext;

	if (p == NULL)
	{
		pctl->pxprv = NULL;
		return; // Return if no more to send
	}
	pctl->pxprv = &pctl->pend;	// Save in a static var

	/* Load the mailbox with the message.  CAN ID low bit starts xmission. */
	CAN_TDTxR(pctl->vcan, CAN_MBOX0) =  p->can.dlc;	 	// CAN_TDT0R:  mailbox 0 time & length p 660
	CAN_TDLxR(pctl->vcan, CAN_MBOX0) =  p->can.cd.ui[0];	// CAN_TDL0RL: mailbox 0 data low  register p 661 
	CAN_TDHxR(pctl->vcan, CAN_MBOX0) =  p->can.cd.ui[1];	// CAN_TDL0RH: mailbox 0 data low  register p 661 
	/* Load CAN ID and set TX Request bit */
	CAN_TIxR (pctl->vcan, CAN_MBOX0) =  (p->can.id | 0x1); 	// CAN_TI0R:   mailbox 0 identifier register p 659
	return;
}
/******************************************************************************
 * u32 can_driver_getcount(struct CAN_CTLBLOCK* pctla, struct CAN_CTLBLOCK* pctlb, struct CAN_BLOCK_CTS* pcts);
 * @brief	: Get current linked list counts for all six possible lists.
 * @param	: pctla = pointer to control block for CAN module ("a")
 * @param	: pctla = pointer to control block for CAN module, if a 2nd one used ("b")
 * @param	: pcts = pointer to stucts with current counts for all six possible lists.
 * @return	: free list count
 ******************************************************************************/
u32 can_driver_getcount(struct CAN_CTLBLOCK* pctla, struct CAN_CTLBLOCK* pctlb, struct CAN_BLOCK_CTS* pcts)
{
	u32 save[2];	// IER register saved during disable of interrupts
	
	disable_ints(save);	// Disable CAN interrupts so counts don't change
	if (pctla != NULL)
	{ // Here, first CAN 
		pcts->catx  = pctla->txbct;
		pcts->carx0 = pctla->ptrs0.bct;
		pcts->carx1 = pctla->ptrs1.bct;
	}
	if (pctlb != NULL)
	{ // Here, second CAN.
		pcts->cbtx  = pctlb->txbct;
		pcts->cbrx0 = pctlb->ptrs0.bct;
		pcts->cbrx1 = pctlb->ptrs1.bct;
	}
	reenable_ints(save);
	return friict;	// Return number of blocks calloc'ed
}
/*#######################################################################################
 * ISR routine for CAN: "Transmit Interrupt" <-- (RQCP0 | RQCP1 | RQCP2)
 *####################################################################################### */
/* p 647 The transmit interrupt can be generated by the following events:
–   Transmit mailbox 0 becomes empty, RQCP0 bit in the CAN_TSR register set.
–   Transmit mailbox 1 becomes empty, RQCP1 bit in the CAN_TSR register set.
–   Transmit mailbox 2 becomes empty, RQCP2 bit in the CAN_TSR register set.

(CAN_TSR(CAN1) & 0xf)
Bit 3 TERR0: Transmission error of mailbox0
  This bit is set when the previous TX failed due to an error.
Bit 2 ALST0: Arbitration lost for mailbox0
  This bit is set when the previous TX failed due to an arbitration lost.
Bit 1 TXOK0: Transmission OK of mailbox0
  The hardware updates this bit after each transmission attempt.
   0: The previous transmission failed
   1: The previous transmission was successful
  This bit is set by hardware when the transmission request on mailbox 1 has been completed
  successfully. Please refer to Figure 227
Bit 0 RQCP0: Request completed mailbox0
  Set by hardware when the last request (transmit or abort) has been performed.
  Cleared by software writing a “1” or by hardware on transmission request (TXRQ0 set in
  CAN_TI0R register).
  Clearing this bit clears all the status bits (TXOK0, ALST0 and TERR0) for Mailbox 0.
*/
void CAN_TX_IRQHandler(struct CAN_CTLBLOCK* pctl)
{
	 __attribute__((__unused__))int temp;	// Dummy for readback of hardware registers


	/* JIC: mailboxes 1 & 2 are not used and should not have a flag */
	if ((CAN_TSR(pctl->vcan) & (CAN_TSR_RQCP1 | CAN_TSR_RQCP2)) != 0)
	{ // Here, something bogus going on.
		CAN_TSR(pctl->vcan) = (CAN_TSR_RQCP1 | CAN_TSR_RQCP2);	// Turn flags OFF
		pctl->can_errors.can_cp1cp2 += 1;	// Count: (RQCP1 | RQCP2) unexpectedly ON
		temp = CAN_TSR(pctl->vcan);	// JIC Prevent tail-chaining
		return;
	}

	u32 tsr = CAN_TSR(pctl->vcan);	// Copy of status register mailbox 0 bits

	/* Do this early so it percolates down into the hardware. */
	CAN_TSR(pctl->vcan) = CAN_TSR_RQCP0;	// Clear mailbox 0: RQCP0 (which clears TERR0, ALST0, TXOK0)

	/* JIC we got a TX completion when we did not load the mailbox, e.g. initialization. */
	if (pctl->pend.plinknext == NULL) // Is the linked list empty?
	{ // Here, yes.  Something bogus, and never try to remove a block when the list is empty!
		pctl->can_errors.txint_emptylist += 1; // Count: TX interrupt with pending list empty
		return;
	}

	/* Check for a bogus interrupt. */
	if ( (tsr & CAN_TSR_RQCP0) == 0) // Is mailbox0 RQCP0 (request complete) ON?
	{ // Here, no RXCPx bits are on, so interrupt is bogus.
		temp = CAN_TSR(pctl->vcan);	// JIC Prevent tail-chaining
		return;
	}

	if ((tsr & CAN_TSR_TXOK0) != 0) // TXOK0: Transmission OK for mailbox 0?
	{ // Here, yes. Flag msg for removal from pending list.
		moveremove2(pctl);	// remove from pending list, add to free list
	}
	else if ((tsr & CAN_TSR_TERR0) != 0) // Transmission error for mailbox 0? 
	{ // Here, TERR error bit, so try it some more.
		pctl->can_errors.can_txerr += 1; 	// Count total CAN errors
		pctl->pxprv->plinknext->x.xb[0] += 1;	// Count errors for this msg
		if (pctl->pxprv->plinknext->x.xb[0] > pctl->pxprv->plinknext->x.xb[1])
		{ // Here, too many error, remove from list
			pctl->can_errors.can_tx_bombed += 1;	// Number of bombouts
			moveremove2(pctl);	// Remove msg from pending queue
		}
	}
	else if ((tsr & CAN_TSR_ALST0) != 0) // Arbitration lost for mailbox 0?
	{ // Here, arbitration for mailbox 0 failed.
		pctl->can_errors.can_tx_alst0_err += 1; // Running ct of arb lost: Mostly for debugging/monitoring
		if ((pctl->pxprv->plinknext->x.xb[2] & SOFTNART) != 0)
		{ // Here this msg was not to be re-sent, i.e. NART
			pctl->can_errors.can_tx_alst0_nart_err += 1; // Mostly for debugging/monitoring
			moveremove2(pctl);	// Remove msg from pending queue
		}
	}
	else
	{ // Here, no bits on, therefore something bogus.
		pctl->can_errors.can_no_flagged += 1; // Count for monitoring purposes
	}

	loadmbx2(pctl);		// Load mailbox 0.  Mailbox should be available/empty.
	temp = CAN_TSR(pctl->vcan);	// JIC Prevent tail-chaining
	return;
}
/* --------------------------------------------------------------------------------------
* static void moveremove2(struct CAN_CTLBLOCK* pctl);
* @brief	: Remove msg from pending list and add to free list
  --------------------------------------------------------------------------------------- */
static void moveremove2(struct CAN_CTLBLOCK* pctl)
{
	volatile struct CAN_POOLBLOCK* pmov;
	u32 save[2];

	disable_ints(save);	// TX or RX(other) interrupts might remove a msg from the free list.
	/* Remove from pending; move to free list. */
//?	if ((pctl->pfor != NULL) && (pctl->pfor == pctl->pxprv->plinknext)) // Does pfor need adjustment?
//?	{ // Here, yes. 'pfor' points to the msg removed in the following statements.
//?		pctl->pfor = pctl->pxprv; // Move 'pfor' back "up" one msg in linked list
//?		pctl->can_errors.can_pfor_bk_one += 1; // Keep track of instances
//?	}
	// Remove from pending list
	pmov = pctl->pxprv->plinknext;	// Pts to removed item
	pctl->pxprv->plinknext = pmov->plinknext;

	// Adding to free list
	pmov->plinknext = frii.plinknext; 
	frii.plinknext  = pmov;
	pctl->txbct -= 1;	// Decrement running ct of pending list msgs
	friict += 1;		// Increment running ct of free list inventory
	reenable_ints(save);
	return;
}
/*#######################################################################################
 * ISR routine for CAN: "FIFO 1 interrupt" -- FMP1, FULL1, FOVR1
 *####################################################################################### */
struct RXIRQPARAM	// For passing constants to RX IRQ Handler
{
	struct CAN_CTLBLOCK** pctl_addr;// pctl0|1
	u32 fifo;			// CAN_FIFO0|CAN_FIFO1
	u32 rfr;			// CAN_RFR0|CAN_RFR1
};
volatile u32 debugT1;
volatile int debugT3;
volatile int debugT2 = 0; //0x7fffffff;

void CAN_RX_IRQHandler(const struct RXIRQPARAM* p, struct CAN_RCV_PTRS* ptrs)
{
	u32 save[2];				// Save CAN1 and CAN2 IER registers
	struct CAN_CTLBLOCK* pctl = *p->pctl_addr;// Get control block pointer
	struct CAN_POOLBLOCK* pnew;	// Temp block pointer
	u32 fifo1DTW = DTWTIME;	// Save time of msg arrival

	/* While away any and all FIFO msgs.  This also prevents an interrupt 
		re-entry that has no msg from storing a duplicate msg.  */
	while ((CAN_RFxR(pctl->vcan, p->rfr) & 0x03) != 0)
	{ // Here, FMPx shows FIFO has one or more msgs.
		/* Get a free block from the free list. */
		disable_ints(save);	// TX or RX(other) interrupts might remove a msg from the free list.
		pnew = (struct CAN_POOLBLOCK*)frii.plinknext; // cast to stop warning that pnew not volatile
		if ((pnew == NULL) || (ptrs->bct >= ptrs->bmx))
		{ // Here, either no free list blocks OR this RX reached its limit
			CAN_RFxR(pctl->vcan, p->rfr) = (1 << 5);
			pctl->can_errors.can_msgovrflow += 1;	// Count overflows
			reenable_ints(save);
			return;	// Return failure: no space & screwed
		}	
		frii.plinknext = pnew->plinknext;
		friict -= 1; 		// Decrement running count of free list inventory
		ptrs->bct += 1;	// Increment running count of of RX pending list

		/* 'pnew' points to the block that is now not linked. */

		/* Copy time stamp and CAN register data to pnew block. */
		pnew->x.xw	   = fifo1DTW;			 // DTW tick time-stamp
		pnew->can.id       = CAN_RIxR (pctl->vcan, p->fifo);// ID, RTR, IDE
		pnew->can.dlc	   = CAN_RDTxR(pctl->vcan, p->fifo);// time, data length CAN_RDTxR p 663
		pnew->can.cd.ui[0] = CAN_RDLxR(pctl->vcan, p->fifo);// Data (32b) Low
		pnew->can.cd.ui[1] = CAN_RDHxR(pctl->vcan, p->fifo);// Data (32b) High

		/* Release hardware FIFO 0 or 1 */	
		CAN_RFxR(pctl->vcan, p->rfr) = (1 << 5);	// Write bits to release FIFO msg

		if (ptrs->ptail == NULL)
		{ // Here, list is empty
			ptrs->ptail     = pnew;	// Add to taill
			pnew->plinknext = pnew; // Pt to head
		}		
		else
		{ // Here, one or more is on the list. */
			pnew->plinknext = ptrs->ptail->plinknext; // Move head ptr to new
			ptrs->ptail->plinknext = pnew;	// Pt to new tail previous tail block
			ptrs->ptail = pnew; 		// Pt to new tail
		}
		reenable_ints(save);

		/* Extend interrupt processing, if pointer set. */
		if (ptrs->func_rx != NULL)	// Skip if no address is setup
			(*ptrs->func_rx)(pctl, pnew);	// Go do something (like trigger low level int)
	}
debugT1 += 1;
	return;
}

u32 debugD = 0;
/* Fixed parameters for irq handling of RX0, RX1 for CAN1 and CAN2 */
static const struct RXIRQPARAM can1rx0 = {&pctl0, CAN_FIFO0, CAN_RFR0};
static const struct RXIRQPARAM can1rx1 = {&pctl0, CAN_FIFO1, CAN_RFR1};
static const struct RXIRQPARAM can2rx0 = {&pctl1, CAN_FIFO0, CAN_RFR0};
static const struct RXIRQPARAM can2rx1 = {&pctl1, CAN_FIFO1, CAN_RFR1};
/* ################### INTERRUPT VECTORS POINT TO THESE ############################################## */
// Pass pointer to 'const' struct, plus pointer to RX0, or RX1 struct (which is not a const)
void CAN1_TX_IRQHandler(void) {return CAN_TX_IRQHandler(pctl0);}
void CAN2_TX_IRQHandler(void) {return CAN_TX_IRQHandler(pctl1);}

void CAN1_RX0_IRQHandler(void){return CAN_RX_IRQHandler(&can1rx0, &pctl0->ptrs0); }
void CAN1_RX1_IRQHandler(void){return CAN_RX_IRQHandler(&can1rx1, &pctl0->ptrs1); }
void CAN2_RX0_IRQHandler(void){return CAN_RX_IRQHandler(&can2rx0, &pctl1->ptrs0); }
void CAN2_RX1_IRQHandler(void){return CAN_RX_IRQHandler(&can2rx1, &pctl1->ptrs1); }
/* ################################################################################################### */

