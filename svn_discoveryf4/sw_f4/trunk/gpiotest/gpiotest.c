/******************************************************************************
* File Name          : gpiotest
* Date First Issued  : 02/26/2012
* Board              : STM32 Discovery board
* Description        : Test program for gpio pins
*******************************************************************************/


/* 
This routine first blinks the four on-board LEDs, then waits for "commands" from
the PC.
Command: 'Xnb'<CR>
Where: X is port designator = a, b, c, d, e
n is bit number of part = 0 - 15
b is 0 or 1.  zero for off (low), one is for on (high)

This routine will then set/reset the bit.

Open minicom on the PC with 115200 baud and 8N1.

*/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libmiscstm32f4/systick1.h"
#include "libmiscstm32f4/clockspecifysetup.h"

#include "libdiscoveryf4/DISCpinconfig.h"	// Pin configuration for STM32 Discovery board

/* Put this variable in static memory to test that the variable are being initialized correctly */
double dpi = 3.14159265358979323;


/* The following values provide --
External 8 MHz xtal
sysclk  =  168 MHz
PLL48CK =   48 MHz
PLLCLK  =  168 MHz
AHB     =  168 MHz
APB1    =   42 MHz
APB2    =   84 MHz
*/

const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc: 0 = internal 16 MHz rc; 1 = external xtal controlled; 2 = ext input; 3 ext remapped xtal; 4 ext input */ \
1,			/* Source for main PLL & audio PLLI2S: 0 = HSI, 1 = HSE selected */ \
APBX_4,			/* APB1 clock = SYSCLK divided by 0,2,4,8,16; freq <= 42 MHz */ \
APBX_2,			/* APB2 prescalar code = SYSCLK divided by 0,2,4,8,16; freq <= 84 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2 and HCLK) */ \
8000000,		/* External Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
7,			/* Q (PLL) divider: USB OTG FS, SDIO, random number gen. USB OTG FS clock freq = VCO freq / PLLQ with 2 ≤ PLLQ ≤ 15 */ \
PLLP_2,			/* P Main PLL divider: PLL output clock frequency = VCO frequency / PLLP with PLLP = 2, 4, 6, or 8 */ \
84,			/* N Main PLL multiplier: VCO output frequency = VCO input frequency × PLLN with 64 ≤ PLLN ≤ 432	 */ \
2			/* M VCO input frequency = PLL input clock frequency / PLLM with 2 ≤ PLLM ≤ 63 */
};


/* points for malloc test */
char * pmal1;
char * pmal2;
char * pmal3;
char * pmal4;

/* sprintf buffers */
#define PRNTSIZE	256
	char ww[PRNTSIZE];
	char xx[PRNTSIZE];
	char yy[PRNTSIZE];
	char zz[PRNTSIZE];

/* LED pin configuration for PD 12-15) */
//  mode gen output, pushpull, 25 MHz, no pull up/down, no alternative function */
const struct PINCONFIG ppoutput = {1, 0, 1, 0, 0};

/* LED identification
Discovery LEDs: PD 12, 13, 14, 15

|-number on pc board below LEDs
|   |- color
v vvvvvv  macro
12 green   
13 orange
14 red
15 blue
*/

/* ************************************************************
Turn the LEDs on in sequence, then turn them back off 
***************************************************************/
static int lednum = 12;	// Lowest port bit numbered LED
void toggle_4leds (void)
{
	if ((GPIO_ODR(GPIOD) & (1<<lednum)) == 0)
	{ // Here, LED bit was off
		GPIO_BSRR(GPIOD) = (1<<lednum);	// Set bits = all four LEDs off
	}
	else
	{ // HEre, LED bit was on
		GPIO_BSRR(GPIOD) = (1<<(lednum+16));	// Reset bits = all four LEDs on
	}
	lednum += 1;		// Step through all four LEDs
	if (lednum > 15) lednum = 12;

}
/***************************************************************************************************
Simple looping delay
****************************************************************************************************/
void looping_delay(unsigned int x)
{
	volatile unsigned int k=0;
	while (k++ < x);
	return;
}
/***************************************************************************************************
Experimenting with sprintf
****************************************************************************************************/
int faux_float (char *p, double x)
{
	int ix = x;
	unsigned int ixx;
	if (x >= 0)
	{
		ixx = rint((x - ix)*1000000);
		return	sprintf(p," %6d.%0i6 ",ix,ixx);
	}
	x = -x;
	ix = x;
	ixx = rint((x - ix)*1000000);
	if (ix == 0)
	{
		return sprintf(p,"     -0.%0i6 ",ixx);
	}
	return sprintf(p," %6d.%0i6 ",-ix,ixx);
		
}

/***************************************************************************************************
And now for the main routine 
****************************************************************************************************/
int main(void)
{
	volatile int i = 0; 	// Timing loop variable
	int j;			// Another (fixed pt FORTRAN!) variable

	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running

	/* This configures all for pins at once */
//	DISCgpiopins_Config();	// Now, configure pins

	/* Alternative setup scheme: four led pins on port D, pins 12,13,14,15 */
	for (j = 12; j < 16; j++)
		f4gpiopins_Config ((volatile u32 *)GPIOD, j, (struct PINCONFIG*)&ppoutput);


	for (j = 0; j < 4; j++)
	{
		LED_green_on;
		LED_blue_on;
		LED_orange_off;
		LED_red_off;
		
		looping_delay(750000);

		LED_green_off;
		LED_blue_off;
		LED_orange_on;
		LED_red_on;

		looping_delay(750000);
		LED_green_on;
		LED_blue_off;
		LED_orange_on;
		LED_red_off;
		
		looping_delay(750000);

		LED_green_off;
		LED_blue_on;
		LED_orange_off;
		LED_red_on;

		looping_delay(750000);
	}
/* 
The foregoing lets the hapless op know that it started OK.
When the leds start a circular pattern the hapless op can lift
his fat carcass off the chair with a rousing round of HUZZAHs.  
*/
double dX = sqrt(dpi);
dX = sin((dX*dX)*(5.0/4.0));

/* Test that malloc is not going crazy */
#define PMAL	256
pmal1 = malloc(PMAL);
pmal2 = malloc(PMAL);
pmal3 = malloc(PMAL);
pmal4 = malloc(PMAL);

/* When the leds begin a circular pattern.  In 'gdb' hit ctl C and do
printf "%s",xx
and you, the no longer hapless op, will be pleased with the wonderful numbers eminating. 
*/
int xidx = sprintf(       xx,"This is a  faux float test =");	xidx += faux_float (&xx[xidx], dpi); 
xidx    += sprintf(&xx[xidx]," and sin(pi*(5.0/4.0)): =");	xidx += faux_float (&xx[xidx], dX); 
	   sprintf(&xx[xidx],"\n");

/* Check that the floating point works */
sprintf(ww,"Float f: %f\n",dpi);
sprintf(yy,"Float e: %e\n",dpi);
sprintf(zz,"Float g: %g\n",dpi);


/* --------------------- Flash LEDs in a circular pattern ----------------------------------------------- */
//	for (j = 0; j < 40000; i++)
	i = 0;
	while (1==1)
	{
		/*LED blink rate timing */
		if ( i++ > 500000)
		{
			toggle_4leds();
			i = 0; j += 1;
		}


	}
	LEDSALL_off	// Turn off all 4 LEDs


	return 0;	
}

