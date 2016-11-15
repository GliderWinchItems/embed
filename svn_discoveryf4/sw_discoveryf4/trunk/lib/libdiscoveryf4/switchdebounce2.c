/******************************************************************************
* File Name          : switchdebounce2.c
* Date First Issued  : 01/19/2014
* Board              : Discovery F4
* Description        : Debounce switches
*******************************************************************************/
/*
This routine handles the debouncing of switches.  The switches can be a mix of
switches connected to gpio pins, or switches that are monitored via 'spi2.rw',
which reads parallel-serial shift registers into memory periodically, the period
set by 'countdowntimer.c' output capture interrupts (currently 1/2 ms).

The countdowntimer interrupt triggers a spi2 read/write cycle, and the completion of
the spi read/write cycle calls 'static void do_switches(void)' in this routine.

Initialization of this routine also intializes 'spi2.rw'.

'structs' with the parameters for switches are linked by calls to--
debouncesw_add - for gpio connected switches
debouncespisw_add - for spi/parall-serial switches

See switchdebounce.h for details on the parameters in the switch structs.

The state of a switch can be tested in 'main' by testing 'flag' in the struct for that 
particular switch.  'flag' always carries the latest debounced state--
  zero = pin low
  not-zero = pin high

Alternately, a change of state of the switch can cause a callback to a function.
The function called is a pointer in the struct for the switch.  The callback is
conditional on a parameter 'cbx' in the struct that specifies if the callback 
is to be made when the switch opens, closes, or either opens or closes (either
transition).

Note that the callback is under interrupt and a) should not take long, b) has to
take care of any sharing of variables with code running at a different processor
level.

*/
#include <malloc.h>
#include <stdio.h>

#include "nvicdirect.h" 
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/timer.h"
#include "libopencm3/stm32/nvic.h"


#include "DISCpinconfig.h"
#include "countdowntimer.h"
#include "switchdebounce2.h"
#include "spi2rw.h"


/* This holds the root (last added, but beginning) of the linked list of structs. */
static struct SWITCHDEBVARS* pdbnchead  = 0;  // Ending struct in list has NULL  

/* Subroutine prototypes */
static void do_switches(void);
static void do_spi2(void);

/* Pointers to buffers for parallel-serial in & out */
static char* pspiout;	// Output (if you weren't able to guess)
static char* pspiin;	// Input
static int   ctspi;	// Number of bytes 

/* **************************************************************************************
 * void debouncesw_init(char *pout, char* pin, int count);
 * @brief	: Initialize for switch debouncing of parallel-serial extension
 * @param	: char *pout = pointer to byte array with bytes to output
 * @param	: char *pin  = pointer to byte array to receive bytes coming in
 * @param	: int count  = byte count of number of write/read cycles
 * ************************************************************************************** */
void debouncesw2_init(char *pout, char* pin, int count)
{
	pspiin = pin; pspiout = pout; ctspi = count; // Save locally
	spi2_readdoneptr = &do_switches;	// Timer triggers spi read/write
	timer_sw_ptr = &do_spi2;		// spi completion checks switches
	spi2rw_init();	// SPI is used to read/write parallel-serial hardware for switch expansion
	return;
}
/* **************************************************************************************
 * static struct SWITCHDEBVARS* debouncesw_common(const struct SWITCHDEBPARAMS* ptr, u32 x);
 * @brief	: Get and link a switch variables struct to the linked list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @param	: x = initial pin reading
 * @return	: NULL = malloc failed.  Pointer to struct with flag
 * @note	: No checking of the struct for valid values--BE CAREFUL
 * ************************************************************************************** */
static struct SWITCHDEBVARS* debouncesw_common(const struct SWITCHDEBPARAMS* ptr, u32 x)
{
	struct SWITCHDEBVARS* pvar;

	pvar = (struct SWITCHDEBVARS*)malloc(sizeof(struct SWITCHDEBVARS));
	if (pvar == NULL) return 0;

	pvar->tbounce = 0;	// Initial debounce timer
	pvar->flag = x;		// Initial switch state
	
	/* Save where to find the fixed parameters for this switch. */
	pvar->pfix = ptr;
	
	/* Add to list (the 1st one added has ptr->next = 0) */ 
	pvar->pnext = pdbnchead;	// ptr->pnext pts to previous timer struct in chain
	pdbnchead = pvar;	// Switch checking becomes active when when this instruction completes

	return pvar;
}
/* **************************************************************************************
 * u32* debouncesw_add(const struct SWITCHDEBPARAMS* ptr);
 * @brief	: Add fixed and variable structs for this switch to the list for GPIO port & pins
 * @param	: ptr = Pointer to struct with parameters to be used
 * @return	: NULL = failed. Not NULL = pointer to flag.
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */
u32* debouncesw2_add(const struct SWITCHDEBPARAMS* ptr)
{
	u32 x;
	struct SWITCHDEBVARS* pvar;

	struct PINCONFIG pin;	// Used for pin configuration
		pin.mode  = GPIO_MODE_INPUT;	// mode: Input
		pin.type  = 0;			// output type: not applicable
		pin.speed = 0;			// speed: not applicable
		pin.pupdn = 0;			// pull up/down:
		pin.afrl  = 0;			// alternate function: not used

	/* Configure I/O pin for this switch. */	
	pin.pupdn = (ptr->updn & 0x3);	// Set pull up/down code
	f4gpiopins_Config( (volatile u32*)ptr->port, ptr->pin, &pin);

	/* SPDT switch requires two pins and may be different ports. */
	if (ptr->port2 != NULL) // Did the caller specify a 2nd port & pin?
	{ // Here, we assume a SPDT type switch with two pins is used, so configure the pin.
		f4gpiopins_Config( (volatile u32*)ptr->port2, ptr->pin2, &pin);		
	}
	
	/* Set initial switch based on what we see on the first pin. */
	x = ( GPIO_IDR((u32)ptr->port)  & (1 << ptr->pin));	// Get latest switch status

	/* Initialize struct to hold the variables for this switch and add it to linked list */
	pvar = debouncesw_common(ptr, x);
	return &pvar->flag;	// Return pointer to flag
}

/* **************************************************************************************
 * u32* debouncespisw_add(const struct SWITCHDEBPARAMS* ptr);
 * @brief	: Add fixed and variable structs for this switch to the list for spi/parallel-serial.
 * @param	: ptr = Pointer to struct with parameters to be used
 * @return	: NULL = failed. Not NULL = pointer to flag.
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */
u32* debouncespisw2_add(const struct SWITCHDEBPARAMS* ptr)
{
	u32 x;

	/* Set initial switch based on what we see. */
	x = ( *(u8*)ptr->port & (1 << ptr->pin));

	/* Initialize struct and add to linked list */
	return &(debouncesw_common(ptr, x))->flag; // Return pointer to flag
}

/* =====================================================================================
Switch debouncing update
This routine is called from 'countdowntimers' when the output capture for checking switches
interrupts.
   ===================================================================================== */
int x2flag = 0; // Debug

static void do_switches(void)
{
	struct SWITCHDEBVARS* p = pdbnchead; // Pointer to linked list of variables
	struct SWITCHDEBPARAMS* ptr; // Pointer to fixed switch parameters
	
	u32 x;	// Temp for 'flag' for pin status (zero, or non-zero)
	u32 x2;	// Temp for 2nd pin status in SPDT case
x2flag += 1; // Debug

	while (p != 0) // When pnext is null there are no more switches to check
	{ // Here, p points to struct holding switch variables

		ptr = (struct SWITCHDEBPARAMS*)p->pfix;	// Get pointer to struct with fixed parameters for this switch

		/* Get the latest state of the switch */
		if ( ((u32)ptr->port & 0xff0000000) >= (u32)0x200000000 ) // gpio or spi/parallel-serial?
		{ // Here, address is somewhere in SRAM, so it is an SPI parallel-serial switch
			x = ( GPIO_IDR((u32)ptr->port)  & (1 << ptr->pin));	// Get latest switch status
		}
		else
		{ // Here, address is not in SRAM, and presumed to be in the gpio peripheral range
			x = ( *(u8*)ptr->port & (1 << ptr->pin));
		}

		if (ptr->port2 == NULL) // Is this a SPDT type switch?
		{ // Here, the switch is not a SPDT case, and uses timing to debounce
			if (p->tbounce > 0) // Are we still timing a switch state change?
			{ // Here, still timing a previous transition
				p->tbounce -= 1; // Timing count down
			}
			else
			{ // Here, debounce timing has expired
				if (x != p->flag)	// Has pin state changed?
				{ // Here, pin differs from the last time we checked.
					if (p->flag == 0) // Which type of transition might we have?
					{ // Here, new is '1', previously was '0'
						p->flag = x;
						p->tbounce = ptr->topeni;
						/* If there is an address set AND either 0->1 transition, or either transition, call routine */
						if ( (ptr->func != 0) && (ptr->cbx != 0x1) )
							(*ptr->func)(p);	// Callback: execute some function
					}
					else
					{ // Here, new is '0', previous was '1'
						p->flag = x;
						p->tbounce = ptr->tclosei;
						/* If there is an address set AND either 1->0 transition, or either transition, call routine */
						if ((ptr->func != 0) && (ptr->cbx != 0x0) )
							(*ptr->func)(p);	// Callback: execute some function
					}
				}
			}
		}
		else
		{ // Here the switch is a SPDT type and uses logic to debounce
			/* Get the latest state of the 2nd pin  */
			if ( ((u32)ptr->port2 & 0xff0000000) >= (u32)0x200000000 ) // gpio or spi/parallel-serial?
			{ // Here, address is somewhere in SRAM, so it is an SPI parallel-serial switch
				x2 = ( GPIO_IDR((u32)ptr->port2)  & (1 << ptr->pin2));	// Get latest switch status
			}
			else
			{ // Here, address is not in SRAM, and presumed to be in the peripheral range
				x2 = ( *(u8*)ptr->port2 & (1 << ptr->pin2));
			}
			
			/* When pins x and x2 are both high or both low, leave the state unchanged */
			if ((x2 == 0) && (x != 0))
				p->flag = x;  // pin2 high, pin low
			else
			{
				if ((x2 != 0) && (x == 0))
					p->flag = x; // pin2 low pin high
			}
		}
		p = p->pnext;	// Step to next switch in list
	}
	return;
}

/* =====================================================================================
Initiate spi read/write cycle.  
This routine is called from the countdowntimer output capture interrupt every 1/2 ms.
Completion of the spi read/write calls the 'do_switches' routine above.  
The pointers for the routine calls are setup in 'debouncesw_init' above.
   ===================================================================================== */
static void do_spi2(void)
{
	/* Start another read/write to the spi */
	if (spi2_busy() != 0)

		spi2_rw (pspiout, pspiin, ctspi);

	return;
}
