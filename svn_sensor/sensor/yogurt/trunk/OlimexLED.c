/******************************************************************************
* File Name          : OlimexLED.c
* Date First Issued  : 08/20/2015
* Board              : f103
* Description        : LED flashing for the Olimex board's LED
*******************************************************************************/

#include "OlimexLED.h"
#include "pinconfig_all.h"
#include "libopenstm32/gpio.h"
#include "libmiscstm32/DTW_counter.h"
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/
static uint32_t tix;	// System clock tick count
static int32_t toggleinc = 0;

static const struct PINCONFIGALL pin_LED = {(volatile u32 *)GPIOC, 12, OUT_PP, MHZ_50};
#define LEDBIT 12
/******************************************************************************
 * void OlimexLED_init(void);
 * @brief	: Init LED on Olimex board
*******************************************************************************/
void OlimexLED_init(void)
{
	pinconfig_all( (struct PINCONFIGALL *)&pin_LED);// Configure pin
	GPIO_BRR(GPIOE) = (1<<LEDBIT);			// Reset bit = LED on
	tix = DTWTIME;
	OlimexLED_settogglerate(2);	
	return;
}
/******************************************************************************
 * void OlimexLED_settogglerate(int32_t togglerate);
 * @brief	: Set rate for toggling LED
 * @param	: togglerate = toggles per 10 sec; 0 = continous OFF; -1 = ON
*******************************************************************************/
void OlimexLED_settogglerate(int32_t togglerate)
{
	toggleinc = (sysclk_freq * 10)/togglerate;	
	return;
}
/******************************************************************************
 * static void toggle(void);
 * @brief	: Toggle the LED
*******************************************************************************/
static void toggle(void)
{
	if ((GPIO_ODR(GPIOC) & (1<<LEDBIT)) == 0)
	{ // Here, LED bit was off
		GPIO_BSRR(GPIOC) = (1<<LEDBIT);	// Set bit = LED off
	}
	else
	{ // Here, LED bit was on
		GPIO_BRR(GPIOC) = (1<<LEDBIT);	// Reset bit = LED on
	}
	return;
}
/******************************************************************************
 * void OlimexLED_togglepoll(void);
 * @brief	: Toggle LED at a rate (call this in polling loop)
*******************************************************************************/
void OlimexLED_togglepoll(void)
{
	if (toggleinc == 0)
	{
		GPIO_BSRR(GPIOC) = (1<<LEDBIT);	// Set bit = LED off
		tix = DTWTIME + 64000000;
		return;
	}
	if (toggleinc < 0)
	{
		GPIO_BRR(GPIOC) = (1<<LEDBIT);	// Reset bit = LED on
		tix = DTWTIME + 64000000;
		return;
	}
	if ( ((int)DTWTIME - (int)tix) > 0)
	{ // Here time expired
		toggle();
		tix += toggleinc;
	}
	return;	
}
/******************************************************************************
 * void OlimexLED_flashpin(uint32_t gpio, uint32_t pinnumber);
 * @brief	: Turn LED ON when gpio pin is high, (assume pin was configured)
 * @param	: gpio = address of gpio port
 * @param	: pinnumber = gpio pin number on port
*******************************************************************************/
void OlimexLED_flashpin(uint32_t gpio, uint32_t pinnumber)
{
	if ((GPIO_IDR(gpio) & (1 << pinnumber)) == 0)
	{ // Here, pin is low, set bit high to turn LED off
		GPIO_BSRR(GPIOC) = (1<<LEDBIT);	// Set bit = LED off
	}
	else
	{ // Here, pin is high, set bit low to turn LED on
		GPIO_BRR(GPIOC) = (1<<LEDBIT);	// Reset bit = LED on
	}
	return;	
}

