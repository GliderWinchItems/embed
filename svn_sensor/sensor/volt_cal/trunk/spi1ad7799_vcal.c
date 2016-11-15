/******************************************************************************
* File Name          : spi1ad7799_vcal.c
* Date First Issued  : 10/07/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : SPI1 routines for AD7799, multi-channel readings
*******************************************************************************/

#include "spi1ad7799_vcal.h"
#include "PODpinconfig.h"
#include "pinconfig_all.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/exti.h"

#define NULL	0

static char	spi1_vcal_xmt;			// SPI1 dummy byte sent when reading
static char	xmt_flag;			// Flag for ISR: 0 = xmit; not zero = rcv 
static int	spi1_vcal_cnt;			// SPI1 byte counter
static unsigned char 	*spi1_vcal_ptr;		// SPI1 buffer pointer
void 	(*spi1_vcal_exti2ptr)(void) = NULL;	// Low Level function call

/* APB2 bus frequency is used for setting up the divider for SPI1 */
extern unsigned int	pclk2_freq;	/*	SYSCLKX/APB2 divider	E.g. 36000000 	*/
/* AD7799 minimum SCLK pulse width is 100ns, i.e. 5 MHz.  Set bus divider to stay at or below this freq */

#define SCLKFREQ	5000000	/* Max frequency (Hz) to be used with AD7799 */

/******************************************************************************
 * void delay(uint32_t ticks);
 * @brief	: Timing loop using DTW counter
 * @param	: ticks = sysclock tick count
*******************************************************************************/
/* APB2 bus frequency is used for setting up the divider for SPI1 */
extern unsigned int	pclk2_freq;	/*	SYSCLKX/APB2 divider	E.g. 36000000 	*/

#define WAITDTW(tick)	while (( (int)tick ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 )
static void delay(uint32_t ticks)
{
	volatile uint32_t tend = *(volatile unsigned int *)0xE0001004 + ticks;
	WAITDTW(tend);
	return;
}

/* ----------------------- SPI1 function pointer stack ------------------------- */
#define SPI1STACKCT	8	// Pointer stack size
static void* stk[SPI1STACKCT];		// Pointer stack
static void** pstk = &stk[0];		// Point to next available 
/******************************************************************************
 * void spi1ad7799_vcal_push(void* pexti2 );
 * @brief	: Stack next function pointers
 * @param	: pexti2 = pointer to next function
*******************************************************************************/
void spi1ad7799_vcal_push(void* pexti2 )
{
	EXTI_IMR &= ~(1 << 6);	// External interrupt request from Line 6 is disabled
	*pstk++ = pexti2;	// Push address of function
if (pstk >= &stk[SPI1STACKCT]) {while(1==1);} // Trap 
	return;
}
/******************************************************************************
 * void spi1ad7799_vcal_stkcall(void);
 * @brief	: Pop address from stack and call function 
*******************************************************************************/
void spi1ad7799_vcal_stkcall(void)
{
	void (*x)(void);
	x = *--pstk;
if (pstk <= &stk[0]) {while(1==1);} // Trap 
	(*x)();		// Call function

	return;
}
/******************************************************************************
 * void spi1ad7799_vcal_DOUT_ext(void* pexti2);
 * @brief	: Configure AD7799 DOUT pin: for external interrupt
 * @param	: pexti2 = pointer to function to call when interrupt occurs
*******************************************************************************/
void spi1ad7799_vcal_DOUT_ext(void* pexti2)
{
/* When DOUT goes low at the end of a read or write, i.e. when the AD7799 goes
   from busy to not-busy, the interrupt is vectored to 'pexti2' where the next
   operation is initiaited. */

	spi1_vcal_exti2ptr = pexti2;	// Address of func to call upon interrupt

	EXTI_PR &= ~(1 << 6);	// Clear pending if something turned it on.

	/* Enable PA6 (ad7799 DOUT) to cause an interrupt. */
	EXTI_IMR |= (1 << 6);	// External interrupt request from Line 6 is enabled
	return;
}

const struct PINCONFIGALL test_exti     = {(volatile u32 *)GPIOE,  8, IN_PU    ,      0};

const struct PINCONFIGALL spi1_ncs1   = {(volatile u32 *)GPIOB, 10, OUT_PP   , MHZ_50};
const struct PINCONFIGALL spi1_ncs2   = {(volatile u32 *)GPIOA,  4, OUT_PP   , MHZ_50};
const struct PINCONFIGALL spi1_sck    = {(volatile u32 *)GPIOA,  5, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL spi1_so     = {(volatile u32 *)GPIOA,  6, IN_PU    ,      0};
const struct PINCONFIGALL spi1_si     = {(volatile u32 *)GPIOA,  7, OUT_AF_PP, MHZ_50};
/******************************************************************************
 * int spi1ad7799_vcal_init(void);
 *  @brief Initialize SPI for SD Card Adapter
 *  @return	: zero = success; not zero = failed
*******************************************************************************/
static void stk_trap(void) {while (1==1);}	// stack trap 

int spi1ad7799_vcal_init(void)
{
	unsigned int uiX,uiZ,uiM;	// Used in computing baud divisor
	int i;
	int err;

	/* Power switches turned on with stabilization delay in 'p1_initialization.c' */

	/* Set end of interrupt handling of read and write addresses */
	for (i = 0; i < SPI1STACKCT; i++) // Initialize addresses on stack
		stk[i] = &stk_trap;
	// stk[0] is set to trap; start at stk[1], so one-too-many pops causes trap. */
	pstk = &stk[1];

	/* Enable: SPI1 and  bus clocking for alternate function */
	RCC_APB2ENR |= ((RCC_APB2ENR_SPI1EN) | (RCC_APB2ENR_AFIOEN));

	err  = 0;
	err |=  pinconfig_all( (struct PINCONFIGALL *)&spi1_ncs1);	// /CS ad7799-1
	err |=  pinconfig_all( (struct PINCONFIGALL *)&spi1_ncs2);	// /CS ad7799-2
	err |=  pinconfig_all( (struct PINCONFIGALL *)&spi1_sck);	// Clock out AF
	err |=  pinconfig_all( (struct PINCONFIGALL *)&spi1_so);	// MISO
	err |=  pinconfig_all( (struct PINCONFIGALL *)&spi1_si);	// MISO

	/* PA6 is used to detect data ready when AD7799 is in continuous read mode. */
	/* JIC (since PA6 used as ext2 interrupt) set a pull-up on PA6. */
	AFIO_EXTICR2  &= ~(0xf << 8); 	// Select pin 6, port A (PA(x) = 0) for AD7799 DOUT pull-down interrupt
	EXTI_FTSR     |= (1 << 6);	// Falling Trigger for line 6

	/* Set and enable interrupt controller for EXTI */
	NVICIPR (NVIC_EXTI9_5_IRQ, SPI1_PRIORITY);	// Set interrupt priority SAME AS SPI1
	NVICISER(NVIC_EXTI9_5_IRQ);			// Enable interrupt controller for EXTI2

	/* Note: PB10	AD7799_1 /CS:gpio_out and PA4	AD7799_2 /CS:	gpio_out was setup by earlier call to PODgpiopins_Config() */
	
	/* Compute baud divisor - (uiM = 0 for divide 2 to uiM = 7 for divide by 256)*/
	uiX = (pclk2_freq / SCLKFREQ) + 1;
	uiM = 0; uiZ = 2;
	while ( uiZ < uiX){ uiM += 1; uiZ *= 2; } // Find divisor that exceeds uiX division ratio
	uiM += 1;
	if (uiM > 7) uiM = 7;	// Greater than 7 means the SCLK speed may be higher than spec.
uiM = 3; // Debug slowest rate

	/* SPI-CR1 (see p 693 Ref Manual) */
	//         (enable peripheral| baud divisor | master select | CK 1 when idle | phase  )
	SPI1_CR1 =       (1 << 6)    | (uiM << 3)   |   (1 << 2)    |    (1 << 1)    |  0x01    ;

	/* Enable AD7799_1; disable AD7799_2 (if present) */
	GPIOB_BRR  = (1<<10);		// Enable  AD7799_1 (/CS pin (PB10) low)
	GPIOA_BSRR = (1<<4);		// Disable AD7799_2 (/CS pin (PA4) high)

	/* SPI-CR2 use default, no interrupt masks enabled at this point */

	/* Set and enable interrupt controller for SPI1 */
	NVICIPR (NVIC_SPI1_IRQ, SPI1_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_SPI1_IRQ);			// Enable interrupt controller for SPI1

	return err;
}
/******************************************************************************
 * unsigned short ad7799_1_ready(void);
 * @brief	: Test ad7799_1 data out line low (data is ready)
 * @return	: Zero = ready; not-zero = data line still high
*******************************************************************************/
unsigned short ad7799_vcal_1_ready(void)
{
	return (GPIOB_IDR & GPIO10);	// Return bit 10 of port B
}
/******************************************************************************
 * void spi1_vcal_read (unsigned char *p, int count, char xmit);
 * @brief	: read 'count' bytes, into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
 * @param	: char xmit  = outbound char during spi cycle
*******************************************************************************/
void spi1_vcal_read (unsigned char *p, int count, char xmit)
{
	xmt_flag = 1;			// Show ISR this is a rcv sequence
	spi1_vcal_ptr = p;		// Set pointer for interrupt handler to store incoming data
	spi1_vcal_cnt = count;		// Set byte count for interrupt handler
	spi1_vcal_xmt = SPI1_DR;	// Clear last recieve buffer full flag
	SPI1_DR  = xmit;		// Start 1st xmit dummy byte to get 1st receive byte
	spi1_vcal_xmt = xmit;		// Save dummy byte that is sent for subsequent reads
	SPI1_CR2 |= (SPI_CR2_RXNEIE);	// Turn on receive buffer loaded interrupt enable
	return;
}
/******************************************************************************
 * void spi1_vcal_write (unsigned char *p, int count);
 * @brief	: read 'count' bytes, into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
*******************************************************************************/
void spi1_vcal_write (unsigned char *p, int count)
{
	/* If the programmer bozo didn't check for busy we have no choice but to loop */
	spi1_vcal_ptr = p;		// Set pointer for interrupt handler
	spi1_vcal_cnt = count;		// Set byte count for interrupt handler
	xmt_flag = 0;			// Show ISR this is a xmit sequence
	SPI1_CR2 |= (SPI_CR2_TXEIE);	// Turn on xmit buffer empty interrupt enable
	/* At this point the xmit buffer interrupt will pick up TXE interrupt and send the first byte */
	return;
}
/******************************************************************************
 * char spi1_vcal_busy (void);
 * @brief	: Check for buffer bytes remaining and spi1 busy bit 
 * @return	: not zero means busy
*******************************************************************************/
char spi1_vcal_busy (void)
{
	return ( spi1_vcal_cnt | ((SPI1_SR & SPI_SR_BSY) != 0) );
}
/******************************************************************************
 * void spi1_vcal_ad7799_vcal_reset_noint (void);
 * @brief	: Non-interrupting (test) for reseting the AD7799
*******************************************************************************/
void spi1_vcal_ad7799_vcal_reset_noint (void)
{
	__attribute__((__unused__))int dummy;
	SPI1_DR  = 0xff;		// Start xmit
	while ( (SPI1_SR & SPI_SR_TXE) == 0 );// Wait for TX buffer to be empty
	SPI1_DR  = 0xff;		// Start xmit
	while ( (SPI1_SR & SPI_SR_TXE) == 0 );// Wait for TX buffer to be empty
	SPI1_DR  = 0xff;		// Start xmit
	while ( (SPI1_SR & SPI_SR_TXE) == 0 );// Wait for TX buffer to be empty
	SPI1_DR  = 0xff;		// Start xmit
	while ( (SPI1_SR & SPI_SR_TXE) == 0 );// Wait for TX buffer to be empty
	while ( (SPI1_SR & SPI_SR_BSY) != 0 );
	while ( (SPI1_SR & SPI_SR_RXNE) == 1 )	dummy = SPI1_DR;// Clear read buffer

	return;
}
/*#######################################################################################
 * SPI1 routine
 *####################################################################################### */
void SPI1_IRQHandler_vcal(void)
{
	__attribute__((__unused__))unsigned int dummy;

	if (xmt_flag == 0)
	{
		if ( ((SPI1_CR2 & SPI_CR2_TXEIE) != 0) && ((SPI1_SR & SPI_SR_TXE) != 0) )	
		{ /* Here, this is a xmit sequence being executed AND transmit buffer empty flag is on */
			if (spi1_vcal_cnt <= 0)	// Have we exhausted the count?
			{ /* Here, yes, the last byte was loaded into xmit buffer the last time the ISR executed */
				SPI1_CR2 &= ~SPI_CR2_TXEIE;	// Turn off xmit buffer empty interrupt enable
				dummy = SPI1_DR;		// Clear receive buffer full flag
				SPI1_CR2 |=  SPI_CR2_RXNEIE;	// Enable receive buffer full interrupt		
				return;
			}
			else
			{ /* Here, the byte count shows there is more to do */
				SPI1_DR = *spi1_vcal_ptr++;	// Load next byte for xmit
				spi1_vcal_cnt -= 1; 		// Decrement byte count
				return;
			}
		}
		else
		{ /* Here, the expected interrupt is a receive buffer full, which was loaded from the last byte of the xmit sequence */
			if ( ((SPI1_CR2 & SPI_CR2_RXNEIE) != 0) && ((SPI1_SR & SPI_SR_RXNE) != 0) ) // Check for bogus interrupt
			{ /* Here, valid interrupt.  Xmit sequence should be completed, including the last byte sending finished */
				SPI1_CR2 &= ~SPI_CR2_RXNEIE;	// Turn off RXE interrupt enable
				spi1ad7799_vcal_stkcall();	// Call function from stack
			}
		}
	}
	else
	{ /* Here, a receive sequence being executed */
		/* Here, we are doing a receive */
		if ( ((SPI1_CR2 & SPI_CR2_RXNEIE) != 0) && ((SPI1_SR & SPI_SR_RXNE) != 0) ) // Check for bogus interrupt
		{ /* Here, valid interrupt. */
			*spi1_vcal_ptr++ = SPI1_DR;		// Get byte that was read
			spi1_vcal_cnt -= 1;	 		// Decrement byte count
			if (spi1_vcal_cnt <= 0)		// Have we exhausted the count?
			{ /* Here, yes, this byte is the reponse from the last dummy byte transmitted */ 
				SPI1_CR2 &= ~SPI_CR2_RXNEIE;	// Turn off RXE interrupt enable
				spi1ad7799_vcal_stkcall();	// Call function from stack
				return;
			}
			else
			{
				SPI1_DR = spi1_vcal_xmt;	// Load dummy byte to start next spi cycle
			}
		}
	}
	return;
}
/* 
Note 1: At this point the last byte in the shift register has been sent therefore loading the receiver buffer.  A call to a routine to do the next step 
is made using a pointer to the function.  The function called might call spi1_vcal_read, or spi1_vcal_write to start the next operation.  The 'busy' flag should be
off since the last char was completed.  Consequently, the routine setting up the next step should be have to loop waiting for the busy.
*/
/*#######################################################################################
 * void EXTI2_IRQHandler(void)
 *####################################################################################### */
void EXTI9_5_IRQHandler(void)
{
	
	if ((EXTI_PR & (1<<6)) != 0)
	{ // Disable PA6 (ad7799 DOUT) interrupts.
		EXTI_IMR &= (1 << 6);	// External interrupt request from Line 6 is masked
		EXTI_PR  &= (1 << 6);	// Pending flag is reset
		/* Call other routines if an address is set up */
		if (spi1_vcal_exti2ptr != 0)	// Skip if no address 
			(*spi1_vcal_exti2ptr)();	// Go do something
	}

	return;
}

