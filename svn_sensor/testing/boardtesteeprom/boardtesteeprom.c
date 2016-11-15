/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : boardtesteeprom.c
* Author             : deh
* Date First Issued  : 05/26/2013
* Board              : STM32F103RxT6 sensor board
* Description        : Test eeprom
*******************************************************************************/

/* 
Hack of 'boardtest1.c' to test/develop routines for eeprom.

Open minicom on the PC with 115200 baud and 8N1.

Timing for write--
time(us) = 16 * number chars + 3480 * number of 32 blocks;
1.02 secs for 8192

Timing for read--
time(us) = 16.7 * number chars;
131 ms for 8192


b15x = green led
c04x = red led 19 & pin 2 pad HC7414 (5.0v)
c05x = red led 20 & pin 4 pad HC7414 (5.0v)
c10x = HC7414 pin 12 (3.3v)

*/
#include <math.h>
#include <string.h>


#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"
#include "SENSORpinconfig.h"

#include "rw_eeprom.h"
#include "spi1eeprom.h"

#define DWT_CYCNT (*(volatile unsigned int *)0xE0001004) // DWT_CYCNT

/* For test with and without XTAL clocking */
#define NOXTAL 
#ifdef NOXTAL
/* No xtal--use internal osc (reset default) */
/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
// INTERNAL RC osc parameters -- 64 MHz
const struct CLOCKS clocks = { \
HSOSELECT_HSI,	/* Select high speed osc 			*/ \
PLLMUL_16X,		/* Multiplier PLL: 0 = not used 		*/ \
0,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

#else
/* External xtal: use osc with external xtal */
/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_9X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSI/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

#endif


static void display (int address, int count);
static void zero_a(void);
static void prthdr(void);

/* This if for test sprintf */
char vv[180];	// sprintf buffer

/* This is for the tiny printf */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART1_txint_putc(c);
	}

/* LED identification
(SSN = eagle silk screen name)
(SCN = eagle signal name)

SSN:	$U19 RED 
SCN:	PC4_LED

SSN:	$U20 RED (Nearest RJ-11)
SCN:	PC5_LED

SSN:	$U21 GREEN (Near top left of board)
SCN:	PB15_SPI2_DI

*/

static char a[8192];

/* ************************************************************
Step through the LEDs
***************************************************************/
static int ledct;

void walk_LEDs(void)
{
	switch (ledct)
	{
	case 0: LED19RED_off;		LED20RED_off;		LED21GREEN_on;		break;
	case 1: LED19RED_off;		LED20RED_on;		LED21GREEN_off;		break;
	case 2: LED19RED_on;		LED20RED_off;		LED21GREEN_off;		break;
	default: ledct = 0; break;
	}
	ledct += 1;		// Step through all four LEDs
	if (ledct > 2) ledct = 0;
	return;
}
/***************************************************************************************************
And now for the main routine 
****************************************************************************************************/
int main(void)
{
	struct USARTLB lb;	// Holds the return from 'getlineboth' of char count & pointer
volatile int i = 0; 		// Timing loop variable
	int j;			// Another (fixed pt FORTRAN!) variable
	int eeprom_ret;		// eeprom function call returned
	u32 t2 = 0;
	u32 t1;
	u32 t3;
	float fT;

/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup((struct CLOCKS *)&clocks);		// Get the system clock and bus clocks running

	SENSORgpiopins_Config();	// Now, configure pins

	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter)

	/* Use DTW_CYCCNT counter for timing */
/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)

/* --------------------- Initialize usart --------------------------------------------------------- */
/*	USARTx_rxinttxint_init(...,...,...,...);
	Receive:  rxint	rx into line buffers
	Transmit: txint	tx with line buffers
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx line buffer size, (long enough for the longest line)
		number of rx line buffers, (must be > 1)
		tx line buffer size, (long enough for the longest line)
		number of tx line buffers, (must be > 1)
*/
	USART1_rxinttxint_init(115200,32,2,8,3); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rboardtesteeprom.c--eeprom test 05-26-2013\n\r");
	USART1_txint_send();	// Start the line buffer sending

/* --------------------- eeprom --------------------------------------------------------------------- */
	if ((j=rw_eeprom_init()) != 0)
	{
		printf("eeprom init failed: %i\n\r",j); USART1_txint_send();
	}

	/* Print header */
	prthdr();
	
/* --------------------- Flash LEDs a number of times ----------------------------------------------- */
//	for (j = 0; j < 40000; i++)
int address;
int count;
int sw;

	zero_a();	// Load array with stuff

	while (1==1)
	{
		/*LED blink rate timing */
		if ( i++ > 500000)
		{
			walk_LEDs();
			i = 0; j += 1;
		}
		lb = USART1_rxint_getlineboth();		// Get both char count and pointer
		if (lb.p > (char*)0)				// Check if we have a completed line
		{ // Here we have a pointer to the line and a char count
			USART1_txint_puts(lb.p);		// Echo back the line just received
			USART1_txint_puts ("\n");		// Add line feed to make things look nice
			USART1_txint_send();			// Start the line buffer sending
			switch (*lb.p)
			{
			case 'n': // Set write enable latch to on or off
				sscanf (lb.p+1,"%x",&count);
				printf ("write status register with:  0x%02x\n\r",count); 	USART1_txint_send();
				eeprom_ret=eeprom_write_enable_latch(count); // Write command
				if ( (eeprom_ret & 0xff00) != 0)	// Check for error
				{ // Here, something went wrong
					printf("write-enable-latch setup error: 0x%04x\n\r",eeprom_ret); USART1_txint_send();
					break;
				}
				/* Read status register to see if the write worked. */
				eeprom_ret=eeprom_read_status_reg();	// Read status register
				if ( (eeprom_ret & 0xff00) != 0)	// Check for setup error
				{ // Here, something went wrong
					printf("status reg read error: 0x%04x\n\r",eeprom_ret); USART1_txint_send();
					break;
				}
				printf ("re-read of write status reg: 0x%02x\n\r",eeprom_ret); USART1_txint_send();
				break;				


			case 'v': // Write status register
				sscanf (lb.p+1,"%x",&count); count &= 0xff; // jic
				printf ("write status register with:  0x%02x\n\r",count); 	USART1_txint_send();
				eeprom_ret=eeprom_write_status_reg((unsigned char)count); // Write status register
				if ( (eeprom_ret & 0xff00) != 0)	// Check for error
				{ // Here, something went wrong
					printf("write status reg error: 0x%04x\n\r",eeprom_ret); USART1_txint_send();
					break;
				}
				/* Read status register to see if the write worked. */
				eeprom_ret=eeprom_read_status_reg();	// Read status register
				if ( (eeprom_ret & 0xff00) != 0)	// Check for setup error
				{ // Here, something went wrong
					printf("status reg read error: 0x%04x\n\r",eeprom_ret); USART1_txint_send();
					break;
				}
				printf ("re-read of write status reg: 0x%02x\n\r",eeprom_ret); USART1_txint_send();
				break;				
			
			case 's': // Read & display status register 
/******************************************************************************
 * unsigned int eeprom_read_status_reg(void);
 * @brief	: Read the status register
 * @return	: high byte = 0 for OK, 0xff bad; low byte = status register
******************************************************************************/
				eeprom_ret=eeprom_read_status_reg();	// Read status register
				if ( (eeprom_ret & 0xff00) != 0)	// Check for setup error
				{ // Here, something went wrong
					printf("status reg read error: %04x\n\r",eeprom_ret); USART1_txint_send();
					break;
				}
				printf ("status reg: 0x%02x\n\r",eeprom_ret); USART1_txint_send();
				break;

			case 'r': // Read from eeprom 
				zero_a();	// Load array with stuff
				sscanf (lb.p+1, "%x %i",&address, &count);	// address-hex, count-decimal
				printf ("read: address: %u = 0x%04x, count %u\n\r",address,address,count); 	USART1_txint_send();
/******************************************************************************
 * int eeprom_read(u16 address, char *p, int count);
 * @brief	: Read from eeprom
 * @param	: address--eeprom address to start read (0-8191)
 * @param	: p--pointer to buffer 
 * @param	: count--number of bytes to read (1-8192)
 * @return	: zero = OK. not zero = failed initial setup
******************************************************************************/
				t1 = DWT_CYCNT;		// Get system 32b counter at beginning of operation.

				/* Read into 8K byte array at same position as 8K eeprom */
				if ((eeprom_ret=eeprom_read(address,(char*)&a[address], count)) != 0)
				{ // Here, the write setup failed (arguments out of range, e.g.)
					printf("eeprom_read: return failed--%u\n\r",eeprom_ret);		USART1_txint_send();
					break;
				}
				while ((eeprom_ret=eeprom_busy()) > 0); // Wait for sequence to complete
				if (eeprom_ret < 0)	// Did something time out?
				{ // Here yes.
					t2 = DWT_CYCNT;
					fT = 1E6/sysclk_freq;	// microsecs per counter tick
					t3 = fT * (t2-t1);	// microsecs for the operation
					printf("system timer count: %u, microsecs: %u\n\r",(t2-t1),t3);		USART1_txint_send();
					printf("eeprom_busy returned: 0x%02x\n\r",eeprom_ret);			USART1_txint_send();
					break;
				}
				t2 = DWT_CYCNT;
				fT = 1E6/sysclk_freq;	// microsecs per counter tick
				t3 = fT * (t2-t1);	// microsecs for the operation
				printf("system timer count: %u, microsecs: %u\n\r",(t2-t1),t3);			USART1_txint_send();


				display (address,count);
//				display (0, 8192);	// Display all
				break;

			case 'w': // Write to eeprom
				sscanf (lb.p+1, "%x %i %x",&address, &count, &sw);

				if (sw == 0)
					zero_a();
				else
					memset(a,sw,8192);
				
				printf ("write: address: %u = 0x%04x, count %u, byte %02x\n\r",address,address,count,sw); 	USART1_txint_send();
				display (address,count);
/******************************************************************************
 * int eeprom_write(u16 address, char *p, int count);
 * @brief	: Write to eeprom
 * @param	: address--eeprom address to start write (0-8191)
 * @param	: p--pointer to buffer 
 * @param	: count--number of bytes to write (1-8192)
 * @return	: zero = OK. not zero = failed initial setup.
******************************************************************************/
extern int eeprom_loopctr;	// Timeout loop counter

volatile unsigned int lpctr=0; // Delay to allow USART to complete so that single step debugging isn't interrupted.
while (lpctr++ < 300000);

				t1 = DWT_CYCNT;		// Get system 32b counter at beginning of operation.
				/* Write out of memory array position into eeprom */
				if ((eeprom_ret=eeprom_write(address,(char*)&a[address], count)) != 0)
				{ // Here, the write setup failed (arguments out of range, e.g.)
					t2 = DWT_CYCNT; // Get system 32b counter at end of operation.
					printf("eeprom_write: return failed--%u\n\r",eeprom_ret);		USART1_txint_send();
					break;
				}
				while ((eeprom_ret=eeprom_busy()) > 0);  // Wait for sequence to complete
				t2 = DWT_CYCNT;
				fT = 1E6/sysclk_freq;	// microsecs per counter tick
				t3 = fT * (t2-t1);	// microsecs for the operation
				printf("system timer count: %u, microsecs: %u\n\r",(t2-t1),t3);			USART1_txint_send();
				if (eeprom_ret < 0)	// Did something time out?
				{ // Here yes.
					printf("eeprom_busy returned:   0x%02x\n\r",eeprom_ret);		USART1_txint_send();
					printf("eeprom_status register: 0x%02x\n\r",eeprom_status_register);	USART1_txint_send();
					break;
				}
					printf("eeprom_status register: %u\n\r",eeprom_status_register);	USART1_txint_send();
					printf("debug1--busy loop ctr : %u\n\r",debug1);			USART1_txint_send();
					printf("eeprom_loopctr ctr    : %u\n\r",eeprom_loopctr);		USART1_txint_send();
				break;

			default:
				USART1_txint_puts ("Oops!\n\r");USART1_txint_send();
				break;				
			}

			prthdr();
			USART1_txint_puts("\n\r");	USART1_txint_send();
		}

	}

	LED19RED_off;		LED20RED_off;		LED21GREEN_on;




	return 0;	
}
/************************************************************************************************
static void display (int address, int count)
 * @brief	: Display in hex 
 * @param	: address = eeprom address of 1st byte
 * @param	: count = number of bytes
 ************************************************************************************************/
static void perr(int x)
{
	printf ("Bogus numbers or timeout (5): error # %u\n\r",x);	USART1_txint_send(); 	return;
}
static void display (int address, int count)
{
	int i;
	int idx = address;
	if (count < 1) 		{perr(1); return;}
        if (count > 8192) 	{perr(2); return;}
        if (address < 0 ) 	{perr(3); return;}
        if (address > 8191) 	{perr(4); return;}
	if ((address + count) > 8192) {perr(5); return;}

//	printf("%04x:", address); 	USART1_txint_send();
	for (i = 0; i < count; i++)
	{
		if ((i & 0x1f) == 0x00)
		{ 
			printf("\n\r%04x: ", address); 	USART1_txint_send();
			address += 0x20;
		}
		printf (" %02x",a[i+idx]); 	USART1_txint_send();
	}
	USART1_txint_puts("\n\r");	USART1_txint_send();
	return;
}
/************************************************************************************************
static void zero_a(void);
 * @brief	: Load array
 ************************************************************************************************/
void zero_a(void)
{
	int i;
	for (i = 0; i < 8192; i++)
	{
		a[i] = i;
	}
	return;
}
/************************************************************************************************
static void prthdr(void);
 * @brief	: print header
 ************************************************************************************************/
static void prthdr(void)
{
	USART1_txint_puts("Enter: 'n d<ret>' set write enable latch (bit 2 of status): 0 = OFF, not 0 = ON \n\r");	USART1_txint_send();
	USART1_txint_puts("Enter: 'r xxxx nnnn<ret>' to read 'n' bytes starting at hex address 'x'  and display \n\r");	USART1_txint_send();
	USART1_txint_puts("Enter: 's <ret>' to read status register  and display \n\r");				USART1_txint_send();
	USART1_txint_puts("Enter: 'v xx (in hex)<ret>' to write status register with 'xx' \n\r");			USART1_txint_send();
	USART1_txint_puts("Enter: 'w xxxx nnnn xx<ret>' to display then write 'n' bytes starting at hex address 'xxxx', fill with (hex) 'xx', xx = 0 ctr\n\r");	USART1_txint_send();
	return;
}
