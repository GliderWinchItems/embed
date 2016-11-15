/******************************************************************************
* File Name          : canwinch_setup_F103_Olimex.c
* Date First Issued  : 08-12-2015
* Board              : F103
* Description        : Setup initializtion of CAN1 for winch, F103 Olimex board
*******************************************************************************/
#include "can_driver.h"
#include "can_driver_filter.h"
#include "can_driver_port.h"
#include "db/gen_db.h"
#include "common_can.h"
#include "libmiscstm32/clockspecifysetup.h"
#include "libusartstm32/nvicdirect.h"

/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_8X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSI/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
/* Parameters for setting up CAN, based on 64MHz clock */
static const struct CAN_PARAMS2 params = { \
CANWINCH_BAUDRATE,	// CAN baudrate for winch (ref: 'svn_common/trunk/common_can.h')
1,		/* CAN number (1 = CAN1, 2 = CAN2) */
1,		/* port: port: 0 = PA 11|12; 1 = PB; 2 = PD 0|1;  (port > 2 is not valid)  */
0,		/* silm: CAN_BTR[31] Silent mode (0 or non-zero) */
0,		/* lbkm: CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback) */
1,		/* sjw:  CAN_BTR[24:25] Resynchronization jump width */
2,		/* tbs2: CAN_BTR[22:20] Time segment 2 (e.g. 5) */
13,		/* tbs1: CAN_BTR[19:16] Time segment 1 (e.g. 12) */
1,		/* dbf:  CAN_MCR[16] Debug Freeze; 0 = normal; non-zero = */
0,		/* ttcm: CAN_MCR[7] Time triggered communication mode */
1,		/* abom: CAN_MCR[6] Automatic bus-off management */
0,		/* awum: CAN_MCR[5] Auto WakeUp Mode */
0,		/* nart: CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = transmit once) */
{NVIC_USB_HP_CAN_TX_IRQ, NVIC_USB_HP_CAN_TX_IRQ_PRIORITY},	/* TX  interrupt (number & interrupt priority) */
{NVIC_USB_LP_CAN_RX0_IRQ, NVIC_USB_LP_CAN_RX0_IRQ_PRIORITY},	/* RX0 interrupt (number & interrupt priority) */
{NVIC_CAN_RX1_IRQ, NVIC_CAN_RX1_IRQ_PRIORITY},			/* RX1 interrupt (number & interrupt priority) */
};
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
/* Hardware filters */

// Designate a high priority block of msgs that go to FIFO1
//     One 32b ID plus mask                   CAN ID               MASK              FIFO/SCALE/MODE       BANK NUM  ODD/EVEN
static const struct CANFILTERPARAM fb0 = {CANWINCH_FIFO1_ID1, CANWINCH_FIFO1_MSK1, CANFILT_FIFO1_32b_MSK_ID,  0, CANFILT_EVEN};

// Designate all CAN IDs not snagged by 'fb0' and 'fb1' filters to go to FIFO 0
//                                            CAN ID               MASK              FIFO/SCALE/MODE       BANK NUM  ODD/EVEN
static const struct CANFILTERPARAM fb2 = {CANWINCH_FIFO0_ID1, CANWINCH_FIFO0_MSK1, CANFILT_FIFO0_32b_MSK_ID,  2, CANFILT_EVEN};

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/******************************************************************************
 * struct CLOCKS* canwinch_setup_F103_pod_clocks(void);
 * @brief 	: Supply 'clock' params that match CAN setup params
 * @return	:  pointer to struct
*******************************************************************************/
struct CLOCKS* canwinch_setup_F103_Olimex_clocks(void)
{
	return (struct CLOCKS*)&clocks;
}
/******************************************************************************
 * static int canwinch_setup_F103_filter(u32 canid);
 * @brief	: Setup the filters 
 * @param	: canid = CAN id used for reset msg to this unit
 * @return	: 0 = success; -1 for error
*******************************************************************************/
static int canwinch_setup_F103_filter(u32 canid)
{
	/* Turn off active bit for all filters (including those for CAN2!). */
	can_driver_filter_deactivate_all();

	/* Insert new filters and activate. */
	int ret;
	ret  = can_driver_filter_insert((struct CANFILTERPARAM*)&fb0); // Hi-priority group
	ret |= can_driver_filter_add_two_32b_id(canid, CANID_DUMMY, 1, 1); // Unit ID
	ret |= can_driver_filter_insert((struct CANFILTERPARAM*)&fb2); // FIFO0 gets all others
	return ret;
}
/******************************************************************************
 * struct CAN_CTLBLOCK* canwinch_setup_F103_pod(const struct CAN_INIT* pinit, u32 canid);
 * @brief 	: Provide CAN1 initialization parameters: winch app, F103, pod board
 * @param	: pinit = pointer to msg buffer counts for this CAN
 * @param	: canid = CAN id used for reset msg to this unit
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
struct CAN_CTLBLOCK* canwinch_setup_F103_Olimex(const struct CAN_INIT* pinit, u32 canid)
{
	//                                parameters, TX buff, RX0 buff, RX1 buff
	struct CAN_CTLBLOCK* pctl;
	pctl = can_driver_init( (struct CAN_PARAMS2*)&params, pinit);

	/* Early CAN init failure returns NULL. */
	if (pctl == NULL) return NULL;

	/* Check for more init problems. */
	if (pctl->ret != 0) return pctl;

	/* Here, CAN init OK.  Setup hardware msg filtering */
	int ret = canwinch_setup_F103_filter(canid);
	if (ret != 0) pctl->ret = (ret - 7); // Set ret to -8 or -9

	return pctl;
	
}


