/******************************************************************************
* File Name          : CAN_test_msgs.c
* Date First Issued  : 11/29/2013
* Board              : Discovery F4
* Description        : PC<->gateway--generate CAN format msgs for testing
*******************************************************************************/
/*

*/
#include "common_can.h"
#include "USB_PC_gateway.h"
#include "PC_gateway_comm.h"
#include <stdio.h>
#include <string.h>

/* The following sets the number of test messages per second. */
#define CANTESTINC_PC  (186000000/75) // system clock ticks per sec/ number msgs per sec
#define CANTESTINC_CAN (186000000/4)

static unsigned int t_dly_pc;
static unsigned int t_dly_can;
/* **************************************************************************************
 * void CAN_test_msg_init(void);
 * @brief	: Initialize time for generating time msgs
 * ************************************************************************************** */
void CAN_test_msg_init(void)
{
	t_dly_pc  = *(volatile unsigned int *)0xE0001004 + CANTESTINC_PC;
	t_dly_can = *(volatile unsigned int *)0xE0001004 + CANTESTINC_CAN;
	return;
}

/* **************************************************************************************
 * struct CANRCVBUF* CAN_test_msg_PC(void);
 * @brief	: Generate a CAN test msg that goes to the PC
 * @return	:  0 = no msg 
 *  		:  not zero = pointer to buffer with msg
 * ************************************************************************************** */
static unsigned int t_dly_pc;
#define CANTESTNUMBER_PC	5
static int idx = CANTESTNUMBER_PC;
static const struct CANRCVBUF canPC[CANTESTNUMBER_PC] = \
{
{0x66400000, 	/* 11 bit id */
 0x00000008, 	/* 8 data bytes */
{0xfedcba9876543210}, 	/* data bytes */
},
{
 0x66600000,	/* 11 bit id */
 0x00000004,	/* 4 data byte */
{0xaabbccdd},	/* data byte */
},
{
 0x66800000,	/* 11 bit id */
 0x00000000,	/* zero byte ct */
{0x00000000},	/* null data */
},
{
 0x9999999c,	/* 29 bit id */
 0x00000008,	/* 8 data byte ct */
{0xeeccaa8866442200}, 	/* data bytes */
},
{
 0x66c00002,	/* 11 bit id, RTR */
 0x00000000,	/* zero byte ct */
{0x00000000},	/* null data */
},
};



struct CANRCVBUF* CAN_test_msg_PC(void)
{
	/* Is it time to send a new round of msgs? */
	if (((int)(*(volatile unsigned int *)0xE0001004 - t_dly_pc)) > 0)
	{ // Here, yes.
		t_dly_pc += CANTESTINC_PC;	// Compute next time
		idx = 0;
	}
	if (idx >= CANTESTNUMBER_PC) return 0;
	return (struct CANRCVBUF*)&canPC[idx++];	
}


/* **************************************************************************************
 * struct CANRCVBUF* CAN_test_msg_CAN(void);
 * @brief	: Generate a CAN test msg that goes to the CAN bus
 * @return	:  0 = no msg 
 *  		:  not zero = pointer to buffer with msg
 * ************************************************************************************** */
static unsigned int t_dly_can;
#define CANTESTNUMBER_CAN	4
static int idx1 = CANTESTNUMBER_CAN;
static const struct CANRCVBUF canCAN[CANTESTNUMBER_CAN] = \
{
{0x66400000, 	/* 11 bit id */
 0x00000008, 	/* 8 data bytes */
{0xfedcba9876543210}, 	/* data bytes */
},
{
 0x66500000,	/* 11 bit id */
 0x00000004,	/* 4 data byte */
{0xaabbccdd},	/* data byte */
},
{
 0x66600000,	/* 11 bit id */
 0x00000000,	/* zero byte ct */
{0x00000000},	/* null data */
},
{
 0x9999999c,	/* 29 bit id */
 0x00000008,	/* 8 data byte ct */
{0xeeccaa8866442200}, 	/* data bytes */
}
};

struct CANRCVBUF* CAN_test_msg_CAN(void)
{

	/* Is it time to send a msg? */
	if (((int)(*(volatile unsigned int *)0xE0001004 - t_dly_can)) > 0)
	{ // Here, yes.
		t_dly_can += CANTESTINC_CAN;	// Compute next time
		idx1+= 1; if (idx1 >= CANTESTNUMBER_CAN) idx1 = 0;
		return (struct CANRCVBUF*)&canCAN[idx1];
	}
	return 0;	
}
