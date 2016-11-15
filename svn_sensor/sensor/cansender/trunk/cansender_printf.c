/******************************************************************************
* File Name          : cansender_printf.c
* Date First Issued  : 09/08/2016
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "cansender_printf.h"
#include "libusartstm32/usartallproto.h"

/* **************************************************************************************
 * void cansender_printf(struct CANSENDERLC* psend);
 * @brief	: Print the values
 * @param	: psend = pointer to struct with the values 
 * ************************************************************************************** */
void cansender_printf(struct CANSENDERLC* psend)
{
int i = 0;
printf("CANSENDER: values: pointer = %08X\n\r",(int)psend);

printf("%2d	%d	%s\n\r",i + 0, (unsigned int)psend->size,     "  0 Number of elements in the following list");
printf("%2d	%d	%s\n\r",i + 1, (unsigned int)psend->crc,      "  1 Tension_1: CRC for tension list");
printf("%2d	%d	%s\n\r",i + 2, (unsigned int)psend->version,  "  2 Version number");
printf("%2d	%d	%s\n\r",i + 3, (unsigned int)psend->hbct,     "  3 Heartbeat count of time (ms) between msgs");
printf("%2d	%08X	%s\n\r",i + 4, (unsigned int)psend->cid_hb,   "  4 CANID: Hearbeat msg sends running count");
printf("%2d	%08X	%s\n\r",i + 5, (unsigned int)psend->cid_poll, "  5 CANID: Poll this cansender");
printf("%2d	%08X	%s\n\r",i + 6, (unsigned int)psend->cid_pollr,"  6 CANID: Response to POLL");

	USART1_txint_send(); 
	return;
}
