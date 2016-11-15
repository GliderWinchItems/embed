/******************************************************************************
* File Name          : hd44780.c
* Date First Issued  : 10/30/2015
* Board              : 
* Description        : LCD manipulation via PCF8574 I2C to parallel
*******************************************************************************/

#include "i2c1.h"
#include "hd44780.h"
#include "DTW_counter.h"

#define LCDLINELENGTH	16	// Number of positions on one LCD line
#define PCF8574ADDR	0x27	// Address of single unit on I2C bus
#define I2C1SCLKRATE	9000	// SCL clock rate (Hz) (about 1 ms per byte)



static void waitLCD(int32_t ticks);

/* SYSCLK frequency is used to time delays. */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/*
Bit of data byte sent to PCF8574
| LCD pin
0 RS Register Select
1 R/W Read NotWrite
2 E Clock
3 A (LED backlight) 1 = ON, 0 = OFF
4 D4
5 D5
6 D6
7 D7 MSB

RS and R/W bits
0 0 - Write command register
0 1 - Write data register
1 0 - Read status register
1 1 - Read data register

*/
extern volatile uint16_t i2cseqbusy;	// Not zero = Read or Write sequence is busy

static uint8_t pinA;	// Pin "A": 0x08 is bit for Backlight LED; bit on = lighted

/* ----------------------------------------------------------------------------- 
 * void timedelay (u32 ticks);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 ------------------------------------------------------------------------------- */
#define WAITDTW(tick)	while (( (int)tick ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 )
static void timedelay (unsigned int ticks)
{
	u32 t1 = ticks + DTWTIME; // DWT_CYCNT
	WAITDTW(t1);
	return;
}
/******************************************************************************
 * static void loadI2C(uint8_t* p, uint16_t count, uint32_t loop);
 * @brief	: load data into I2C buffer 
 * @param	: p = pointer to bytes
 * @param	: count = number to load
 * @param	: loop: 0 = don't loop if buffer full; not zero = loop until buff free
*******************************************************************************/
static void loadI2C(uint8_t* p, uint16_t count, uint32_t loop)
{
	uint16_t num;
	do
	{
		num = i2c1_vcal_putbuf(p, count);
		count = count - num;
	} while ((count != 0) && (loop != 0));
	return;
}
/******************************************************************************
 * static void sendLCD4(uint8_t data);
 * @brief	: Execute sequence to init the LCD
 * @param	: data = byte to be sent (in 4 bit mode)
*******************************************************************************/
static void sendLCD4(uint8_t data)
{
	uint8_t s[3];
	s[0] = data | pinA;
	s[1] = s[0] | 0x04;	// E high
	s[2] = s[0];		// E low
	loadI2C(s, 3, 1);	// Send
	return;
}
/******************************************************************************
 * static void sendLCD8(uint8_t data);
 * @brief	: Execute sequence to init the LCD
 * @param	: data = byte to be sent (in 4 bit mode)
*******************************************************************************/
static void sendLCD8(uint8_t data)
{
	uint8_t s[6];
	s[0] = (data & 0xf0) | pinA;// High order first	
	s[1] = s[0] | 0x04;	// E high
	s[2] = s[0];		// E low
	s[3] = (data << 4) | pinA;	// Low order
	s[4] = s[3] | 0x04;	// E high
	s[5] = s[3];		// E low
	loadI2C(s, 6, 1);
	return;
}
/******************************************************************************
 * void sendLCDchar(uint8_t data);
 * @brief	: 
 * @param	: data = byte to be sent (in 4 bit mode)
*******************************************************************************/
void sendLCDchar(uint8_t data)
{ // Nibble left justified; R/S = 1; backlight bit
	sendLCD4( (data & 0xf0) | 0x01 | pinA );
	sendLCD4( (data << 4)   | 0x01 | pinA );
	return;
}
/******************************************************************************
 * static void waitLCD(uint16_t ticks);
 * @brief	: Load I2C buffer with last byte loaded for delay
 * @param	: ticks = number of bytes loaded to I2C bus
*******************************************************************************/
/*
The delay is a count of IC2 bus bytes that is inserted into the circular buffer.  
Since we are only executing 'write' to the LCD, the R/W bit serves to distinguish
between a count and some data byte.  The count is restricted to 6 bits so multiple
entries are made to realized larger counts.

When the I2C routine encounters a byte with bit 1 ON it saves the count and 
continues sending the last byte until the count is exhausted at which time it
fetches the next byte from the buffer.

When the bytes in the buffer are exhausted the I2C routine continues sending the
last byte.
*/
static void waitLCD(int32_t ticks)
{
	uint8_t tmp;
	while (ticks > 0)
	{
		if (ticks > 63)
		{ // Here, load max delay count
			tmp = (63 << 2);
			ticks -= 63;
		}
		else
		{ // Load remaining delay count
			tmp = (ticks << 2);
			ticks = 0;
		}
		tmp |= 0x02;	// Flag byte as a count, not data
		// Load byte into buffer; loop if buffer is full
		while (i2c1_vcal_putbuf(&tmp, 1) == 0); 
	} 
	return;
}
/******************************************************************************
 * static void init_LCD(void);
 * @brief	: Execute sequence to init the LCD
*******************************************************************************/
/*
http://web.alfredstate.edu/weimandn/lcd/lcd_initialization/lcd_initialization_index.html
*/
void init_LCD(void)
{
uint32_t tpus = sysclk_freq/1000000;	// Ticks per microsecond
	int i;
	for (i = 0; i < 3; i++)
	{
		sendLCD4(0x30);
		timedelay(10000 * tpus);
	}
	for (i = 0; i < 2; i++)
	{
		sendLCD4(0x20);
		timedelay(10000 * tpus);
	}
	sendLCD4(0x80); // Two lines, 5x7 font
	timedelay(6000 * tpus);

	sendLCD4(0x00); // 
	waitLCD(1);
	sendLCD4(0x80); // Display ON/OFF control
	timedelay(6000 * tpus);

	sendLCD4(0x00); // 
	sendLCD4(0x10); // Clear display
	timedelay(100000 * tpus);

	// Entry mode: I/D = increment, S = shift display
	sendLCD4(0x00); // 
	sendLCD4(0x60); // I/D = 1; S = 0

	// bit 3: D = Display ON, 2: C = Cursor (under-bar), 1: B = Blinking cursor
	sendLCD4(0x00); // 
	sendLCD4(0xE0); // D =1, C = 1, B = 0
	timedelay(10000 * tpus);

	return;
}
/******************************************************************************
 * void hd44780_home(void);
 * @brief	: Return cursor to home position
*******************************************************************************/
void hd44780_home(void)
{
	sendLCD4(0x00);
	sendLCD4(0x02);
	waitLCD(4);
	return;
}
/******************************************************************************
 * void hd44780_setline(uint8_t line);
 * @brief	: Set cursor at beginning of line
 * @param	: line = line number (0 or 1)
*******************************************************************************/
void hd44780_setline(uint8_t line)
{
	if (line == 0)
		sendLCD4(0x80);	// Address to start line 0
	else
		sendLCD4(0xC0);	// Address to start line 1
	sendLCD4(0x00);
	waitLCD(400);		// Plenty of delay 
	return;
}
/******************************************************************************
 * void hd44780_backlight(uint8_t onoff);
 * @brief	: Set back light (is set at next char written)
 * @param	: onoff: 0 = off; not zero = on
*******************************************************************************/
void hd44780_backlight(uint8_t onoff)
{
	if (onoff == 0)
		pinA = 0x00;
	else
		pinA = 0x08;
	return;
}
/******************************************************************************
 * void hd44780_puts(char* p, uint8_t line);
 * @brief	: Load string into LCD line
 * @param	: p = pointer to char string (zero terminated)
 * @param	: line = line number (0 or 1)
*******************************************************************************/
static uint8_t alt;
/* Note: CR, LF are ignored.*/
void hd44780_puts(char* p, uint8_t line)
{
	uint32_t ct = 0;

//	alt ^= 0xff;
//	if (alt == 0) return;

	/* Position cursor and beginning line, 'line' */
	hd44780_setline(line & 0x01);

	/* Load chars up to zero terminator; limit to line length. */
	while ((*p != 0) && (ct < 16))
	{
		if (!((*p == 0x0D) | (*p == 0x0A)))
		{
			sendLCDchar(*p++);
			ct += 1;
		}
	}

	return;
}
/******************************************************************************
 * void hd44780_init(uint8_t address);
 * @brief	: Init I2C and LCD
 * @param	: address = I2C address (not including R/W bit)
*******************************************************************************/
void hd44780_init(uint8_t address)
{
	uint32_t tpus = sysclk_freq/1000000;	// Ticks per microsecond

	pinA = 0x08;			// Start with display back light ON

	i2c1_vcal_init(I2C1SCLKRATE);	// Init I2C comm

	timedelay(200000 * tpus);	// HD44780 Power on delay (> 152 ms)

	i2c1_vcal_start (address);	// Start I2C continuous sending

	init_LCD();			// Get LCD ready for displaying chars

	return;
}
