/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcardtest.c (originally sdcardtest2.c)
* Hackeroos          : caw, deh
* Date First Issued  : 05/25/2011
* Date Last Edit     : 20130623.2040
* Board              : STM32F103VxT6_pod_mm (USART1) or Olimex P103 (USART2)
* Description        : Test program for SDCARD driven from SPI2
*******************************************************************************/

/* NOTE: 
Some provisions in this routine not in the other usart test routines--
Change statements with USARTx to match board.

The LED setup is for the Olimex stm32 board.

Open minicom on the PC with 115200 baud and 8N1 and this routine
*/

int caw_init(void);
int (* const caw_init_ptr)(void) = caw_init;
void print_cawreal(char * p);
void dump_cawreal(char * p);

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
#include "./sdcard.h"
#include "libsupportstm32/sdcard_csd.h"
#include "libsupportstm32/sdcard_csd_print.h"
#include "libsupportstm32/sdcard_cid_print.h"

#include "./crc_ccitt.h"


/* Simpleton routines to convert input ascii */
int dumbasctoint(char *p);
int dumb1st(char *p);
int dumb2nd(char *p);


char caw_cid_data[SDCARD_CID_SIZE];
char caw_csd_data[SDCARD_CSD_SIZE];
char caw_realdata[SDC_DATA_SIZE];
char caw_realdataX[SDC_DATA_SIZE];
char caw_tmpdata[SDC_DATA_SIZE];

int caw_arg;							/* Arg for rw test */
int caw_flag;
void (*ptrwf)(void) = (void*)0;			/* Pointer to read/write test */
#define	NEXT_STATE(funct) do { void (funct)(void); ptrwf = funct; } while(1==0)
unsigned long pass_no, data_mask;

/* ----------------- Select USART based on board type  ----------- */
// USART1 = POD, Sensor
// USART2 = Olimex

//#define OLIMEX	// Comment out for POD
#ifdef OLIMEX
#define USART_txint_putc USART2_txint_putc
#define USART_txint_send USART2_txint_send
#define USART_txint_puts USART2_txint_puts
#define USART_rxinttxint_init USART2_rxinttxint_init
#define USART_rxint_getlineboth USART2_rxint_getlineboth
#else
#define USART_txint_putc USART1_txint_putc
#define USART_txint_send USART1_txint_send
#define USART_txint_puts USART1_txint_puts
#define USART_rxinttxint_init USART1_rxinttxint_init
#define USART_rxint_getlineboth USART1_rxint_getlineboth
#endif

/* ----------------- Clocking -------------------------------------------------- */

/* For test with and without XTAL clocking */
//#define NOXTAL 	// Comment to use XTAL
#ifdef NOXTAL

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

/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
// EXTERNAL xtal osc parameters -- 64 MHz
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_8X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSI/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

#endif


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

unsigned int uiSys0,uiSys1,uiSysDur;	// Time write operation
unsigned int uiMax;		// Find max write time
unsigned long long ulAve;	// Ave write time

/* The following 'dummy' causes USART2 routines to be brought 
   in so that we don't get undefined reference. */
void dummy (void) {USART2_txint_putc(' ');}


/* This is for the tiny printf */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART_txint_putc(c);
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
/* Stupid routine for toggling the gpio pin for the LED -- POD */
/*****************************************************************************************/
/* LED identification

|-number on pc board below LEDs
|   |- color
v vvvvvv  macro
3 green   
4 red
5 green
6 yellow
*/


static int lednum = LED3;	// Lowest port bit numbered LED
void toggle_4leds (void)
{
	if ((GPIO_ODR(GPIOE) & (1<<lednum)) == 0)
	{ // Here, LED bit was off
		GPIO_BSRR(GPIOE) = (1<<lednum);	// Set bits = all four LEDs off
	}
	else
	{ // HEre, LED bit was on
		GPIO_BRR(GPIOE) = (1<<lednum);	// Reset bits = all four LEDs on
	}
	lednum += 1;		// Step through all four LEDs
	if (lednum > LED6) lednum = LED3;
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
	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running
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
	if ( (temp = USART_rxinttxint_init(115200,32,2,48,4)) != 0 )
	{ // Here, something failed 
		return temp;
	}

	/* Announce who we are */
	USART_txint_puts("\n\rSDCARDTEST\n\r");  USART_txint_send();	// Start the line buffer sending
	int nSpi2;
	int nTogglerate = 500000;	// Flashing at moderate speed

	/* Display things for to entertain the hapless op */
	printf ("pclk1_freq  (MHz): %9u\n\r",pclk1_freq/1000000);		USART_txint_send();
	printf ("sysclk_freq (MHz): %9u\n\r",k);				USART_txint_send();
	printf ("caw_init()   (us): %9u\n\r",j/k);				USART_txint_send();
	
#if (1==0)	/* Grubby test code */
{
	int x, i, j, k;
	unsigned char b[64];
	
	x = sdcard_ll_init();
	printf("sdcard_ll_init()                   0x%08x\n\r", x);
	USART_txint_send();
	
	x = sdcard_ll_cmd(CMD0, 0, NULL);
	printf("sdcard_ll_cmd(CMD0, 0, NULL)       0x%08x\n\r", x);
	USART_txint_send();

	for(i=0,j=4; i<j; i+=1) b[i] = 0x55;
	x = sdcard_ll_cmd(CMD8, 0x1aa, b);
	printf("sdcard_ll_cmd(CMD8, 0x1aa, b)      0x%08x, ", x);
	printf("b[0/%d] is 0x", j-1);
	for(i=0; i<j; i+=1) printf("%02x", b[i]); printf("\r\n");
	USART_txint_send();
	
	for(i=0,j=4; i<j; i+=1) b[i] = 0x55;
	x = sdcard_ll_cmd(CMD58, 0, b);
	printf("sdcard_ll_cmd(CMD58, 0, b)         0x%08x, ", x);
	printf("b[0/%d] is 0x", j-1);
	for(i=0; i<j; i+=1) printf("%02x", b[i]); printf("\r\n");
	USART_txint_send();

	for(i=0,j=4; i<j; i+=1) b[i] = 0x55;
	for(k=0; k<10000; k+=1)
	{
		x = sdcard_ll_cmd(ACMD41, 1<<30, NULL);
		if(x == SDCARD_LL_TIMEOUT)
			break;
		if((x & R1_IN_IDLE_STATE) == 0)
			break;
	}
	printf("sdcard_ll_cmd(ACMD41, 1<<30, NULL) 0x%08x, ", x);
	printf("after %d loops\r\n", k);
	USART_txint_send();

	for(i=0,j=4; i<j; i+=1) b[i] = 0x55;
	x = sdcard_ll_cmd(CMD58, 0, b);
	printf("sdcard_ll_cmd(CMD58, 0, b)         0x%08x, ", x);
	printf("b[0/%d] is 0x", j-1);
	for(i=0; i<j; i+=1) printf("%02x", b[i]); printf("\r\n");
	USART_txint_send();

	for(i=0,j=16; i<j; i+=1) b[i] = 0x55;
	x = sdcard_ll_cmd(CMD9, 0, b);
	printf("sdcard_ll_cmd(CMD9, 0, b)          0x%08x, ", x);
	printf("b[0/%d] is 0x", j-1);
	for(i=0; i<j; i+=1) printf("%02x", b[i]); printf("\r\n");
	USART_txint_send();

	for(i=0,j=16; i<j; i+=1) b[i] = 0x55;
	x = sdcard_ll_cmd(CMD10, 0, b);
	printf("sdcard_ll_cmd(CMD10, 0, b)         0x%08x, ", x);
	printf("b[0/%d] is 0x", j-1);
	for(i=0; i<j; i+=1) printf("%02x", b[i]); printf("\r\n");
	USART_txint_send();

	do {} while(1==1);
}
#endif		/* Grubby test code */
	
	systick1 = SYSTICK_getcount32();	// Get 32 bit before starting card setup
	nSpi2 = caw_init();			// Do the fantastic SD Card initialization fandango
	systick0 = SYSTICK_getcount32();	// Get end time
	j = (systick1-systick0);		// Time duration in terms of systicks
	k = sysclk_freq/1000000;		// Scale from Hz to MHz

	/* Display things for to entertain the hapless op */
	printf ("pclk1_freq  (MHz): %9u\n\r",pclk1_freq/1000000);		USART_txint_send();
	printf ("sysclk_freq (MHz): %9u\n\r",k);				USART_txint_send();
	printf ("caw_init()   (us): %9u\n\r",j/k);				USART_txint_send();

	/* Experiment with different SCLK speeds */
	// Display divisor setup by spi2sdcard.c 
	printf ("spi2 divide code : %9u\n\r",(SPI2_CR1 >> 3) & 0x7);		USART_txint_send();
	// Change the divisor and dispaly new value
//	if ( (k = spi2_sclk_set_divisor_code(6)) != 0) {printf("divide code err: %u\n\r",k);USART_txint_send();}
//	printf ("spi2 divide code : %9u\n\r",spi2_sclk_get_divisor_code()); 	USART_txint_send();	

	/* LED flashing is a quick way for a myopic op to know if it is working */
	if ( nSpi2 != 0)		// Did it initialize correctly?
	{ // Here no.  Show the return error number.
		printf ("caw_init() return code = 0x%x(%d)\n\r",nSpi2, nSpi2); USART_txint_send();
		nTogglerate /= 15;	// ==+> Flash fast if an error <====
	}
	else
	{
	/* Shameless hack to make it work on POD without changing routines for Olimex */
#ifdef OLIMEX
	sdcard_csd_print_POD = 0;	// 0 = USART2;
	sdcard_cid_print_POD = 0;	// 0 = USART2;
#else
	sdcard_csd_print_POD = 1;	// 1 = USART1;
	sdcard_cid_print_POD = 1;	// 1 = USART1;_
#endif
	
		/* List the CSD fields */
		sdcard_csd_print(caw_csd_data);	// Print the extracted CSD fields
		unsigned int sd_size = sdcard_csd_memory_size (caw_csd_data);
		unsigned int sd_block_size = sdcard_csd_block_size (caw_csd_data);
		printf ("Old POD size with bug--SD Card size: number blocks %u, of block size: %u\n\r",sd_size,sd_block_size);	
		printf ("Correct size PC sees---SD Card size: number blocks %u, of block size: %u\n\r",(sd_size + 1024),sd_block_size);	

		USART_txint_send();
	
		/* List CID fields */
		sdcard_cid_print(caw_cid_data);	// Print the extracted CSD fields

//		/* caw_test */
//		#include "sdlog.h"
//		sdlog_init();
//		printf("\n\r"); USART_txint_send();
//		printf("sdlog_debug_get_card_size_in_blocks() returns %u\n\r", 
//		sdlog_debug_get_card_size_in_blocks()); USART_txint_send();
	}

	USART_txint_puts("\n\rNOTE: very rudimentary type-in editing \n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'p xxxx<ret>' to read block number xxxx into buffer and display \n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'w xxxx<ret>' to write block number xxxx out of buffer \n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'e xxxx<ret>' to erase block number xxxx\n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'c xxxx|yyyy <ret>' to compare block x with block y \n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'd xxxx|nnnn <ret>' to dump n blocks starting with block x \n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'a xxxx<ret>' to write blocks 0 to (xxxx-1)\n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'b xxxx<ret>' to read blocks 0 to (xxxx-1)\n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'y xxxx<ret>' 'a xxxx<ret>' followed by 'b xxxx<ret>' loop\n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'Y<ret>' to terminate a 'y' loop\n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'f xxxx<ret>' fill the buffer with xxxx\n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'g xxxx<ret>' fill the buffer with pattern for block num xxxx\n\r");	USART_txint_send();
	USART_txint_puts("Enter: 's<ret>' print R2 status\n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'u<ret>' print acmd13 status\n\r");	USART_txint_send();
	USART_txint_puts("Enter: 'z<ret>' print crc-ccitt of buffer and duration\n\r");	USART_txint_send();

/* ========================== Endless loop ================================================ */
	/* Blink the LED on the board so that we can see it is alive */
	while (1==1) 
	{
		int cksum;
		
		/*LED blink rate timing */
		if ( i++ > nTogglerate)
		{
#ifdef OLIMEX	/* Select Olimex LED, or POD LEDs */
			toggle_led();	// Olimex (one LED)
#else
			toggle_4leds();	// POD (four LEDs)
#endif
			i = 0;
		}
		
		if(ptrwf)
			(*ptrwf)();
		
		/* The following is partly a demonstration of the different ways to handle USART input */
		lb = USART_rxint_getlineboth();		// Get both char count and pointer
			/* Check if a line is ready.  Either 'if' could be used */
//			if (lb.ct > 0)				// Check if we have a completed line
		if (lb.p > (char*)0)				// Check if we have a completed line
		{ // Here we have a pointer to the line and a char count
			USART_txint_puts(lb.p);		// Echo back the line just received
			USART_txint_puts ("\n");		// Add line feed to make things look nice
			USART_txint_send();			// Start the line buffer sending
			systick0 = SYSTICK_getcount32();	// Get 32 bit time for the first case below
			ullsystick0 = SYSTICK_getcount64();	// For long elapsed time (down to the very last tick)

			switch (*lb.p)

			{
			/* When the operator types 'p xxxxxxx' <ret> read and display the block number  */
			case 'p':
				uiBlockNum = dumbasctoint (lb.p);
				systick1 = SYSTICK_getcount32(); // Get 32 bit before starting card setup
				k = sdcard_read(uiBlockNum, caw_realdata);
				cksum = sdc_get_crc();
				systick0 = SYSTICK_getcount32();// Time of completion
		
				/* Check that read return is OK */	
				printf("Status is \"0x%04x\"\r\n", sdcard_ll_cmd(CMD13, 0, NULL));
				if ( k != 0 )
				{ // Here failed...maybe
					printf ("\n\rsdcard_read return: 0x%x/%d\n\r",k,k);  USART_txint_send();
				}
				else
				{
					/* Time the read and print in microseconds */
					j = (systick1-systick0);	// System ticks during the read
					k = sysclk_freq/1000000;	// Scale system clock to microseconds
					printf ("/* Block number: %u, ",uiBlockNum);
					printf ("checksum is 0x%04x/0x%04x, ", cksum, crc_ccitt(0, (unsigned char *)caw_realdata, SDC_DATA_SIZE));
					printf ("sdcard_read duration (us): %9u */\n\r",j/k);	USART_txint_send();
					print_cawreal (caw_realdata); 	USART_txint_send();	// Display block
				}
				break;
			/* When the operator types 'w xxxxxxx' <ret> write the block and display the block number  */
			case 'w':
				uiBlockNum = dumbasctoint (lb.p);
				systick1 = SYSTICK_getcount32(); // Get 32 bit before starting card setup
				k = sdcard_write(uiBlockNum, caw_realdata);
				systick0 = SYSTICK_getcount32();// Time of completion
		
				/* Check that write return is OK */	
				printf("Status is \"0x%04x\"\r\n", sdcard_ll_cmd(CMD13, 0, NULL));
				if ( k != 0 )
				{ // Here failed...maybe
					printf ("\n\rsdcard_write return: 0x%x\n\r",k);  USART_txint_send();
				}

				/* Time the read and print in microseconds */
				j = (systick1-systick0);	// System ticks during the read
				k = sysclk_freq/1000000;	// Scale system clock to microseconds


				j = (systick1-systick0);	// System ticks during the read
				k = sysclk_freq/1000000;	// Scale system clock to microseconds
				printf ("Block number: %u    ",uiBlockNum);
				printf ("sdcard_write duration (us): %9u\n\r",j/k);	USART_txint_send();
				break;

			/* 'e xxxxxx' <ret> erases 1 block at block number xxxxx */
			case 'e':
				uiBlockNum = dumbasctoint (lb.p);
				systick1 = SYSTICK_getcount32(); // Get 32 bit before starting card setup
				k = sdcard_erase(uiBlockNum, 1);
				systick0 = SYSTICK_getcount32();// Time of completion
		
				/* Check that erase return is OK */	
				printf("Status is \"0x%04x\"\r\n", sdcard_ll_cmd(CMD13, 0, NULL));
				if ( k != 0 )
				{ // Here failed...maybe
					printf ("\n\rsdcard_erase return: %u\n\r",k);  USART_txint_send();
				}

				/* Time the erase and print in microseconds */
				j = (systick1-systick0);	// System ticks during the read
				k = sysclk_freq/1000000;	// Scale system clock to microseconds


				j = (systick1-systick0);	// System ticks during the read
				k = sysclk_freq/1000000;	// Scale system clock to microseconds
				printf ("Block number: %u    ",uiBlockNum);
				printf ("sdcard_erase duration (us): %9u\n\r",j/k);	USART_txint_send();
				break;
				
			/* When the operator types 'c xxxxxxx yyyyyyy' <ret> read and compare the two blocks  */
			case 'c':
				j = dumb1st (lb.p);
				if (j < 0) break;	// Break on a bad input
				k = dumb2nd (lb.p);
				if (k < 0) break;	// Break on a bad input
				printf ("Compare block %u with block %u\n\r",j,k);	USART_txint_send();	
				sdcard_read(j, caw_realdata);
				sdcard_read(k, caw_realdataX);
				l = 0;
				for ( i = 0; i <SDC_DATA_SIZE; i++)
				{
					if (caw_realdata[i] != caw_realdataX[i])
					{
						printf ("%3u %02x %02x\n\r",i,caw_realdata[i],caw_realdataX[i]);USART_txint_send();
						l = 1;	// Set flag
					}
				}
				if (l == 0 )
					{printf ("No differences\n\r");USART_txint_send();}
				break;
				
			/* When the operator types 'd xxx nnn' <ret> dump from block xxx to block (xxx+nnn-1)  */
			case 'd':
				j = dumb1st (lb.p);
				if (j < 0) break;	// Break on a bad input
				k = dumb2nd (lb.p);
				if (k < 0) break;	// Break on a bad input
				printf ("Dump %u blocks starting at block %u\n\r",k,j);	USART_txint_send();
				for(l=j; l<j+k; l+=1)
				{
					int err;
					
					err = sdcard_read(l, caw_realdata);
					printf("# blk %d, err 0x%x, cksum 0x%04x\n\r", l, err, sdc_get_crc());
					if(!err)
						dump_cawreal(caw_realdata);
				}
				break;

			/* Command 'a xxxx <ret>' writes from block 0 to (xxx-1) */
			case 'a':
				uiMax = 0; ulAve = 0;
				printf ("pass.w.blocknum.max(usec).ave(usec)\n\r");USART_txint_send();
				NEXT_STATE(a_funct);
				data_mask = 0;
				caw_arg = dumbasctoint(lb.p + 1);
				caw_flag = (1==0);
				break;
				
			/* Command 'b xxxx <ret>' reads and checks from block 0 to (xxx-1) */
			case 'b':
				NEXT_STATE(b_funct);
				data_mask = 0;
				caw_arg = dumbasctoint(lb.p + 1);
				caw_flag = (1==0);
				break;
			
			/* Command 'y xxxx <ret>' is 'a xxxx<ret>' followed by 'b xxxx<ret>' loop */
			case 'y':
				uiMax = 0; ulAve = 0;
				printf ("pass.w.blocknum.max(usec).ave(usec)\n\r");USART_txint_send();
				NEXT_STATE(a_funct);
				pass_no = 0;
				data_mask = 0;
				caw_arg = dumbasctoint(lb.p + 1);
				caw_flag = (1==1);
				break;
				
			case 'Y':				/* Abort a 'y' command */
				ptrwf = 0;
				break;
				
			/* Command 'f xxxx <ret>' fills the buffer with the longword xxxx */
			case 'f':
			{
				unsigned long *p = (unsigned long *)(caw_realdata + 0);
				unsigned long *q = (unsigned long *)(caw_realdata + SDC_DATA_SIZE);

				k = dumbasctoint(lb.p + 1);
				while(p < q)
					*p++ = k;
				break;
			}
			
			/* 'g xxxxxx" <ret> fills the buffer with the "pattern" for block xxxxxx. */
			case 'g':
				uiBlockNum = dumbasctoint (lb.p);
				for(j=0; j<SDC_DATA_SIZE; j+=16)
				{
					sprintf(caw_tmpdata, "%15d\n", uiBlockNum);
					memcpy(caw_realdata+j, caw_tmpdata, 16);
				}
				print_cawreal (caw_realdata); 	USART_txint_send();	// Display block
				break;

			/* Print status */
			case 's':
				printf("Status is \"0x%04x\"\r\n", sdcard_ll_cmd(CMD13, 0, NULL));
				break;
				
			case 'u':
			{
				unsigned char status_buf[SDC_STATUS_SIZE];
				
				sdcard_ll_cmd(ACMD13, 0, (void *)status_buf);
				for(k=0; k<SDC_STATUS_SIZE; k+=1)
				{
					if((k%16) == 0)
						printf("%4d> ", k);
					printf("%02x ", status_buf[k]);
					if((k%16) == 15)
						printf("\r\n");
					USART_txint_send();
				}
				break;
			}
			
			/* 'z' <ret> calculates the checksum on the buffer (as well as how long it took) */
			case 'z':
				systick1 = SYSTICK_getcount32(); // Get 32 bit before starting card setup
				cksum = crc_ccitt(0, (unsigned char *)caw_realdata, SDC_DATA_SIZE);
				systick0 = SYSTICK_getcount32();// Time of completion
		
				/* Time the erase and print in microseconds */
				j = (systick1-systick0);	// System ticks during the checksum
				k = sysclk_freq/1000000;	// Scale system clock to microseconds
				printf ("crc-ccitt is 0x%04x, duration (us): %u\n\r", cksum, j/k); USART_txint_send();
				break;
		
			/* Default... */
			default:
				printf("Huh?\r\n");
				break;
			}

		USART_txint_send();
		}
	}
	return 0;
}

/*****************************************************************************************/
static unsigned long block_no, block_max;
static unsigned long rand;
#if (1==1)
static const unsigned long rand_seed = 0xdeadbeef;	/* My favorite 32-bit hex constant */
static const unsigned long rand_incr = 982451653;	/* The 50-million-th Mersenne prime */
#else
static const unsigned long rand_seed = 0;
static const unsigned long rand_incr = 1;
#endif
static int error_count, error_count_copy;
static int err;

void a_funct(void)
{
	block_no = 0;
	block_max = caw_arg;
	rand = rand_seed;
	ulAve = uiMax = 0;
	error_count = error_count_copy = 0;
	NEXT_STATE(a_00);
	
	printf("%u.w.%07u.%06u.%06u> ", pass_no, block_no, uiMax/(sysclk_freq/1000000),uiSysDur/(sysclk_freq/1000000) );
	USART_txint_send();
}

void a_00(void)
{
	unsigned long *p = (unsigned long *)(caw_realdata + 0);
	unsigned long *q = (unsigned long *)(caw_realdata + SDC_DATA_SIZE);
	
	while(p < q)
	{
//		*p++ = rand ^ data_mask;
*p++ = 0;
		rand += rand_incr;
	}
	uiSys0 = SYSTICK_getcount32();	// Get 32 bit systick time at beginning of write
	
	err = sdcard_write(block_no, caw_realdata);
	if(err)
	{
		int n = error_count - error_count_copy;
		if(n < 8)
		{
			printf("[%x]", err);
			USART_txint_send();
		}
		error_count += 1;
	}

	NEXT_STATE(a_01);
}

void a_01(void)
{
	const int bpp = 1000;
	
	/* Get time write completed and compute duration for write */
	uiSysDur = uiSys0 - SYSTICK_getcount32();
	
	/* Find the longest duration write */
	if  (uiSysDur > uiMax) uiMax = uiSysDur;
	ulAve += uiSysDur;
		
	block_no += 1;
	
	if((block_no % bpp) == 0)
	{
		if((block_no % (bpp * 10)) != 0)
		{
			printf((error_count == error_count_copy) ? "." : "*");
		}
		else
		{
			sdcard_retries_t rt;
			int i;
			
			sdcard_retries_get(&rt);
			
			for(i=0; i<RETRY_LIMIT; i+=1)
				printf(" %u ", rt.retries[i]);
			
			if(error_count)
			{
				printf(" // w/ %d errors!", error_count);
				error_count = 0;
			}
			printf("\r\n%u.w.%07u.%06u.%06u> ", pass_no, block_no, 
						uiMax/(sysclk_freq/1000000),
						(ulAve/(bpp*10))/(sysclk_freq/1000000) );
			ulAve = uiMax = 0;
		}

		error_count_copy = error_count;
		USART_txint_send();
	}

	if(block_no < block_max)
		NEXT_STATE(a_00);
	else
	{
		printf ("\n\r");
		USART_txint_send();
		
		if(caw_flag)
			NEXT_STATE(b_funct);
		else
			ptrwf = 0;
	}
}

void b_funct(void)
{
	block_no = 0;
	block_max = caw_arg;
	error_count = error_count_copy = 0;
	rand = rand_seed;	
	NEXT_STATE(b_00);
	
	printf("%u.r.%07u> ", pass_no, block_no);
	USART_txint_send();
}

void b_00(void)
{
	err = sdcard_read(block_no, caw_realdata);
	if(err)
	{
		int n = error_count - error_count_copy;
		if(n < 8)
		{
			printf("[%x]", err);
			USART_txint_send();
		}
		error_count += 1;
	}

	NEXT_STATE(b_01);
}

void b_01(void)
{
	unsigned long *p = (unsigned long *)(caw_realdata + 0);
	unsigned long *q = (unsigned long *)(caw_realdata + SDC_DATA_SIZE);
	const int bpp = 1000;
	int flag = (1==1);
	
	while(p < q)
	{
		if(*p++ != (rand ^ data_mask) && flag)
			error_count += 1;
		rand += rand_incr;
	}

	block_no += 1;
	
	if((block_no % bpp) == 0)
	{
		if((block_no % (bpp * 10)) != 0)
		{
			printf((error_count == error_count_copy) ? "." : "*");
		}
		else
		{
			sdcard_retries_t rt;
			int i;
		
			sdcard_retries_get(&rt);
			
			printf("              ");
			for(i=0; i<RETRY_LIMIT; i+=1)
				printf(" %u ", rt.retries[i]);
		
			if(error_count)
			{
				printf(" // w/ %d errors!", error_count);
				error_count = 0;
			}
			printf("\r\n%u.r.%07u> ", pass_no, block_no);
		}

		error_count_copy = error_count;
		USART_txint_send();
	}

	if(block_no < block_max)
		NEXT_STATE(b_00);
	else
	{
		printf ("\n\r");
		USART_txint_send();
		
		pass_no += 1;
		data_mask ^= ~0;
		
		if(caw_flag)
			NEXT_STATE(a_funct);
		else
			ptrwf = 0;
	}
}

/*****************************************************************************************/
/* caw stuff                                                                             */
/*****************************************************************************************/
int caw_init(void)
{
	return sdcard_init(caw_cid_data, caw_csd_data);
}

/*****************************************************************************************/
/* Print out a block in hex.  16 per line                                                */
/*****************************************************************************************/
void print_cawreal(char * p)
{
	int i,j;
	
	for ( i = 0; i <SDC_DATA_SIZE; i += 16)
	{
		for (j = 0; j < 16; j++)
		{
			printf ("0x%02x, ",*(p+j));
		}
		printf (" /* ");
		for (j = 0; j < 16; j++)
		{
			if ( (*(p+j) >= 0x20) && (*(p+j) < 0x7f) )
				printf ("%c",(*(p+j) & 0x7f));
			else
				printf (".");
		}
		printf (" */\n\r");
		USART_txint_send();
		p += 16;
	}
}

/*****************************************************************************************/
/* Print out a block in hex.  16 per line                                                */
/*****************************************************************************************/
void dump_cawreal(char * p)
{
	int i,j;	

	for ( i = 0; i <SDC_DATA_SIZE; i += 16)
	{
		for (j = 0; j < 16; j++)
			printf ("%02x",*(p+j));
		printf ("\n\r");
		USART_txint_send();
		p += 16;
	}
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
	printf ("Input err: Put a '|' between the block numbers\n\r");	USART_txint_send();
		
	return -1;
}
int dumb2nd(char *p)
{
	while (*p++ != '|');	// Spin forward to separator
	return dumbasctoint(p);
}

