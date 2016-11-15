/******************************************************************************
* File Name          : canwinch_pod.h
* Date First Issued  : 10/02/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : CAN routines for winch instrumentation--POD board
*******************************************************************************/
/* 
See p610 of Ref Manual 
*/

#ifndef __CANWINCH
#define __CANWINCH

#include "libopencm3/cm3/common.h"	// libopenstm32 (u32,...u8)
#include "common_can.h"


/* struct CAN_PARAMS holds the values used to setup the CAN registers during 'init' */
// baud rate CAN_BTR[9:0]: brp = (pclk1_freq/(1 + TBS1 + TBS2)) / baudrate;

/* Default value string, base on 36 MHz AHB */
#define CAN_PARAMS_DEFAULT {500000,2,0,0,4,5,12,1,0,1,0,0}

struct CAN_PARAMS
{
	u32	baudrate;	// E.g. 500000 = 500K bps
	u8	port;		// port: 0 = PA 11|12; 2 = PB; 3 = PD 0|1;  (1 = not valid; >3 not valid) 
	u8	silm;		// CAN_BTR[31] Silent mode (0 or non-zero)
	u8	lbkm;		// CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback)
	u8	sjw;		// CAN_BTR[24:25] Resynchronization jump width
	u8	tbs2;		// CAN_BTR[22:20] Time segment 2 (e.g. 5)
	u8	tbs1;		// CAN_BTR[19:16] Time segment 1 (e.g. 12)
	u8	dbf;		// CAN_MCR[16] Debug Freeze; 0 = normal; non-zero =
	u8	ttcm;		// CAN_MCR[7] Time triggered communication mode
	u8	abom;		// CAN_MCR[6] Automatic bus-off management
	u8	awum;		// CAN_MCR[5] Auto WakeUp Mode
	u8	nart;		// CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = auto retry)
};

/******************************************************************************/
int can_init_pod(struct CAN_PARAMS *p);
/* @brief 	: Setup CAN pins and hardware
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @return	:  n = remaining counts; 0 = enter init mode timedout;-1 = exit init mode timeout; -2 = bad port
*******************************************************************************/
void can_filter_unitid(u32 myunitid);
/* @brief 	: Setup filter bank for Checksum check & Program loading
 * @param	: "my" can id
 * @return	: Nothing for now.
*******************************************************************************/
struct CANRCVSTAMPEDBUF * canrcv_get(void);
/* @brief	: Get pointer to non-high priority CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
*******************************************************************************/
struct CANRCVTIMBUF * canrcvtim_get(void);
/* @brief	: Get pointer to high priority CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
int can_msg_put(struct CANRCVBUF *px);
/* @brief	: Load msg into buffer and start mailbox 0
 * @param	: pointer to struct with msg: id & data
 * @return	: number msg remaining buffered
 ******************************************************************************/
void can_nxp_setRS(int rs);
/* @brief 	: Set RS input to NXP CAN driver (TJA1051) (on some PODs)
 * @param	: rs: 0 = NORMAL mode; not-zero = SILENT mode 
 * @return	: Nothing for now.
*******************************************************************************/
void can_msg_rcv_compress(struct CANRCVBUF *px);
/* @brief	: Set .dlc with count to eliminate high order bytes equal zero
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_rcv_expand(struct CANRCVBUF *px);
/* @brief	: Fill bytes not received with zeros
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_setsize(struct CANRCVBUF *px, int count);
/* @brief	: Set .dlc with byte count to send
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
int can_msg_txchkbuff(void);
/* @brief	: Check buffer status for xmit buffer
 * @return	: 0 = buffer full; (CANXMTBUFSIZE - 1) = slots available
 ******************************************************************************/
u16 can_filtermask16_add(u32 m);
/* @brief 	: Add a 16 bit id & mask to the filter banks
 * @param	: mask and ID: e.g. (CAN_UNITID_MASK | (CAN_TIMESYNC3 >> 16)
 * @return	: Filter number; negative for you have filled it up!
 NOTE: These mask|ID's assign to the FIFO 0 (default)
*******************************************************************************/

/* Pointers to functions to be executed under a low priority interrupt, forced by a CAN interrupt */
extern void	(*highpriority_ptr)(void);	// Address of function to call TIM7 forced interrupt
extern void	(*lowpriority_ptr)(void);	// Address of function to call TIM5 forced interrupt
extern void 	(*timing_sync_ptr)(void);	// Address of function to call upon FIFO 1 interrupt

#endif 

