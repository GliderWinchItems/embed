/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcardtest2.c
* Hackeroos          : caw, deh
* Date First Issued  : 05/25/2011
* Board              : STM32F103VxT6_pod_mm (USART1) or Olimex P103 (USART2)
* Description        : Test program for SDCARD driven from SPI2
*******************************************************************************/

/* NOTE: 
Some provisions in this routine not in the other usart test routines--
Change statements with USARTx to match board.

The LED setup is for the Olimex stm32 board.

Open minicom on the PC with 115200 baud and 8N1 and this routine
*/

/* Subroutine protoypes */
int caw_init(void); int (* const caw_init_ptr)(void) = caw_init;
void print_cawreal(char * p);
void sdcard_csd_list(void);


#include <math.h>
#include <string.h>

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"

#include "libopenstm32/systick.h"


#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/systick1.h"



#include "libmiscstm32/printf.h"
//#include "libmiscstm32/clocksetup.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "PODpinconfig.h"

#include "spi2sdcard.h"
#include "sdcard.h"
#include "libsupportstm32/sdcard_csd.h"
#include "sdcard_csd_print.h"
#include "sdcard_cid_print.h"

/* Simpleton routines to convert input ascii */
int dumbasctoint(char *p);
int dumb1st(char *p);
int dumb2nd(char *p);

char csd_buf[SDCARD_CSD_SIZE];
char cid_buf[SDCARD_CID_SIZE];

char caw_realdata[SDC_DATA_SIZE];
char caw_realdataX[SDC_DATA_SIZE];

/* ----------------- Clocking -------------------------------------------------- */

/* 'struct CLOCKS clocks' is used to setup the clock source, PLL, dividers, and bus clocks 
See P 84 of Ref Manual for a useful diagram.
../lib/libmiscstm32/clockspecifysetup.h has the 'enum' values that may help
in making mistakes(!) */
struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_6X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 0,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 0,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/
extern unsigned int	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/

/* ----------------- Clocking -------------------------------------------------- */


/* This if for test sprintf */
char vv[180];	// sprintf buffer

/* These are for testing SYSTICK routines */
u32 systick0;	// Save SYSTICK counter reading
u32 systick1;	// Save SYSTICK counter reading
u32 systick2;	// Save SYSTICK counter reading
u32 systick3;	// Save SYSTICK counter reading
u32 systick4;	// Save SYSTICK counter reading
u32 systickdur0;	// Duration result
unsigned long long ullsystick0;	// Very big number
unsigned long long ullsystick1;	// Very big number, also

/* This is for the tiny printf */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART2_txint_putc(c);
	}
/*****************************************************************************************/
/* Setup the gpio for the LED on the Olimex stm32 P103 board */
/*****************************************************************************************/
void gpio_setup(void)
{
	PODgpiopins_Config();	// Setup the pins for the STM32F103VxT6_pod_mm
	/* Setup GPIO pin for LED (PC12) (See Ref manual, page 157) */
	// 'CRH is high register for bits 8 - 15; bit 12 is therefore the 4th CNF/MODE position (hence 4*4)
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*4));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_2_MHZ) ) << (4*4));
	
}
/*****************************************************************************************/
/* Stupid routine for toggling the gpio pin for the LED */
/*****************************************************************************************/
#define LEDBIT	12	// Olimex board LED bit 
void toggle_led (void)
{
	if ((GPIO_ODR(GPIOC) & (1<<LEDBIT)) == 0)
	{
		GPIO_BSRR(GPIOC) = (1<<LEDBIT);	// Set bit
	}
	else
	{
		GPIO_BRR(GPIOC) = (1<<LEDBIT);	// Reset bit
	}
}
/* =============================== MAIN ================================================ */
/*****************************************************************************************/
/* And now for the main routine */
/*****************************************************************************************/
/* =============================== MAIN ================================================ */
int main(void)
{
	u16 temp;
	int i = 0; 		// Timing loop variable
	int j;			// Another (fixed pt FORTRAN!) variable
	int k; 			// Just a temp var
	int l;

	unsigned int uiBlockNum;// Block number typed int
	struct USARTLB lb;	// Holds the return from 'getlineboth' of char count & pointer
 
/* ---------- Initializations ------------------------------------------------------ */
//	clocksetup();		// Get the system clock and bus clocks running
	clockspecifysetup(&clocks);// Get the system clock and bus clocks running
	gpio_setup();		// Need this to make the LED work
	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine
	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter)

	/* Initialize usart 
	USARTx_rxinttxint_init(...,...,...,...);
	Receive:  rxint	rx into line buffers
	Transmit: txint	tx with line buffers
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx line buffer size, (long enough for the longest line)
		number of rx line buffers, (must be > 1)
		tx line buffer size, (long enough for the longest line)
		number of tx line buffers, (must be > 1)
	*/
	// Initialize USART and setup control blocks and pointer
	if ( (temp = USART2_rxinttxint_init(115200,32,2,48,4)) != 0 )
	{ // Here, something failed 
		return temp;
	}

	/* Announce who we are */
	USART2_txint_puts("\n\rsdcardtest2 USART2 txint rxint 08-28-2011\n\r");
	USART2_txint_send();	// Start the line buffer sending

	int nSpi2;
	int nTogglerate = 500000;	// Flashing at moderate speed
	
	/* --------------- initialization and messing-around with sd card -----------------------*/
	systick1 = SYSTICK_getcount32();	// Get 32 bit before starting card setup
	nSpi2 = caw_init();			// Do the fantastic SD Card initialization fandango
	systick0 = SYSTICK_getcount32();	// Get end time
	j = (systick1-systick0);		// Time duration in terms of systicks
	k = sysclk_freq/1000000;		// Scale from Hz to MHz

	/* Display things for to entertain the hapless op */
	printf ("pclk1_freq  (MHz)        : %9u\n\r",pclk1_freq/1000000);		USART2_txint_send();
	printf ("sysclk_freq (MHz)        : %9u\n\r",k);				USART2_txint_send();
	printf ("caw_init()   (us)        : %9u\n\r",j/k);				USART2_txint_send();

	/* Experiment with different SCLK speeds */
	// Display divisor setup by spi2sdcard.c 
	printf ("Routine spi2 divide code : %9u\n\r",(SPI2_CR1 >> 3) & 0x7);		USART2_txint_send();
	// Change the divisor and dispaly new value
	if ( (k = spi2_sclk_set_divisor_code(2)) != 0) {printf("divide code err: %u\n\r",k);USART2_txint_send();}
	printf ("Forced  spi2 divide code : %9u\n\r",spi2_sclk_get_divisor_code()); 	USART2_txint_send();	

	/* LED flashing is a quick way for a myopic op to know if it is working */
	if ( nSpi2 != 0)		// Did it initialize correctly?
	{ // Here no.  Show the return error number.
		printf ("caw_init() return code = %x \n\r",nSpi2); USART2_txint_send();
		nTogglerate /= 15;	// ==+> Flash fast if an error <====
	}

	/* List the CSD fields */
//	sdcard_csd_list();
	sdcard_csd_print(csd_buf);	// Print the extracted CSD fields
	unsigned int sd_size = sdcard_csd_memory_size (csd_buf);
	unsigned int sd_block_size = sdcard_csd_block_size (csd_buf);
	printf ("SD Card size: number blocks %u, of block size: %u\n\r",sd_size,sd_block_size);								USART2_txint_send();

	/* List CID fields */
	sdcard_cid_print(cid_buf);	// Print the extracted CSD fields
	

	/* Some instructions for the clueless op */
	USART2_txint_puts("\n\rNOTE: very rudimentary type-in editing \n\r");					USART2_txint_send();
	USART2_txint_puts("Enter: 'p xxxx<ret>' to read block number xxxx into buffer and display \n\r");	USART2_txint_send();
	USART2_txint_puts("Enter: 'w xxxx<ret>' to write block number xxxx out of buffer \n\r");		USART2_txint_send();
	USART2_txint_puts("Enter: 'c xxxx|yyyy <ret>' to compare block x with block y \n\r");			USART2_txint_send();


/* ========================== Endless loop ================================================ */
	/* Blink the LED on the board so that we can see it is alive */
	while (1==1) 
	{
		/*LED blink rate timing */
		if ( i++ > nTogglerate)
		{
			toggle_led();
			i = 0;
		}
		
		/* The following is partly a demonstration of the different ways to handle USART input */
		lb = USART2_rxint_getlineboth();		// Get both char count and pointer
			/* Check if a line is ready.  Either 'if' could be used */
//			if (lb.ct > 0)				// Check if we have a completed line
		if (lb.p > (char*)0)				// Check if we have a completed line
		{ // Here we have a pointer to the line and a char count
			USART2_txint_puts(lb.p);		// Echo back the line just received
			USART2_txint_puts ("\n");		// Add line feed to make things look nice
			USART2_txint_send();			// Start the line buffer sending
			systick0 = SYSTICK_getcount32();	// Get 32 bit time for the first case below
			ullsystick0 = SYSTICK_getcount64();	// For long elapsed time (down to the very last tick)

			switch (*lb.p)

			{
			/* When the operator types 'p xxxxxxx' <ret> read and display the block number  */
			case 'p':
				uiBlockNum = dumbasctoint (lb.p);
				systick1 = SYSTICK_getcount32(); // Get 32 bit before starting card setup
				k = sdcard_read(uiBlockNum, caw_realdata);
				systick0 = SYSTICK_getcount32();// Time of completion
		
				/* Check that read return is OK */	
				if ( k != 0 )
				{ // Here failed...maybe
					printf ("\n\rsdcard_read return: %u\n\r",k);  USART2_txint_send();
				}

				/* Time the read and print in microseconds */
				j = (systick1-systick0);	// System ticks during the read
				k = sysclk_freq/1000000;	// Scale system clock to microseconds
				printf ("Block number: %u    ",uiBlockNum);
				printf ("sdcard_read duration (us): %9u\n\r",j/k);	USART2_txint_send();
				print_cawreal (caw_realdata); 	USART2_txint_send();	// Display block
				break;
			/* When the operator types 'w xxxxxxx' <ret> write the block and display the block number  */
			case 'w':
				uiBlockNum = dumbasctoint (lb.p);
				systick1 = SYSTICK_getcount32(); // Get 32 bit before starting card setup
			int ii,iend = 1;
				for (ii = 0; ii < iend; ii++)
					k = sdcard_write(uiBlockNum +ii, caw_realdata);
				systick0 = SYSTICK_getcount32();// Time of completion
		
				/* Check that read return is OK */	
				if ( k != 0 )
				{ // Here failed...maybe
					printf ("\n\rsdcard_read return: %u\n\r",k);  USART2_txint_send();
				}

				/* Time the read and print in microseconds */
				j = (systick1-systick0);	// System ticks during the read
				k = sysclk_freq/1000000;	// Scale system clock to microseconds
				printf ("Block number: %u    ",uiBlockNum);
				printf ("sdcard_write duration (us): %9u blks %u\n\r",j/k,iend);	USART2_txint_send();
				break;

			/* When the operator types 'c xxxxxxx yyyyyyy' <ret> read and compare the two blocks  */
			case 'c':
				j = dumb1st (lb.p);
				if (j < 0) break;	// Break on a bad input
				k = dumb2nd (lb.p);
				printf ("Compare block %u with block %u\n\r",j,k);	USART2_txint_send();	
				sdcard_read(j, caw_realdata);
				sdcard_read(k, caw_realdataX);
				l = 0;
				for ( i = 0; i <SDC_DATA_SIZE; i++)
				{
					if (caw_realdata[i] != caw_realdataX[i])
					{
						printf ("%3u %02x %02x\n\r",i,caw_realdata[i],caw_realdataX[i]);USART2_txint_send();
						l = 1;	// Set flag
					}
				}
				if (l == 0 )
					{printf ("No differences\n\r");USART2_txint_send();}
				break;
			}
		}
	}
	return 0;
}
/*****************************************************************************************/
/* caw stuff                                                                             */
/*****************************************************************************************/
int caw_init(void)
{
	return sdcard_init(cid_buf,csd_buf);
}
/*****************************************************************************************/
/* Print out a block in hex.  16 per line                                                */
/*****************************************************************************************/
void print_cawreal(char * p)
{
	int i,j;	
		j = 0;
		for ( i = 0; i <SDC_DATA_SIZE; i += 16)
		{
			for (j = 0; j < 16; j++)
			{
				printf ("%02x ",*(p+j));
			}
			printf ("   ");
			for (j = 0; j < 16; j++)
			{
				if ( (*(p+j) >= 0x20) && (*(p+j) < 0x7f) )
					printf ("%c",(*(p+j) & 0x7f));
				else
					printf (".");
			}
			printf ("\n\r");
			USART2_txint_send();
			p += 16;
		}
		return;
}
/*****************************************************************************************/
/* Convert input to int			                                                */
/*****************************************************************************************/
int dumbasctoint(char *p)
{
	int x = 0;

	while (*p != 0)
	{
		if   ((*p >= '0') && (*p <= '9') )
		{
			x = x * 10 + (*p - '0');
		}	
		p++;	
	}
	return x;
}
/*****************************************************************************************/
/* Convert two numbers 			                                                */
/*****************************************************************************************/
int dumb1st(char *p)
{
	int x = 0;

	char *p2 = p;
	while (*p2 != 0)
	{
		if (*p2 == '|')
		{	

			while (*p != '|')	// Stop at separator
			{
				if   ((*p >= '0') && (*p <= '9') )
				{
					x = x * 10 + (*p - '0');
				}	
				p++;	
			}
			return x;
		}
		p2++;
	}
	printf ("Input err: Put a '|' between the block numbers\n\r");	USART2_txint_send();
		
	return -1;
}
int dumb2nd(char *p)
{
	while (*p++ != '|');	// Spin forward to separator
	return dumbasctoint(p);
}

