/******************************************************************************
* File Name          : can_filter_print.c
* Date First Issued  : 08/01/2016
* Board              : F103 or F4
* Description        : List CAN filter register
*******************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include "can_filter_print.h"
#include "libusartstm32/usartallproto.h"
#include "can_driver_filter.h"



#include "../../../../svn_pod/sw_stm32/trunk/lib/libopenstm32/common.h"
#include "../../../../svn_pod/sw_stm32/trunk/lib/libopenstm32/can.h"


/* **************************************************************************************
 * void can_filter_print(unsigned int size);
 * @param	: size = number of filter banks on this part
 * @brief	: Print the values of the CAN filter
 * ************************************************************************************** */
void can_filter_print(unsigned int size)
{
	unsigned int i;
	unsigned int x1,x2;
	int ret;
	
	printf("0x%08X CAN filter master register (CAN_FMR)\n\r",(unsigned int)CAN_FMR(CAN1));
	printf("0x%08X CAN filter mode register (CAN_FM1R)\n\r",(unsigned int)CAN_FM1R(CAN1));
	printf("0x%08X CAN filter scale register (CAN_FS1R)\n\r", (unsigned int)CAN_FS1R(CAN1));
	printf("0x%08X CAN filter FIFO assignment register (CAN_FFA1R)\n\r", (unsigned int)CAN_FFA1R(CAN1));
	printf("0x%08X CAN filter activation register (CAN_FA1R)\n\r", 	(unsigned int)CAN_FA1R(CAN1));

	printf("Filter bank i register x (CAN_FiRx) (i=0..27, x=1, 2)\n\r");
	for (i = 0; i < size; i++)
	{
		x1 = CAN_FiR1(CAN1, i);
		x2 = CAN_FiR2(CAN1, i);
		printf( "%2d 0x%08X 0x%08X\n\r",i,x1,x2);
	}
	ret = can_driver_filter_getbanknum(0); // For (CAN1) get filter bank number and odd/even
	printf("Next available bank number: %d  Odd/even: %d\n\r", (ret >> 8), (ret & 1) );

	return;
}
