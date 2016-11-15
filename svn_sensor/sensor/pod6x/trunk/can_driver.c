/******************************************************************************
* File Name          : can_driver.c
* Date First Issued  : 05-28-2015
* Board              : F103 or F4
* Description        : CAN routines common for F103 and F4 CAN drivers
*******************************************************************************/
/*
NOTE: vector.c needs the following interrupt entry 
 labels in the appropriate interrupt vector locations--
(the following are the default F4 names in vector.c,
 the F103 names need changing.)
void CAN1_TX_IRQHandler
void CAN1_RX0_IRQHandler
void CAN1_RX0_IRQHandler

void CAN2_TX_IRQHandler
void CAN2_RX1_IRQHandler
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
#include "nvicdirect.h"
#include "../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/stm32/can.h"
#include "can_driver.h"
#include "can_driver_port.h"
#include "DTW_counter.h"

/* subroutine declarations */
static int can_driver_init_rxbuff(struct CAN_RCV_PTRS* ptrs, u16 nummsgsbuffed);
static void loadmbx2(struct CAN_CTLBLOCK* pctl);
static void moveremove2(struct CAN_CTLBLOCK* pctl);

/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern u32	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern u32	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

#define CAN_TIMEOUT	6000000		// Break 'while' loop count during CAN initialization

static struct CAN_CTLBLOCK* pctl0;	// Pointer to control block for CAN1
static struct CAN_CTLBLOCK* pctl1;	// Pointer to control block for CAN2

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
 * struct CAN_CTLBLOCK*  can_driver_init(struct CAN_PARAMS2 *p, u32 ntx, u32 nrx0, u32 nrx1);
 * @brief 	: Setup CAN pins and hardware
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @param	: ntx  = number of TX  msgs buffered
 * @param	: nrx0 = number of RX0 msgs buffered
 * @param	: nrx1 = number of RX1 msgs buffered
 * @return	: Pointer to control block for this CAN
 *		:  Pointer->ret = return code
 *		:  NULL = cannum not 1 or 2, calloc of control block failed
 *		:   0 success
 *		:  -1 cannum: CAN number not 1 or 2
 *		:  -2 calloc of linked list failed
 *		:  -3 RX0 get buffer failed
 *		:  -4 RX1 get buffer failed
 *		:  -5 port pin setup failed
 *		:  -6 CAN initialization mode timed out
 *		:  -7 Leave initialization mode timed out
*******************************************************************************/
struct CAN_CTLBLOCK* can_driver_init(struct CAN_PARAMS2 *p, u32 ntx, u32 nrx0, u32 nrx1)
{
	int ret;
	u32 can_timeout;
	struct CAN_CTLBLOCK* pctl;
	struct CAN_XMITBUFF* plst;
	struct CAN_XMITBUFF* ptmp;

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

	/* Get CAN xmit linked list and init 'plinknext'. */	
	plst = (struct CAN_XMITBUFF*)&pctl->ptx.frii;
	for (can_timeout = 0; can_timeout < ntx; can_timeout++)
	{
		ptmp = (struct CAN_XMITBUFF*)calloc(1, sizeof(struct CAN_XMITBUFF));
		if ( ptmp == NULL){pctl->ret = -2; return pctl;} // Get buff failed

		plst->plinknext = ptmp;
		plst = ptmp;
	}

	/* Get circular buffer for RX0 */
	ret = can_driver_init_rxbuff(&pctl->ptrs0, nrx0);
	if (ret != 0) {pctl->ret = -3; return pctl;} // failed

	/* Get circular buffer for RX1 */
	ret = can_driver_init_rxbuff(&pctl->ptrs1, nrx1);
	if (ret != 0) {pctl->ret = -4; return pctl;} // failed

	/* Setup CAN port pins and enable module clocking. */
	ret = can_driver_port(p->port, p->cannum);
	if (ret != 0) {pctl->ret = -5; return pctl;} // failed

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


	CAN_MCR(pctl->vcan) &= ~(0xfe);				// Clear bits except for INAQ
	CAN_MCR(pctl->vcan) |=  (bitcvt(p->ttcm) << 7);	// Time triggered communication mode
	CAN_MCR(pctl->vcan) |=  (bitcvt(p->abom) << 6);	// Automatic bus-off management
	CAN_MCR(pctl->vcan) |=  (bitcvt(p->awum) << 5);	// Auto WakeUp Mode
	CAN_MCR(pctl->vcan) |=  (1 << 4);			// No Automatic ReTry (0 = retry; non-zero = xmit only once)

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

	/* CAN interrupt enable register (CAN_IER) */
	CAN_IER(pctl->vcan) |= CAN_IER_FMPIE0;	// FIFO 0 message pending interrupt enable
	CAN_IER(pctl->vcan) |= CAN_IER_FMPIE1;	// FIFO 1 message pending interrupt enable
	CAN_IER(pctl->vcan) |= 0x1;	// TX Interrupt generated when RQCP0 set

	/* Wait until ready */
	while ( (CAN_TSR(pctl->vcan) & CAN_TSR_TME0) == 0);         // Wait for transmit mailbox 0 to be empty

	pctl->ret = 0;	// Set success code
	return pctl;	// Return pointer to control block
}
/*---------------------------------------------------------------------------------------------
 * static int can_driver_init_rxbuff(struct CAN_RCV_PTRS0* ptrs, u16 nummsgsbuffed);
 * @brief 	: Get memory for RX circular buffers
 * @param	: ptrs = pointer to struct of pointers for this buffer.
 * @param	: nummsgsbuffed = Number of CAN msgs to be buffered
 * @return	:  0 = success
 *		: -1 = failed
 ----------------------------------------------------------------------------------------------*/
static int can_driver_init_rxbuff(struct CAN_RCV_PTRS* ptrs, u16 nummsgsbuffed)
{
	struct CANRCVBUF* ptmp;

	/* Get contiguous block for a circular buffer. */
	ptmp = (struct CANRCVBUF*)calloc(nummsgsbuffed, sizeof(struct CANRCVTIMBUF) );
	if (ptmp == NULL) return -1; // Return if failed.

	/* Initial pointers to circular buffer. */
	ptrs->pstart = ptmp;
	ptrs->padd   = ptmp;
	ptrs->ptake  = ptmp;
	ptrs->plast  = ptmp + nummsgsbuffed;
	return 0;		// Success
}
/*---------------------------------------------------------------------------------------------
 * static struct CANRCVBUF*  adv_pointer0(struct CAN_RCV_PTRS* p);
 * @brief	: Advance circular buffer
 * @param	: p = pointer to be advanced
 * return	: new pointer
 ----------------------------------------------------------------------------------------------*/
static struct CANRCVBUF*  adv_pointer(struct CAN_RCV_PTRS* ptrs, struct CANRCVBUF* p)
{
	p += 1; if (p >= ptrs->plast) p = ptrs->pstart;
	return p;
}
/******************************************************************************
 * struct CANRCVBUF* can_driver_get0(struct CAN_CTLBLOCK* pctl);
 * @brief	: Get pointer to non-high priority CAN msg buffer (FIFO 0)
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
struct CANRCVBUF* can_driver_get0(struct CAN_CTLBLOCK* pctl)
{
	struct CANRCVBUF *p;
	if (pctl->ptrs0.ptake == pctl->ptrs0.padd) return NULL;
	p = pctl->ptrs0.ptake;	
	pctl->ptrs0.ptake = adv_pointer(&pctl->ptrs0, p);
	return p;		// Return pointer to buffer
}
/******************************************************************************
 * struct CANRCVBUF* can_driver_get1(struct CAN_CTLBLOCK* pctl);
 * @brief	: Get pointer to high priority CAN msg buffer (FIFO 1)
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
struct CANRCVBUF* can_driver_get1(struct CAN_CTLBLOCK* pctl)
{
	struct CANRCVBUF *p;
	if (pctl->ptrs1.ptake == pctl->ptrs1.padd) return NULL;
	p = pctl->ptrs1.ptake;	
	pctl->ptrs1.ptake = adv_pointer(&pctl->ptrs1, p);
	return p;		// Return pointer to buffer
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
 ******************************************************************************/
int can_driver_put(struct CAN_CTLBLOCK* pctl, struct CANRCVBUF *pcan, u8 maxretryct, u8 bits)
{
	struct CAN_XMITBUFF* pnew;
	struct CAN_TXPOINTERS* pcp = &pctl->ptx;

	/* Get a free block from the free list. */
	DISABLE_TXINT;	// TX interrupt might move a msg to the free list.
	pnew = pcp->frii.plinknext;
	if (pnew == NULL) 
	{
		ENABLE_TXINT;
		pctl->can_errors.can_msgovrflow += 1;	// Count overflows
		return -1;	// Return failure: no space & screwed
	}	
	pcp->frii.plinknext = pnew->plinknext;
	ENABLE_TXINT;
	/* 'pnew' now points to the block that is free (and not linked). */

	/* Build struct/block for addition to the pending list. */
	pnew->can = *pcan;	// Copy CAN msg.
	pnew->maxretryct = maxretryct;	// Maximum number of TERR retry counts
	pnew->bits	= bits;	// Use these bits to set some conditions (see .h file)
	pnew->retryct	= 0;	// Retry counter for TERRs.

	/* Find location to insert new msg.  Lower value CAN ids are higher priority, 
           and when the CAN id msg to be inserted has the same CAN id as the 'pfor' one
           already in the list, then place the new one further down so that msgs with 
           the same CAN id do not get their order of transmission altered. */
 	DISABLE_TXINT;
	for (pcp->pfor = &pcp->pend; (pcp->pfor)->plinknext != NULL; pcp->pfor = (pcp->pfor)->plinknext)
	{
		if (pnew->can.id < ((pcp->pfor)->plinknext)->can.id) // Pay attention: "value" vs "priority"
			break;
		/* Each loop of the search allow a TX interrupt to send the next msg. */
		ENABLE_TXINT;	// Open window to allow a TX interrupt
		DISABLE_TXINT;	// Close window for TX interrupts
	}

	/* Add new msg to pending list. (TX interrupt is still disabled) */
	pnew->plinknext = (pcp->pfor)->plinknext; 	// Insert new msg into 
	(pcp->pfor)->plinknext = pnew;			//   pending list.
	pcp->pfor =  NULL;	// Signal TX interrupt that search loop is not active.
	if (pcp->pxprv == NULL) // Is sending complete?
	{ // pxprv == NULL means CAN mailbox did not get loaded, so CAN is idle.
		loadmbx2(pctl); // Start sending
	}
	else
	{ // CAN sending is in progress.
		if ((pcp->pxprv)->plinknext == pnew) // Does pxprv need adjustment?
		{ // Here yes. We inserted a msg between 'pxprv' and 'pxprv->linknext'
			pcp->pxprv = pnew;	// Update 'pxprv' so that it still points to msg TX using.
			pctl->can_errors.can_pxprv_fwd_one += 1;	// Count: Instances that pxprv was adjusted in 'for' loop
		}
	}
	ENABLE_TXINT;

	return 0;	// Success!
}
/*#######################################################################################
 * ISR routines for CAN:There four interrupt vectors, p 647 fig 235
 *####################################################################################### */

/*---------------------------------------------------------------------------------------------
 * static void loadmbx2(struct CAN_CTLBLOCK* pctl)
 * @brief	: Load mailbox
 ----------------------------------------------------------------------------------------------*/
static u16 seqxmit2 = 0;
static void loadmbx2(struct CAN_CTLBLOCK* pctl)
{
	struct CAN_TXPOINTERS* pcp = &pctl->ptx;
	struct CAN_XMITBUFF* p = pcp->pend.plinknext;

	if (p == NULL)
	{
		pcp->pxprv = NULL;
		return; // Return if no more to send
	}
seqxmit2 += 1;
	pcp->pxprv = &pcp->pend;	// Save in a static var
if(p->can.id != 0x00200000)
{
  p->can.cd.us[1] = seqxmit2;
  p->can.cd.ui[1] = DTWTIME;
  seqxmit2 = 0;
}
else

	/* Load the mailbox with the message.  CAN ID low bit starts xmission. */
	CAN_TDTxR(pctl->vcan, CAN_MBOX0) =  p->can.dlc;	 	// CAN_TDT0R:  mailbox 0 time & length p 660
	CAN_TDLxR(pctl->vcan, CAN_MBOX0) =  p->can.cd.ui[0];	// CAN_TDL0RL: mailbox 0 data low  register p 661 
	CAN_TDHxR(pctl->vcan, CAN_MBOX0) =  p->can.cd.ui[1];	// CAN_TDL0RH: mailbox 0 data low  register p 661 
	/* Load CAN ID and set TX Request bit */
	CAN_TIxR (pctl->vcan, CAN_MBOX0) =  (p->can.id | 0x1); 	// CAN_TI0R:   mailbox 0 identifier register p 659
	return;
}
/*#######################################################################################
 * ISR routine for CAN: "Transmit Interrupt" <-- (RQCP0 | RQCP1 | RQCP2)
 *####################################################################################### */
/* p 647 The transmit interrupt can be generated by the following events:
–   Transmit mailbox 0 becomes empty, RQCP0 bit in the CAN_TSR register set.
–   Transmit mailbox 1 becomes empty, RQCP1 bit in the CAN_TSR register set.
–   Transmit mailbox 2 becomes empty, RQCP2 bit in the CAN_TSR register set. */

/*
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
void CAN_TX_IRQHandler(struct CAN_CTLBLOCK* pctl);

void CAN2_TX_IRQHandler(void){	return CAN_TX_IRQHandler(pctl1);}
void CAN1_TX_IRQHandler(void){	return CAN_TX_IRQHandler(pctl0);}

void CAN_TX_IRQHandler(struct CAN_CTLBLOCK* pctl)
{
	
	 __attribute__((__unused__))int temp;	// Dummy for readback of hardware registers
	struct CAN_TXPOINTERS* pcp = &pctl->ptx;

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
		pcp->pxprv->plinknext->retryct += 1;	// Count errors for this msg
		if (pcp->pxprv->plinknext->retryct > pcp->pxprv->plinknext->maxretryct)
		{ // Here, too many error, remove from list
			pctl->can_errors.can_tx_bombed += 1;	// Number of bombouts
			moveremove2(pctl);	// Remove msg from pending queue
		}
	}
	else if ((tsr & CAN_TSR_ALST0) != 0) // Arbitration lost for mailbox 0?
	{ // Here, arbitration for mailbox 0 failed.
		pctl->can_errors.can_tx_alst0_err += 1; // Running ct of arb lost: Mostly for debugging/monitoring
		if ((pcp->pxprv->plinknext->bits & SOFTNART) != 0)
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
	struct CAN_TXPOINTERS* pcp = &pctl->ptx;
	struct CAN_XMITBUFF* pmov;
	/* Remove from pending; move to free list. */
	if ((pcp->pfor != NULL) && (pcp->pfor == pcp->pxprv->plinknext)) // Does pfor need adjustment?
	{ // Here, yes. 'pfor' points to the msg removed in the following statements.
		pcp->pfor = pcp->pxprv; // Move 'pfor' back "up" one msg in linked list
		pctl->can_errors.can_pfor_bk_one += 1; // Keep track of instances
	}
	// Remove from pending list
	pmov = pcp->pxprv->plinknext;	// Pts to removed item
	pcp->pxprv->plinknext = pmov->plinknext;

	// Adding to free list
	pmov->plinknext = pcp->frii.plinknext; 
	pcp->frii.plinknext  = pmov;
	return;
}

/*#######################################################################################
 * ISR routine for CAN: "FIFO 1 interrupt" -- FMP1, FULL1, FOVR1
 *####################################################################################### */
/* p 647,8 The FIFO 1 interrupt can be generated by the following three events (only X = enabled):
X   Reception of a new message, FMP1 bits in the CAN_RF1R register are not ‘00’.
–   FIFO1 full condition, FULL1 bit in the CAN_RF1R register set.
–   FIFO1 overrun condition, FOVR1 bit in the CAN_RF1R register set. */
void CAN_RX_IRQHandler(struct CAN_CTLBLOCK* pctl, u32 fifo, u32 rfr, struct CAN_RCV_PTRS* ptrs)
{
	 __attribute__((__unused__))int temp;

	u32 fifo1DTW = DTWTIME;	// Save time of msgs arrival

	/* Save time & message in a circular buffer */
	ptrs->padd->id       = CAN_RIxR (pctl->vcan, fifo);	// ID, RTR, IDE
	ptrs->padd->dlc	     = CAN_RDTxR(pctl->vcan, fifo);	// time, data length CAN_RDTxR p 663
	ptrs->padd->cd.ui[0] = CAN_RDLxR(pctl->vcan, fifo);	// Data (32b) Low
	ptrs->padd->cd.ui[1] = CAN_RDHxR(pctl->vcan, fifo);	// Data (32b) High

	/* Release FIFO */	
	CAN_RFxR(pctl->vcan, rfr) |= CAN_RF1R_RFOM1;		// Write bit 5 to release FIFO msg

	/* Extend interrupt processing, if pointer set. */
	if (pctl->func_rx != NULL)	// Skip if no address is setup
		(*pctl->func_rx)(pctl, ptrs, fifo1DTW);	// Go do something
	
	/* Advance index in circular buffer */
	struct CANRCVBUF* ptmp;
	ptmp = adv_pointer(ptrs, ptrs->padd);

	if (ptmp != ptrs->ptake) // This index advance overrun?
	{ // Here, no, we are not going to overrun the buffer
		ptrs->padd = ptmp;
	}
	else
	{ // Here yes. Let us count the times
		pctl->can_errors.can_rx1err += 1; //
	}		

	temp = CAN_RF1R(pctl->vcan);	// Read back register to avoid tail-chaining

	return;
}
void CAN1_RX0_IRQHandler(void){	return CAN_RX_IRQHandler(pctl0, CAN_FIFO0, CAN_RFR0, &pctl0->ptrs0); }
void CAN2_RX0_IRQHandler(void){	return CAN_RX_IRQHandler(pctl1, CAN_FIFO0, CAN_RFR0, &pctl1->ptrs0); }
void CAN1_RX1_IRQHandler(void){	return CAN_RX_IRQHandler(pctl0, CAN_FIFO1, CAN_RFR1, &pctl0->ptrs1); }
void CAN2_RX1_IRQHandler(void){	return CAN_RX_IRQHandler(pctl1, CAN_FIFO1, CAN_RFR1, &pctl1->ptrs1); }

