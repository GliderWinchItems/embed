/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : canwinch_pod.c
* Hackerees          : deh
* Date First Issued  : 12/07/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : CAN routines for winch instrumentation--POD board
*******************************************************************************/
/* 
Page numbers: Ref Manual RM0008 rev 14
CAN starts at p 628.

TIM1 & TIM4 are used for low priority processing after a CAN interrupt.

High priority msgs (RESETALL, TIMESYNC1,2,3 are assigned to FIFO 1, others to FIFO 0.


*/

#include "libmiscstm32/systick1.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/can.h"
#include "PODpinconfig.h"
#include "pinconfig_all.h"
#include "pinconfig.h"
#include "common_time.h"
#include "canwinch_pod.h"

#define NVIC_TIM1_IRQ 	NVIC_TIM1_CC_IRQ

/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern u32	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern u32	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* Interrupts with received messages that are accepted by the hardware filtering are stored in
   a buffer.  The mainline gets a pointer to the buffer, or a NULL if the buffer is empty. */
// The following is for non-high priority CAN msgs (e.g. sensor readings, date/time/tick count, prog loadiing)

#define CANRCVBUFSIZE	32
static struct CANRCVSTAMPEDBUF canbuf[CANRCVBUFSIZE];	// CAN message circular buffer
static int canbufIDXm = 0;		// Index for mainline removing data from buffer
static int canbufIDXi = 0;		// Index for interrupt routine adding data
// The following is for high priority CAN msgs (e.g. RESETALL, TIMESYNCx)

#define CANRCVTIMBUFSIZE	8	// Can data plus timer ticks 
static struct CANRCVTIMBUF cantimbuf[CANRCVTIMBUFSIZE];	// CAN message circular buffer
static int canbuftimIDXm = 0;		// Index for mainline removing data from buffer
static int canbuftimIDXi = 0;		// Index for interrupt routine adding data

/* Pointers to functions to be executed under a low priority interrupt, forced by a CAN interrupt */
void 	(*highpriority_ptr)(void) = 0;	// Address of function to call forced interrupt
void 	(*lowpriority_ptr)(void) = 0;	// Address of function to call forced interrupt

/* Transmit buffering */
#define CANXMTBUFSIZE	8		// Number of message that can be queued for sending
static int canxmtIDXi = 0;		// Index into buffer with xmt messages (interrupt/sending)
static int canxmtIDXm = 0;		// Index into buffer to next available slot to add a xmt message
static struct CANRCVBUF canxmtbuf[CANXMTBUFSIZE];	// Messages queued for sending

/* Filtering */
static u16 can_filt16num;	// 16 bit scale filter number: last assigned

#define CAN_TIMEOUT	2000000		// Break 'while' loop count

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
 * int can_init_pod(struct CAN_PARAMS *p);
 * @brief 	: Setup CAN pins and hardware
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @return	:  n = remaining counts; 0 = enter init mode timedout;-1 = exit init mode timeout; -2 = bad port
*******************************************************************************/
int can_init_pod(struct CAN_PARAMS *p)
{
	int can_timeout;	// Counter for breaking 'while' loops

	/* Enable clocking to CAN module */
	RCC_APB1ENR |= (1<<25);		// CAN1_EN p 144

	RCC_APB2ENR |= RCC_APB2ENR_AFIOEN;	// enable clock for Alternate Function

/* NOTE: The following statement is located in '../lib/libmiscstm32/clocksetup.c' & 'clockspecifysetup.c' */
//	RCC_APB2ENR |= 0x7c;	// Enable A thru Ebvc 

	 /* Note: calculations for PCLK1 = 36MHz */
//	unsigned int brp = (pclk1_freq / 18) / 500000;	// baudrate is set to 500k bit/s

	/* REMAP p 180
	Bits 14:13 CAN_REMAP[1:0]: CAN alternate function remapping
            These bits are set and cleared by software. They control the mapping of alternate functions
            CAN_RX and CAN_TX in devices with a single CAN interface.
            00: CAN_RX mapped to PA11, CAN_TX mapped to PA12
            01: Not used
            10: CAN_RX mapped to PB8, CAN_TX mapped to PB9 (not available on 36-pin package)
            11: CAN_RX mapped to PD0, CAN_TX mapped to PD1 
	OLIMEX = 0x2
	*/

/* Output pins configure for alternate function output push-pull */
const struct PINCONFIGALL pa12 = {(volatile u32 *)GPIOA, 12, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL pb9  = {(volatile u32 *)GPIOB,  9, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL pd1  = {(volatile u32 *)GPIOD,  1, OUT_AF_PP, MHZ_50};

/* Input pins configure for input pull-up */
const struct PINCONFIGALL pa11 = {(volatile u32 *)GPIOA, 11, IN_PU, 0};
const struct PINCONFIGALL pb8  = {(volatile u32 *)GPIOB,  8, IN_PU, 0};
const struct PINCONFIGALL pd0  = {(volatile u32 *)GPIOD,  1, IN_PU, 0};


	/* Setup remapping and configure port pins */
	switch (p->port)
	{
	case 0:	// CAN on port A

		/*  Setup CAN TXD: PA12 for alternate function push/pull output p 156, p 163  */
		pinconfig_all((struct PINCONFIGALL *)&pa12);

		/* Setup CAN RXD: PB11 for input pull up p 164, 167 */
		pinconfig_all((struct PINCONFIGALL *)&pa11);

		AFIO_MAPR |= (0x0 << 13);	 // 00: CAN_RX mapped to PA11, CAN_TX mapped to PA12 p 179
		break;

	case 2:	// CAN on port B

		/*  Setup CAN TXD: PB9 for alternate function push/pull output p 156, p 163  */
		pinconfig_all((struct PINCONFIGALL *)&pb9);

		/* Setup CAN RXD: PB8 for input pull up p 164, 167 */
		pinconfig_all((struct PINCONFIGALL *)&pb8);

		AFIO_MAPR |= (0x2 << 13);	 // 00: CAN_RX mapped to PA11, CAN_TX mapped to PA12 p 179
		break;

	case 3:	// CAN on port D

		/*  Setup CAN TXD: PD1 for alternate function push/pull output p 156, p 163  */
		pinconfig_all((struct PINCONFIGALL *)&pd1);

		/* Setup CAN RXD: PD0 for input pull up p 164, 167 */
		pinconfig_all((struct PINCONFIGALL *)&pd0);

		AFIO_MAPR |= (0x3 << 13);	 // 00: CAN_RX mapped to PA11, CAN_TX mapped to PA12 p 179
		break;
	default:
		return -2;
		break;
	}

	/* ---------- Set Initialization mode -------------------- */
	/* Request initialization p 632, 648.  DEBUG freeze bit on */
	CAN_MCR(CAN1) &= CAN_MCR_DBF;	// Clear DBF since it is set coming out RESET
	CAN_MCR(CAN1) = ((bitcvt(p->dbf) << 16) | (CAN_MCR_INRQ) );

	/* The initialization request (above) causes the INAK bit to be set.  This bit goes off when 11 consecutive
	   recessive bits have been received p 633 */
	can_timeout = CAN_TIMEOUT;	// Counter to break loop for timeout
	while ( ((CAN_MSR(CAN1) & CAN_MSR_INAK) == 0 ) && (can_timeout-- >= 0) );	// Wait until initialization mode starts or times out
	if (can_timeout <= 0 ) return 0;	// Timed out

	/* Compute baud rate prescalar code (BRP), given the baud rate and pclk1_freq, and sampling intervals p 645, fig 233 */
	/* set BTR register so that sample point is at about 72% bit time from bit start */
//	#define TBS1	12	// Bit segment 1
//	#define TBS2	5	// Bit segment 2
	u32 brp;		// Baud rate code
	brp = (pclk1_freq/(1 + (bitcvt(p->tbs1)) + (bitcvt(p->tbs2)))) / p->baudrate;

        /* SJW--Resynchronization jump width: These bits define the maximum number of time quanta the CAN hardware 
	   is allowed to lengthen or shorten a bit to perform the resynchronization. p 640 */
//	#define SJW 4	

	/* Clear and set bit fields p 657 */
	//                         SJW                      TBS2                       TBS1                      BRP
//	CAN_BTR(CAN1) &= ~( ((          0x03) << 24) | ((           0x07) << 20) | ((           0x0F) << 16) | ((          0x1FF) << 0) ); 
//	CAN_BTR(CAN1) |=  ( (((SJW-1) & 0x03) << 24) | (((TBS2-1) & 0x07) << 20) | (((TBS1-1) & 0x0F) << 16) | (((brp-1) & 0x1FF) << 0) );


	CAN_BTR(CAN1) = 0x01230000; 	// Set register to RESET value p 657

	CAN_BTR(CAN1) |=    (bitcvt(p->silm)            << 31);	// Silent mode bit
	CAN_BTR(CAN1) |=    (bitcvt(p->lbkm)            << 30);	// Loopback mode bit
	CAN_BTR(CAN1) |=  (((bitcvt(p->sjw )-1) & 0x03) << 24);	// Resynchronization jump width
	CAN_BTR(CAN1) |=  (((bitcvt(p->tbs2)-1) & 0x07) << 20);	// Time segment 2
	CAN_BTR(CAN1) |=  (((bitcvt(p->tbs1)-1) & 0x0F) << 16); // Time segment 1
	CAN_BTR(CAN1) |=  (((brp-1)            & 0x3FF)  << 0); // Baud rate prescalar

	// Loopback, Silent Mode (self-test) (BTR p 657)
//	CAN_BTR(CAN1) &= ~(CAN_BTR_SILM | CAN_BTR_LBKM); // Silent mode | Loopback mode
//	CAN_BTR(CAN1) |= (mode & (CAN_BTR_SILM | CAN_BTR_LBKM) );

	/* p 649 Option bits in CAN_MCR
Bit 7 TTCM: Time triggered communication mode
        0: Time Triggered Communication mode disabled.
        1: Time Triggered Communication mode enabled
      Note: For more information on Time Triggered Communication mode, please refer to
              Section 24.7.2: Time triggered communication mode.
Bit 6 ABOM: Automatic bus-off management
        This bit controls the behavior of the CAN hardware on leaving the Bus-Off state.
        0: The Bus-Off state is left on software request, once 128 occurrences of 11 recessive bits
        have been monitored and the software has first set and cleared the INRQ bit of the
        CAN_MCR register.
        1: The Bus-Off state is left automatically by hardware once 128 occurrences of 11 recessive
        bits have been monitored.
        For detailed information on the Bus-Off state please refer to Section 24.7.6: Error
        management.
Bit 5 AWUM: Automatic wakeup mode
        This bit controls the behavior of the CAN hardware on message reception during Sleep
        mode.
        0: The Sleep mode is left on software request by clearing the SLEEP bit of the CAN_MCR
        register.
        1: The Sleep mode is left automatically by hardware on CAN message detection.
        The SLEEP bit of the CAN_MCR register and the SLAK bit of the CAN_MSR register are
        cleared by hardware.
Bit 4 NART: No automatic retransmission
        0: The CAN hardware will automatically retransmit the message until it has been
        successfully transmitted according to the CAN standard.
        1: A message will be transmitted only once, independently of the transmission result
        (successful, error or arbitration lost).
 */
	CAN_MCR(CAN1) &= ~(0xfe);				// Clear bits except for INAQ
	CAN_MCR(CAN1) |=  (bitcvt(p->ttcm) << 7);	// Time triggered communication mode
	CAN_MCR(CAN1) |=  (bitcvt(p->abom) << 6);	// Automatic bus-off management
	CAN_MCR(CAN1) |=  (bitcvt(p->awum) << 5);	// Auto WakeUp Mode
	CAN_MCR(CAN1) |=  (bitcvt(p->nart) << 4);	// No Automatic ReTry (0 = retry; non-zero = auto retry)

	/* Leave initialization mode */
	CAN_MCR(CAN1) &= ~CAN_MCR_INRQ;
	can_timeout = CAN_TIMEOUT;	// Counter to break loop for timeout
	while ( ((CAN_MSR(CAN1) & CAN_MSR_INAK) != 0 ) && (can_timeout-- >= 0) );	// Wait until initialization mode starts or times out
	if (can_timeout <= 0 ) return -1;	// Timed out

	/* Set and enable interrupt controller for doing software interrupt */
	// Handles high priority CAN msgs, e.g. RESETALL, TIMESYNCx
	NVICIPR (NVIC_I2C1_EV_IRQ, NVIC_I2C1_EV_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C1_EV_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	// Handles non-high priority msgs, e.g. sensor readings
	NVICIPR (NVIC_I2C1_ER_IRQ, NVIC_I2C1_ER_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C1_ER_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')


	/* Set and enable interrupt controller for CAN interrupts */
	NVICIPR (NVIC_CAN_RX1_IRQ, NVIC_CAN_RX1_IRQ_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_CAN_RX1_IRQ);				// Enable interrupt controller

	NVICIPR (NVIC_CAN_SCE_IRQ, NVIC_CAN_SCE_IRQ_PRIORITY );// Set interrupt priority
	NVICISER(NVIC_CAN_SCE_IRQ);				// Enable interrupt controller

	NVICIPR (NVIC_USB_HP_CAN_TX_IRQ, NVIC_USB_HP_CAN_TX_IRQ_PRIORITY );// Set interrupt priority
	NVICISER(NVIC_USB_HP_CAN_TX_IRQ);				// Enable interrupt controller

	NVICIPR (NVIC_USB_LP_CAN_RX0_IRQ, NVIC_USB_LP_CAN_RX0_IRQ_PRIORITY );// Set interrupt priority
	NVICISER(NVIC_USB_LP_CAN_RX0_IRQ);				// Enable interrupt controller

	/* CAN interrupt enable register (CAN_IER) */
	/* Bit 1 FMPIE0: FIFO message pending interrupt enable
	       0: No interrupt generated when state of FMP[1:0] bits are not 00b.
	       1: Interrupt generated when state of FMP[1:0] bits are not 00b.
	   Bit 4 FMPIE1: FIFO message pending interrupt enable
	       0: No interrupt generated when state of FMP[1:0] bits are not 00b.
	       1: Interrupt generated when state of FMP[1:0] bits are not 00b. */
	CAN_IER(CAN1) |= CAN_IER_FMPIE0;	// FIFO 0 p 655
	CAN_IER(CAN1) |= CAN_IER_FMPIE1;	// FIFO 1 p 655
	CAN_IER(CAN1) |= 0x1;	// Bit0: 1: Interrupt generated when RQCPx bit is set. p 656.

	/* Wait until ready */
	while ( (CAN_TSR(CAN1) & CAN_TSR_TME0) == 0);         // Wait for transmit mailbox 0 to be empty

	return (CAN_TIMEOUT - can_timeout);	// Return wait loop count
}
/*******************************************************************************
 * void can_nxp_setRS(int rs);
 * @brief 	: Set RS input to NXP CAN driver (TJA1051) (on some PODs)
 * @param	: rs: 0 = NORMAL mode; not-zero = SILENT mode 
 * @return	: Nothing for now.
*******************************************************************************/
void can_nxp_setRS(int rs)
{
	/* RS (S) control PB7 (on sensor board) PD11 on pod board */
	// Floating input = resistor controls slope
	// Pin HI = Normal mode
	// Pin LO = sleep/silent mode
	configure_pin ((volatile u32 *)GPIOD, 11);	// configured for push-pull output
	if (rs == 0)
		GPIO_BRR(GPIOD)  |= (1<<11);	// Set bit LO for SILENT mode
	else
		GPIO_BSRR(GPIOD) |= (1<<11);	// Set bit HI for NORMAL mode
	return;
}

/*******************************************************************************
 * void can_filter_unitid(u32 myunitid);
 * @brief 	: Setup filter bank for Checksum check & Program loading
 * @param	: "my" can id
 * @return	: Nothing for now.
*******************************************************************************/
void can_filter_unitid(u32 myunitid)
{
/* p 632
      To initialize the registers associated with the CAN filter banks (mode, scale, FIFO
      assignment, activation and filter values), software has to set the FINIT bit (CAN_FMR). Filter
      initialization also can be done outside the initialization mode.
Note: When FINIT=1, CAN reception is deactivated.
      The filter values also can be modified by deactivating the associated filter activation bits (in
      the CAN_FA1R register).
      If a filter bank is not used, it is recommended to leave it non active (leave the corresponding
      FACT bit cleared).
*/
	/* CAN filter master register p 665 */
	CAN_FMR(CAN1)  |= CAN_FMR_FINIT;	// FINIT = 1; Initialization mode ON for setting up filter banks

	/* CAN filter scale register p 639 p 666.  Default is dual 16 bit mode. */
	// CAN_FS1R(CAN1) |= ????

	/* CAN filter mode register  p 639 666 */
	// CAN_FM1R(CAN1) |= ????

	/* This register can only be modified when the FACTx bit of the CAN_FAxR register is cleared
	   or when the FINIT bit of the CAN_FMR register is set. */

	/*   Set 16 bit filter mask and ID p 640 fig 229 */
	// Note: '>>16' is because for 16b scale mode the unit id goes in the low order word.
	// (CANx, <bank>) =  <[15:8] mask>      <[7:0] id>
	CAN_FiR1(CAN1, 0) = CAN_UNITID_MASK | (CAN_RESETALL  >> 16);	// All units accept this msg
	CAN_FiR2(CAN1, 0) = CAN_UNITID_MASK | (CAN_TIMESYNC1 >> 16);	// All units accept this msg
	CAN_FiR1(CAN1, 1) = CAN_UNITID_MASK | (CAN_TIMESYNC2 >> 16);	// All units accept this msg
	CAN_FiR2(CAN1, 1) = CAN_UNITID_MASK | (CAN_TIMESYNC3 >> 16);	// All units accept this msg
	CAN_FiR1(CAN1, 2) = CAN_UNITID_MASK | (myunitid      >> 16);	// 'IAMUNITNUMBER' is unique to this unit
	CAN_FiR2(CAN1, 2) = CAN_NEVERUSEID;			// Set leftover odd 16b mask|id 
	can_filt16num = 5;	// Show that filter bank 2, FiR2 is available

/* ==> NOTES <==
For the situation--Using the default scale: each 32b register of the filter bank *pair* holds a 16b mask and 16b id
1) The filter bank registers are NOT initialized by reset, i.e. they hold random data out of reset.
2) Both (32b) registers of the pair must be setup, otherwise the register that is not set will be used in the filtering,
   and this can cause strange results since the mask|id might pass unwanted msgs.
3) Setting a register to zero passes *all* messages, (since the mask is 100% don't cares).
*/
	/* Assign filter to FIFO 1. Default: All messages pass through FIFO p 667 */
	CAN_FFA1R(CAN1) |= 0x3;	// Filter banks 0 and 1 pass through FIFO 1

	/* Filter activation */
	/* Bits 27:0 FACTx: Filter active
            The software sets this bit to activate Filter x. To modify the Filter x registers (CAN_FxR[0:7]),
            the FACTx bit must be cleared or the FINIT bit of the CAN_FMR register must be set.
            0: Filter x is not active
            1: Filter x is active */
	CAN_FA1R(CAN1) |= 0x7;	// Activate filter banks 0, 1, 2. p 667

	/* Remove filter registers from initialization mode */
	CAN_FMR(CAN1)  &= ~CAN_FMR_FINIT;	// FINIT = 0; Set initialization mode off for filter banks p 665

	return;
}
/*******************************************************************************
 * u16 can_filtermask16_add(u32 m);
 * @brief 	: Add a 16 bit id & mask to the filter banks
 * @param	: mask and ID: e.g. (CAN_UNITID_MASK | (CAN_TIMESYNC3 >> 16)
 * @return	: Filter number; negative for you have filled it up!
 NOTE: These mask|ID's assign to the FIFO 0 (default)
*******************************************************************************/
u16 can_filtermask16_add(u32 m)
{
	/* F103 has 14 banks that can hold two 16 bit mask|ID's */
	if (can_filt16num >= (14 * 2)) return -1;

	/* CAN filter master register p 665 */
	CAN_FMR(CAN1)  |= CAN_FMR_FINIT;	// FINIT = 1; Initialization mode ON for setting up filter banks

	if ((can_filt16num & 0x1) != 0)	// Does the last filter bank assigned have an used slot?
	{ // Here, odd, therefore we can add this to the currently assigned filter bank
		CAN_FiR2(CAN1,(can_filt16num >> 1)) = m;	// Fill the 2nd slot
	}
	else
	{ /* Here, the count is even, so a new filter bank must be used */
		CAN_FiR1(CAN1,(can_filt16num >> 1)) = m;	// Fill the 1st slot
		CAN_FiR2(CAN1,(can_filt16num >> 1)) = CAN_NEVERUSEID;	// Initialize the odd, 2nd 16b mask|id 

		/* Activate the newly assigned filter bank */
		CAN_FA1R(CAN1) |= (1 << (can_filt16num >> 1));
	}

	/* Update the count of mask|ID's we have set up */
	can_filt16num += 1;	// Show another was added

	/* Remove filter registers from initialization mode */
	CAN_FMR(CAN1)  &= ~CAN_FMR_FINIT;	// FINIT = 0; Set initialization mode off for filter banks p 665
	
	return 0;
}
/******************************************************************************
 * struct CANRCVSTAMPEDBUF* canrcv_get(void);
 * @brief	: Get pointer to non-high priority CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
struct CANRCVSTAMPEDBUF* canrcv_get(void)
{
	struct CANRCVSTAMPEDBUF *p;
	if (canbufIDXi == canbufIDXm) return 0;	// Return showing no new data

	p = &canbuf[canbufIDXm];	// Get return pointer value

	canbufIDXm += 1; if (canbufIDXm >= CANRCVBUFSIZE) canbufIDXm = 0; // Wrap around end	
	return p;			// Return pointer to buffer
}
/******************************************************************************
 * struct CANRCVTIMBUF * canrcvtim_get(void);
 * @brief	: Get pointer to high priority CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
struct CANRCVTIMBUF* canrcvtim_get(void)
{
	struct CANRCVTIMBUF *p;
	if (canbuftimIDXi == canbuftimIDXm) return 0;	// Return showing no new data

	p = &cantimbuf[canbuftimIDXm];	// Get return pointer value	

	/* Advance index ('m' = mainline) */
	canbuftimIDXm += 1; if (canbuftimIDXm >= CANRCVTIMBUFSIZE) canbuftimIDXm = 0;// Reset to beginning
	return p;			// Return pointer to buffer
}
/*---------------------------------------------------------------------------------------------
 * static void loadmbx(struct CANRCVBUF *px)
 * @brief	: Load mailbox
 * @param	: pointer to struct with msg: id & data
 * @return	: 0 = no input buffers remain to be loaded into mailboxes; not zero = more to do
 ----------------------------------------------------------------------------------------------*/
int unsigned can_debugP = 0;	// Msgs added to buffer
int unsigned can_debugM = 0;	// Msgs removed from buffer

static void loadmbx(struct CANRCVBUF *px)
{
	/* Load the mailbox with the message. */
	CAN_TIxR (CAN1, CAN_MBOX0) = px->id;		// CAN_TIxR  mailbox   identifier register p 659
	CAN_TDTxR(CAN1, CAN_MBOX0) = px->dlc;		// CAN_TDTxR: mailbox  time & length p 660
	CAN_TDLxR(CAN1, CAN_MBOX0) = px->cd.ui[0];	// CAN_TDLxRL mailbox  data low  register p 661 
	CAN_TDHxR(CAN1, CAN_MBOX0) = px->cd.ui[1];	// CAN_TDLxRH mailbox  data low  register p 661 

	/* Request a transmission */
	CAN_TIxR (CAN1, CAN_MBOX0) |= 0x1;	// Set TXRQ bit in ID register of mailbox p 669

	return;
}
/******************************************************************************
 * int can_msg_txchkbuff(void);
 * @brief	: Check buffer status for xmit buffer
 * @return	: 0 = or negative, buffer full; (CANXMTBUFSIZE - 1) = slots available
 ******************************************************************************/
int can_msg_txchkbuff(void)
{
	int temp;
	/* Compute remaining msgs in buffer */
	temp = (canxmtIDXi - canxmtIDXm);
	if (temp < 0) temp += CANXMTBUFSIZE;
	
	return	temp-1; // Return number msg remaining buffered
}
/******************************************************************************
 * int can_msg_put(struct CANRCVBUF *px);
 * @brief	: Load msg into buffer and start mailbox 0
 * @param	: pointer to struct with msg: id, dlc, data
 * @return	: number msg remaining buffered
 * ==> NOTE: dlc is expected to be set when this routine is entered.
 ******************************************************************************/
int can_msg_put(struct CANRCVBUF *px)
{
	/* Copy msg into buffer */
	canxmtbuf[canxmtIDXm] = *px;	// Add msg to buffer

	/* Advance buffer index */
	canxmtIDXm += 1; if (canxmtIDXm >= CANXMTBUFSIZE)    canxmtIDXm = 0;

can_debugP += 1;	// Msgs added to buffer

	/* Load mailbox if it is empty and the interrupt request bit has been turned off p 655 */
	// Mailbox needs to be loaded if TME0 and RQCP0 bits are mailbox empty and interrupt request serviced 
	if ( (CAN_TSR(CAN1) & (CAN_TSR_TME0 | CAN_TSR_RQCP0)) == CAN_TSR_TME0)  
	{ // Here yes.  Mailbox is empty.  Load mailbox, set TXRQ	
		loadmbx(&canxmtbuf[canxmtIDXi]);
	}
	
	return	can_msg_txchkbuff(); // Return number msg remaining buffered
}

/******************************************************************************
 * void can_msg_rcv_compress(struct CANRCVBUF *px);
 * @brief	: Set .dlc with count to eliminate high order bytes equal zero
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_rcv_compress(struct CANRCVBUF *px)
{
	int temp;

	/* Set length to send only the signficant bytes (DLC p 660) */
	px->dlc &= ~0xf;	// Clear DLC field
	
	/* Find most significant byte */
	for (temp = 7; temp >= 0; temp--)
	{
		if (px->cd.uc[temp] != 0) break;
	}
	if ( temp >= 0)
	{ // Here, temp = 0->7 , so send 1 -> 8 bytes
		px->dlc |= ((temp+1) & 0xf);   // Number of bytes
	}
	else
	{ // All bytes zero.  Send as RTR
		px->id |= 0x2; // RTR
	}
	return;
}
/******************************************************************************
 * void can_msg_rcv_expand(struct CANRCVBUF *px);
 * @brief	: Fill bytes not received with zeros
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_rcv_expand(struct CANRCVBUF *px)
{
	int i;
	int temp = (px->dlc & 0xf);	// Get byte count in received msg
	if (temp > 8) temp = 8;		// JIC the number exceeds the array
	if (temp <= 0) { px->cd.ull = 0; return; }
	
	/* Clear bytes not received. */
	for (i = 7; i >= temp; i--)	// Work from hi-ord byte down
		px->cd.uc[i] = 0;	// Set unsent byte to zero

	return;
}
/******************************************************************************
 * void can_msg_setsize(struct CANRCVBUF *px, int count);
 * @brief	: Set .dlc with byte count to send
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_setsize(struct CANRCVBUF *px, int count)
{
	px->dlc &= ~0xf;	// Clear dlc field
	px->dlc |= count & 0xf;	// Add new count
	return;
}
/*#######################################################################################
 * ISR routines for CAN:There four interrupt vectors, p 647 fig 235
 *####################################################################################### */

/*#######################################################################################
 * ISR routine for CAN: "Transmit Interrupt" <-- (RQCP0 | RQCP1 | RQCP2)
 *####################################################################################### */
/* p 647 The transmit interrupt can be generated by the following events:
–   Transmit mailbox 0 becomes empty, RQCP0 bit in the CAN_TSR register set.
–   Transmit mailbox 1 becomes empty, RQCP1 bit in the CAN_TSR register set.
–   Transmit mailbox 2 becomes empty, RQCP2 bit in the CAN_TSR register set. */
u32 can_terr = 0; 	// Count of TERR flags

void USB_HP_CAN_TX_IRQHandler(void)
{
	int temp;
//while (1==1);
	CAN_TSR(CAN1) = (CAN_TSR_RQCP1 | CAN_TSR_RQCP2);	// JIC mbx 2 & 3 have a flag

	if ((CAN_TSR(CAN1) & CAN_TSR_RQCP0) != 0) // Is mailbox0 RQCP0 (request complete) ON?
	{ // Here, yes.  
		/* Primitive error handling */
		if ((CAN_TSR(CAN1) & 0x8) != 0) can_terr += 1; // Count errors

		/* Clear RQCPx  */
		CAN_TSR(CAN1) = CAN_TSR_RQCP0;	// Clear RQPx (which clears TERRx, ALSTx, TXOKx)
		
		/* Advance buffer index */
		canxmtIDXi += 1; if (canxmtIDXi >= CANXMTBUFSIZE)    canxmtIDXi = 0;
can_debugM += 1;	// Buffer msgs loaded to mbx

		/* Load more data, if buffer not empty */
		if (canxmtIDXm != canxmtIDXi)	// Are we caught up?
		{ // Here, no.  Msgs remain in buffer
			loadmbx(&canxmtbuf[canxmtIDXi]);	// Load mailbox, set TXRQ
		}
	}
	temp = CAN_TSR(CAN1);	// JIC Prevent tail-chaining

	return;
}
/*#######################################################################################
 * ISR routine for CAN: "FIFO 0 interrupt" -- FMP0, FULL0, FOVR0
 *####################################################################################### */
/* p 647 The FIFO 0 interrupt can be generated by the following three events (only X = enabled):
X   Reception of a new message, FMP0 bits in the CAN_RF0R register are not ‘00’.
–   FIFO0 full condition, FULL0 bit in the CAN_RF0R register set.
–   FIFO0 overrun condition, FOVR0 bit in the CAN_RF0R register set. */
void USB_LP_CAN_RX0_IRQHandler(void)
{
	int temp;
//while(1==1);
	/* Save message in a circular buffer */
	canbuf[canbufIDXi].U.ull    = strAlltime.SYS.ull; // Add time stamp
	canbuf[canbufIDXi].id       = CAN_RI0R(CAN1);	// ID, RTR, IDE
	canbuf[canbufIDXi].dlc	    = CAN_RDT0R(CAN1);	// time, data length CAN_RDTxR p 663
	canbuf[canbufIDXi].cd.ui[0] = CAN_RDL0R(CAN1);	// Data (32b) High, Data (32b) Low
	canbuf[canbufIDXi].cd.ui[1] = CAN_RDH0R(CAN1);	// Data (32b) High, Data (32b) Low

	/* Release FIFO 0 */	
	CAN_RF0R(CAN1) |= CAN_RF0R_RFOM0;	// Write bit 5

	/* Advance index in circular buffer */
	canbufIDXi += 1; if (canbufIDXi >= CANRCVBUFSIZE) canbufIDXi = 0;// Reset to beginning
		
	/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
	NVICISPR(NVIC_I2C1_ER_IRQ);	// Set pending (low priority) interrupt for TIM4 ('../lib/libusartstm32/nvicdirect.h')

	temp = CAN_RF0R(CAN1);	// Read register to avoid tail-chaining

	return;
}
/*#######################################################################################
 * ISR routine for CAN: "FIFO 1 interrupt" -- FMP1, FULL1, FOVR1
 *####################################################################################### */

/* p 647,8 The FIFO 1 interrupt can be generated by the following three events (only X = enabled):
X   Reception of a new message, FMP1 bits in the CAN_RF1R register are not ‘00’.
–   FIFO1 full condition, FULL1 bit in the CAN_RF1R register set.
–   FIFO1 overrun condition, FOVR1 bit in the CAN_RF1R register set. */
void CAN_RX1_Handler(void)
{ // Here, FMP1 bits are not 00.
	int temp;
//while(1==1);
	/* Save time & message in a circular buffer */
	cantimbuf[canbuftimIDXi].U.ull    = SYSTICK_getcount32();// Get current tick time
	cantimbuf[canbuftimIDXi].R.id     = CAN_RI1R(CAN1);	// ID, RTR, IDE
	cantimbuf[canbuftimIDXi].R.dlc	  = CAN_RDT1R(CAN1);	// time, data length CAN_RDTxR p 663
	cantimbuf[canbuftimIDXi].R.cd.ui[0] = CAN_RDL1R(CAN1);	// Data (32b) Low
	cantimbuf[canbuftimIDXi].R.cd.ui[1] = CAN_RDH1R(CAN1);	// Data (32b) High

	/* Release FIFO 1 */	
	CAN_RF1R(CAN1) |= CAN_RF1R_RFOM1;		// Write bit 5 to release FIFO msg

	/* Advance index in circular buffer */
	canbuftimIDXi += 1; if (canbuftimIDXi >= CANRCVTIMBUFSIZE) canbuftimIDXi = 0;// Reset to beginning

	/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
	NVICISPR(NVIC_I2C1_EV_IRQ);	// Set pending (low priority) interrupt for TIM1 ('../lib/libusartstm32/nvicdirect.h')

	temp = CAN_RF1R(CAN1);	// Read register to avoid tail-chaining

	return;
}
/*#######################################################################################
 * ISR routine for CAN: "Status Change Error" -- EWGF, EPVF, BOFF, WKUI, SLAKI
 *####################################################################################### */
/* p 648 The error and status change interrupt can be generated by the following events:
–   Error condition, for more details on error conditions please refer to the CAN Error
    Status register (CAN_ESR).
–   Wakeup condition, SOF monitored on the CAN Rx signal.
–   Entry into Sleep mode. */
void CAN_SCE_Handler(void)
{
//while(1==1);
	return;
}
/*#######################################################################################
 * ISR routine for FIFO 1 (higher than other low priority level)
 *####################################################################################### */
void I2C1_EV_IRQHandler(void)
{
//while(1==1);
/* This interrupt is caused by the CAN FIFO 1 (time sync message) interrupt for further processing at a low interrupt priority */
	/* Call other routines if an address is set up */
	if (highpriority_ptr != 0)	// Having no address for the following is bad.
		(*highpriority_ptr)();	// Go do something
	return;
}
/*#######################################################################################
 * ISR routine for FIFO 0 low priority level
 *####################################################################################### */
void I2C1_ER_IRQHandler(void)
{
//while(1==1);
/* This interrupt is caused by the CAN FIFO 0 (time sync message) interrupt for further processing at a low interrupt priority */
	/* Call other routines if an address is set up */
	if (lowpriority_ptr != 0)	// Having no address for the following is bad.
		(*lowpriority_ptr)();	// Go do something
	return;
}



