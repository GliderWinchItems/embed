/******************************************************************************
* File Name          : gpio1.c
* Date First Issued  : 10/29/2013
* Board              : Discovery F4
* Description        : Simple gpio demo/test using USB to PC link
*******************************************************************************/
/* 
This routine is a crude, simple routine to turn gpio pins on and off by typing 
in the port, pin, and 1|0 for on|off. 

The input editing is simple; none of the usual editing featurs such as backspace.

One way to use this routine--

1) set permissions for ST-LINK
./p [then type in password]
(Be sure you get something like the following--
Setting stlink permissions
/dev/bus/usb/003/046
and not--
Setting stlink permissions
/dev/bus/usb// [PC not seeing 'F4]

2) compile everything (and if OK) load flash
./mm && make flash

3) Open minicom 
sudo minicom /dev/ACM0
if minicom doesn't find it, the 'F4 is not running the usb correctly.
- try pressing reset button, then re-execute minicom
- sometimes pulling the ST-LINK usb, wait a few secs for the PC to
  see it gone, then plug it back in, and do ./p for permissions again,
  will make it work.  

4) Test
All four LEDs will be on.  type d120 to turn off the green, 
d150 turns off blue, d151 turns it on, etc.  It should work for
all the other i/o pins.  With a VOM to check the voltage at pins
that are not connected to a LED.

*/
#include <stdio.h>

#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/gpio.h"

#include "clockspecifysetup.h"

#include "DISCpinconfig.h"	// Pin configuration for STM32 Discovery board

#include "panic_leds.h"
//#include "default_irq_handler.h"


//Don't use: #include "usb1.h" because it will bring in ST defines that are conflict with libopencm3.
void usb1_init(void);	// This is the only link to the usb.  The rest is in 'syscalls.c'

void Default_Handler(void) { panic_leds(5); }

/* Why is this needed!? Not needed in Charlie's 'my_main.c' */
int _read(int file, char *ptr, int len);


/* The following 'struct CLOCKS' values provide --
External 8 MHz xtal
sysclk  =  168 MHz
PLL48CK =   48 MHz
PLLCLK  =  168 MHz
AHB     =  168 MHz
APB1    =   42 MHz
APB2    =   84 MHz

NOTE: PLL48CK *must* be 48 MHz for the USB to work.
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



/* LED identification
Discovery F4 LEDs: PD 12, 13, 14, 15

|-number on pc board below LEDs
|  |- color
vv vvvvvv 
12 green   
13 orange
14 red
15 blue
*/

/* Pin configuration to use pin as gp output.  For the options see--
'../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/stm32/f4/gpio.h' */
const struct PINCONFIG	outpp = { \
	GPIO_MODE_OUTPUT,	// mode: output 
	GPIO_OTYPE_PP, 		// output type: push-pull 		
	GPIO_OSPEED_100MHZ, 	// speed: highest drive level
	GPIO_PUPD_NONE, 	// pull up/down: none
	0 };		// Alternate function code: not applicable

/* ************************************************************
 Build a line of chars from keyboard
***************************************************************/
char vv[64];
char* pvv = &vv[0];
char* pend = &vv[64];
int getline(char c)
{
	*pvv++ = c;
	if (pvv >= pend) pvv -= 1;
	if (c == 0x0d)	// End of line?
	{ // Here yes.
		*pvv++ = 0;	// Line terminator
		pvv = &vv[0];	// Reset pointer
		return 1;
	}
	return 0;
}

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int b,n1,n2,n;

/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup((struct CLOCKS*)&clocks);	// Get the system clock and bus clocks running

	DISCgpiopins_Config();	// Configure pins
LEDSALL_on;

/* ---------------------- Initialize for usb ------------------------------------------------------ */
	usb1_init();	// Initialization for USB (STM32F4_USB_CDC demo package)
	setbuf(stdout, NULL);

/* --------------------- A little output to show the program is alive ------------------------------- */
	/* The following is done 3 times since the PC will take a short period of time to recognize
	   /dev/ttyACM0 is present and chars at the beginning are lost. */
	int i;
	for (i = 0; i < 3; i++)
	{
		/* Announce who we are. */
		printf(" \n\rDISCOVERY F4 GPIO1: 10-31-2013  0\n\r");

		/* Display the clock and bus frequencies. */
		printf ("   hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	
		printf ("  pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	
		printf ("  pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	
		printf (" sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);
	}
/* --------------------- Configure the pins for the four LEDs ---------------------------------------- */
	for (i = 0; i < 4; i++)	// Configure PD 12, 13, 14, 15
		f4gpiopins_Config ((volatile u32 *)GPIOD, (12+i), (struct PINCONFIG*)&outpp);
	
/* --------------------- Endless loop follows -------------------------------------------------------------------------- */
/* --------------------- Do Port bit commands -------------------------------------------------------------------------- */
	printf("Discovery F4 board LED port, pin: green = d12, orange = d13, red = d14, blue = d15\n\r");
	printf("Enter port char (p) a-e, bit (bb) 00-15, (n) 0 or 1 for set low or high as\n\rpbbn\n\rd120 [port D, pin 12 (LED), set low];\n\r");
	char c;
	while(1==1)
	{
		if(_read(0, &c, 1))
		{
//	printf("%02x ", c); // what sort of hex is coming from keyboard via usb?
			if (getline(c) != 0) // Do we have a line?
			{ // Here, yes
				printf("%s\n\r",vv);	// Echo back the line just received

				n1 = vv[1] - '0';	// Simple ASCII to binary
				n2 = vv[2] - '0';
				b  = vv[3] - '0';
				n = n1*10+n2;		// Port bit runs 0 - 15
				if ( (n < 0) | (n > 15) | (b < 0) | (b > 1) )	// Check for bogus numbers
				{
					printf("Bogus numbers\n\r");
				}				
				else
				{	
					printf("port %c pin %i hi/lo %c\n\r",vv[0],n,vv[3]);
					switch (vv[0])	// Case based on port designation a-e
					{
					case 'a':
				f4gpiopins_Config ((volatile u32 *)GPIOA, n, (struct PINCONFIG*)&outpp);
						if (b == 0) GPIO_BSRR(GPIOA) = (1<<(n+16));// Reset bit
						if (b == 1) GPIO_BSRR(GPIOA) = (1<<n);	  // Set bit
						break;
					case 'b':
				f4gpiopins_Config ((volatile u32 *)GPIOB, n, (struct PINCONFIG*)&outpp);
						if (b == 0) GPIO_BSRR(GPIOB) = (1<<(n+16));// Reset bit
						if (b == 1) GPIO_BSRR(GPIOB) = (1<<n);     // Set bit
						break;
					case 'c':
				f4gpiopins_Config ((volatile u32 *)GPIOC, n, (struct PINCONFIG*)&outpp);
						if (b == 0) GPIO_BSRR(GPIOC)  = (1<<(n+16));// Reset bit
						if (b == 1) GPIO_BSRR(GPIOC)  = (1<<n);	   // Set bit
						break;
					case 'd':
				f4gpiopins_Config ((volatile u32 *)GPIOD, n, (struct PINCONFIG*)&outpp);
						if (b == 0) GPIO_BSRR(GPIOD)  = (1<<(n+16));// Reset bit
						if (b == 1) GPIO_BSRR(GPIOD)  = (1<<n);	   // Set bit
						break;
					case 'e':
				f4gpiopins_Config ((volatile u32 *)GPIOE, n, (struct PINCONFIG*)&outpp);
						if (b == 0) GPIO_BSRR(GPIOE)  = (1<<(n+16));// Reset bit
						if (b == 1) GPIO_BSRR(GPIOE)  = (1<<n); 	   // Set bit
						break;
					default:	
						printf("Bogus port number: %i\n\r", n);
						break;
					}
					printf("Enter port cmd: pnnb (p:a-e, x to quit; nn = 00-15; b = 0-1)\n\r");
				}
			}
		}
	}
	return 0;	
}






