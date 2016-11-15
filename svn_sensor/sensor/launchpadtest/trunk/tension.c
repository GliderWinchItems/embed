/******************************************************************************
* File Name          : tension.c
* Date First Issued  : 08/01/2015
* Board              : POD board
* Description        : Test launchpad fp format
*******************************************************************************/
/* 
Strip & Hack of ../tension/trunk/ routines for testing launchpad fp format
*/

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "PODpinconfig.h"
#include "pinconfig_all.h"

#include "usartallproto.h"
#include "clockspecifysetup.h"

/* Subroutine prototypes */
void fpformat(char* p, double d);
void fmtprint(int i, float f,char* p);

/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_8X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSI/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
/* --------------------- Begin setting things up -------------------------------------------------- */ 
	// Start system clocks using parameters matching CAN setup parameters for POD board
	clockspecifysetup((struct CLOCKS*)&clocks);
/* ---------------------- Set up pins ------------------------------------------------------------- */
	PODgpiopins_Config();
	PODgpiopins_default();	// Set gpio port register bits for low power

	/* Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen) */
	MAX3232SW_on;	

	/* Allow some time for the RS232 power-on to stabilize. */
	volatile int t = 0;
	while (t++ < 500000);

/* --------------------- Initialize usart --------------------------------------------------------- */
/*	USARTx_rxinttxint_init(...,...,...,...);
	Receive:  rxint	rx into line buffers
	Transmit: txint	tx with line buffers
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx line buffer size, (long enough for the longest line (is best))
		number of rx line buffers, (must be > 1)
		tx line buffer size, (long enough for the longest line (is best))
		number of tx line buffers, (must be > 1)
*/

	USART1_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rLAUNCHPADTEST: 08-02-2015 v6\n\r");USART1_txint_send();

	/* Display bus rates so's to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);USART1_txint_send();

char ufp[512];
char sfp[64];

double fval = 3.14159;

	fpformat(sfp, fval);
	printf("\n\rfpformt: %s  launchpad fmt: %f\n\r",sfp,fval);
	USART1_txint_send();


	sprintf(ufp, "\n\rfpformt: %s  launchpad fmt: %f\n\r",sfp,fval);
	USART1_txint_puts(ufp);
	USART1_txint_send();

	fval = 111;
	fpformat(sfp, fval);
	sprintf(ufp, "\n\rfpformt: %s  launchpad fmt: %f\n\r",sfp,fval);
	USART1_txint_puts(ufp);
	USART1_txint_send();

	printf("\n\rDONE\n\r");USART1_txint_send();
	while(1);	// Hang 
	return 0;	
}

void fmtprint(int i, float f, char* p)
{
	char w[64];
	double d = f;
	fpformat(w,d);
	printf("%02d\t%s %s\n\r",i,w,p);
	return;
}
void fpformat(char* p, double d)
{
	int i = d;	// Get whole part
	int j = i;
	sprintf(p, "%5i.",i);	// Convert whole part
	if (j < 0) j = -j;  if (d < 0) d = -d;
	double f = (d * 1000) - (j * 1000); // f = fractional part
	sprintf((p+6),"%03i",(int)f);
	return;
}

