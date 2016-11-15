/******************************************************************************
* File Name          : canwinch_pod_common_systick2048_printerr.c
* Date First Issued  : 05/12/2015
* Board              : 
* Description        : print error counts for 'canwinch_pod_common_systick2048'
*******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "canwinch_pod_common_systick2048_printerr.h"
#include "libmiscstm32/printf.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"

static char* p1 = "Count: Buffer overflow when adding a msg";
static char* p2 = "Count  TERR flags";
static char* p3 = "Count: number of times msgs failed due to too many TXERR";
static char* p4 = "Count: arbitration failure total";
static char* p5 = "Count: arbitration failure when NART is on";
static char* p5a= "Count: TX spurious interrupt";
static char* p5b= "Count: TX TSR had no: TERR, ALST0, TXOK bits on";
static char* p5c= "Count: Instances that pfor was adjusted in TX interrupt";
static char* p5d= "Count: Instances that pxprv was adjusted in 'for' loop";
static char* p6 = "Count  FIFO 0 overruns";
static char* p7 = "Count  FIFO 1 overruns";
static char* p8 = "Count: (RQCP1 | RQCP2) unexpectedly ON";
static char* p9 = "Count: 'systickphasing' unexpected 'default' in switch";
static char* pA = "Count: 'systickphasing',lost sync msgs counter";

/******************************************************************************
 * void canwinch_pod_common_systick2048_printerr_header(void);
 * @brief	: print header/description for all the error counts
 ******************************************************************************/
void canwinch_pod_common_systick2048_printerr_header(void)
{
	printf("\n\r COUNTERS in 'canwinch_pod_common_systick2048.c'\n\r");
	printf("1  %s\n\r",p1);
	printf("2  %s\n\r",p2);
	printf("3  %s\n\r",p3);
	printf("4  %s\n\r",p4);
	printf("5  %s\n\r",p5);
	printf("6  %s\n\r",p5a);
	printf("7  %s\n\r",p5b);
	printf("8  %s\n\r",p5c);
	printf("9  %s\n\r",p5d);
	printf("10 %s\n\r",p6);
	printf("11 %s\n\r",p7);
	printf("12 %s\n\r",p8);
	printf("13 %s\n\r",p9);
	printf("14 %s\n\r",pA);USART1_txint_send();
	return;
}

/******************************************************************************
 * void canwinch_pod_common_systick2048_printerr(struct CANWINCHPODCOMMONERRORS* p);
 * @brief	: print all the error counts
 * @param	: p = pointer to struct with error counts
 ******************************************************************************/
void canwinch_pod_common_systick2048_printerr(struct CANWINCHPODCOMMONERRORS* p)
{
	printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d\n\r",
		p->can_msgovrflow,	/* Count: Buffer overflow when adding a msg */
		p->can_txerr,		/* Count  TERR flags */
		p->can_tx_bombed,	/* Count: number of times msgs failed due to too many TXERR */
		p->can_tx_alst0_err,	/* Count: arbitration failure total */
		p->can_tx_alst0_nart_err,/* Count: arbitration failure when NART is on */
		p->can_spurious_int,	/* Count: TSR had no RQCPx bits on (spurious interrupt) */
		p->can_no_flagged,	/* Count: TSR had no: TERR, ALST0, TXOK bits on */
		p->can_pfor_bk_one,	/* Count: Instances that pfor was adjusted in TX interrupt */
		p->can_pxprv_fwd_one,	/* Count: Instances that pxprv was adjusted in 'for' loop */
		p->can_rx0err,		/* Count  FIFO 0 overruns */
		p->can_rx1err,		/* Count  FIFO 1 overruns */
		p->can_cp1cp2,		/* Count: (RQCP1 | RQCP2) unexpectedly ON */
		p->error_fifo1ctr,	/* Count: 'systickphasing' unexpected 'default' in switch */
		p->nosyncmsgctr);	/* Count: 'systickphasing',lost sync msgs counter */
	USART1_txint_send();
	return;		
}

