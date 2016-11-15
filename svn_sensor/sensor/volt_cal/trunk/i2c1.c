/******************************************************************************
* File Name          : i2c1.c
* Date First Issued  : 10/27/2015
* Board              : 
* Description        : I2C for F103 (and others?)
*******************************************************************************/

#include "i2c1.h"
#include <stdio.h>
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/rcc.h"
#include "libopenstm32/i2c.h"
#include "common.h"
#include "pinconfig_all.h"

/* SYSCLK frequency is used to time delays. */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')
extern unsigned int	pclk1_freq;	// APB1 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* Circular buffering bytes to be sent. */
static uint8_t buf[IC2BUFFERSIZE];
static uint8_t* phead;		// Pointer for adding bytes
static uint8_t* ptail;		// Pointer for taking bytes

uint16_t i2cseqbusy;	// Not zero = Read or Write sequence is busy
void 	(*i2c1_ev_ptr)(void) = 0;

/* ----------------------- I2C1 function pointer stack ------------------------- */
#define I2C1STACKCT	8	// Pointer stack size
static void* stk[I2C1STACKCT];		// Pointer stack
static void** pstk = &stk[0];		// Point to next available 
static void stk_trap(void) {while (1==1);}	// stack trap 
/******************************************************************************
 * void i2c1_vcal_push(void* pnexti2 );
 * @brief	: Stack next function pointers
 * @param	: pnexti2 = pointer to next function
*******************************************************************************/
void i2c1_vcal_push(void* pnexti2 )
{
	*pstk++ = pnexti2;	// Push address of function
if (pstk >= &stk[I2C1STACKCT]) {while(1==1);} // Trap 
	return;
}
/******************************************************************************
 * void i2c1_vcal_stkcall(void);
 * @brief	: Pop address from stack and call function 
*******************************************************************************/
void i2c1_vcal_stkcall(void)
{
	void (*x)(void);
	x = *--pstk;
if (pstk <= &stk[0]) {while(1==1);} // Trap 
	(*x)();		// Call function

	return;
}

/* Note: Open Drain mode for I2C is faux open drain as Vdd FET remains connected. */
const struct PINCONFIGALL pin_scl = {(volatile u32 *)GPIOB, 6, OUT_AF_OD, MHZ_50};
const struct PINCONFIGALL pin_sda = {(volatile u32 *)GPIOB, 7, OUT_AF_OD, MHZ_50};
/******************************************************************************
 * void i2c1_vcal_init(uint32_t sclclockrate);
 * @brief	: Initialize i2c1 for standard mode I2C
 * @param	: sclclockrate = SCL rate (Hz)
*******************************************************************************/
void i2c1_vcal_init(uint32_t sclclockrate)
{
	int i;
	uint32_t tmp;

	/* Pointers to circular buffer. */
	phead  = &buf[0];
	ptail = &buf[0];

	i2cseqbusy = 0;		// Set sequence flag not busy

	/* Set end of interrupt handling of read and write addresses */
	for (i = 0; i < I2C1STACKCT; i++) // Initialize addresses on stack
		stk[i] = &stk_trap;
	// stk[0] is set to trap; start at stk[1], so one-too-many pops causes trap. */
	pstk = &stk[1];

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= (1 <<  0);	// RCC_APB2ENR_AFIOEN: Enable Alternate function clocking
	RCC_APB1ENR |= (1 << 21);	// RCC_APB1ENR_I2C1EN: Enable clocking for I2C1

	/*  Setup  pins  */
	pinconfig_all((struct PINCONFIGALL *)&pin_scl);
	pinconfig_all((struct PINCONFIGALL *)&pin_sda);

	I2C1_CR2   = (pclk1_freq/1000000);		// Peripheral clock frequency (MHz)
	I2C1_TRISE = (pclk1_freq/1000000) + 1;		// Max rise time (above + 1)

	//         Standard mode |  Thigh = Tlow |  Clock rate divider
	tmp = (pclk1_freq/(sclclockrate*2));
	if (tmp > 4095) tmp = 4095; 	// Counter is 12 bits, so limit slowest speed
	I2C1_CCR   = ( (0 << 15) | (0 << 14) | (tmp << 0) );

	/* Enable NVIC for: Event and Error interrupt  */
	NVICIPR (NVIC_I2C1_EV_IRQ, NVIC_I2C1_EV_IRQ_PRIORITY );
	NVICISER(NVIC_I2C1_EV_IRQ);	

	I2C1_CR1 |= (1 << 0); 	// Bit 0 PE: Peripheral enable
	
	return;
}
/******************************************************************************
 * static uint8_t* advptr(uint8_t* p);
 * @brief	: Advance pointer in circular buffer
 * @param	: p = pointer to be advanced
 * @return	: Advanced pointer
*******************************************************************************/
static uint8_t* advptr(uint8_t* p)
{
	p += 1;
	if (p >= &buf[IC2BUFFERSIZE]) p = &buf[0];
	return p;
}
/******************************************************************************
 * uint16_t i2c1_vcal_putbuf(uint8_t* p, uint16_t count);
 * @brief	: Load bytes into circular buffer
 * @param	: p = pointer to bytes to be loaded
 * @param	: count = number of bytes to load
 * @return	: count of number loaded
*******************************************************************************/
uint16_t i2c1_vcal_putbuf(uint8_t* p, uint16_t count)
{
	int i;
	uint16_t ctr = 0;	// Count number of bytes loaded
	uint8_t* ptmp;		
	for (i = 0; i < count; i++)
	{
		ptmp = advptr(phead);	// Advance pointer
		if (ptmp == ptail)	// Will we overrun ptail?
		{ // Here, yes.
			return ctr;	// Return count loaded
		}
		*phead = *p++;	// Store byte; advance input ptr
		phead = ptmp;	// Update phead
		ctr += 1;	// Count bytes loaded
	}
	return ctr;
}
/******************************************************************************
 * uint8_t i2c1_vcal_bufempty(void);
 * @brief	: Check if buffer is empty (all bytes sent)
 * @return	: 0 = empty; not zero = buf still has bytes to be sent
*******************************************************************************/
uint8_t i2c1_vcal_bufempty(void)
{
	return (phead - ptail);
}
/******************************************************************************
 * static uint8_t* getbuf(void);
 * @brief	: Get byte from buffer
 * @return	: NULL = no bytes ready; pointer to byte
*******************************************************************************/
static uint8_t* getbuf(void)
{
	uint8_t* ptmp;		
	if (phead == ptail) return NULL; // Return no bytes available
	ptmp = ptail;		// Save for return
	ptail = advptr(ptail);	// Advance pointer
	return ptmp;
}
/******************************************************************************
 * void i2c1_vcal_start (uint16_t address);
 * @brief	: Address unit and start continuous write stream
 * @param	: address = I2C bus address of unit (NOT shifted for LSB R/W bit)
*******************************************************************************/
uint32_t dbgctr;
static uint8_t last;	// Byte to send
static uint16_t addr;	// Saved address
static uint16_t delayctr = 0;
// -------------------------------------------------------------------------------------
static void i2c1_vcal_repeat(void)
{ // Xmit buffer empty; send data byte
	__attribute__((__unused__))unsigned int dummy;
	uint8_t* pbuf;
	dummy = I2C1_SR1;
	dummy = I2C1_SR2;
	i2c1_vcal_push(&i2c1_vcal_repeat); // Next operation after interrupt

	if (delayctr == 0)
	{
		pbuf = getbuf();	// Get next byte to send
		if (pbuf != NULL) 	// Check if no new bytes
		{ // New 'last' byte
/* Since we only 'write' the R/W bit is used to distinguish between
   new data to be sent and a delay count of the last data byte. */
			if ((*pbuf & 0x02) == 0)
			{ // Here data byte
				last = *pbuf;	// Send new byte
//				delayctr = 1;	// Insert delay
			}
			else
			{ // Here a delay count
				delayctr = (*pbuf >> 2);
			}
		}
	}
	else
	{ // Here, delay counter still active
		delayctr -= 1;	// Count down
	}
	I2C1_DR = last;		// Load byte to be sent
dbgctr += 1;
	return;
}
// -------------------------------------------------------------------------------------
static void i2c1_vcal_write1(void)
{ // START completed
	__attribute__((__unused__))unsigned int dummy;
	i2c1_vcal_push(&i2c1_vcal_repeat);// Next operation after interrupt
	I2C1_CR1 &= ~(1 << 8);		// I2C_CR1_START Clear START generation
	dummy = I2C1_SR1;		// First step to clear START gen flag
	I2C1_CR1 |= (1 << 10);		// Bit 10 ACK: Acknowledge Enable
	I2C1_DR   = (addr << 1);	// Load address to be sent + R/W bit; clear START flag
	I2C1_CR2 |= (1 << 10);		// Bit 10 ITBUFEN: xmit buff empty interrupt enable
//$	I2C1_CR2 &= ~(1 << 9);		// Bit 9 ITEVTEN: Event interrupt enable
	return;
}
// -------------------------------------------------------------------------------------
void i2c1_vcal_start (uint16_t address)
{
	addr = address;	// Save for later interrupt usage.
	i2cseqbusy = 1;		// Set sequence flag busy
	i2c1_vcal_push(&i2c1_vcal_write1); // Next operation after interrupt
	I2C1_CR2 |= (1 << 9);	// Bit 9 ITEVTEN: Event interrupt enable
	I2C1_CR1 |= (1 << 8);	// Bit 8 START: Start generation
	return;
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void I2C1_EV_IRQHandler_vcal(void)
{
	__attribute__((__unused__))unsigned int dummy;
	i2c1_vcal_stkcall();	// Call function from stack
	dummy = I2C1_SR1;
	return;
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void I2C1_ER_IRQHandler_vcal(void)
{

}

