/******************************************************************************
* File Name          : switchdebounce.c
* Date First Issued  : 12/28/2013
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

#include "nvicdirect.h" 
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/timer.h"
#include "libopencm3/stm32/nvic.h"


#include "DISCpinconfig.h"
#include "countdowntimer.h"
#include "switchdebounce.h"
#include "spi2rw.h"


/* This holds the root (last added, but beginning) of the linked list of structs. */
static struct SWITCHDEBPARAMS* pdbnchead  = 0;  // Ending struct in list has NULL  

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
void debouncesw_init(char *pout, char* pin, int count)
{
//	spi2rw_init();	// SPI is used to read/write parallel-serial hardware for switch expansion

	pspiin = pin; pspiout = pout; ctspi = count; // Save locally
	spi2_readdoneptr = &do_switches;	// Timer triggers spi read/write
	spi2rw_init();				// initialize spi2 parallel-serial
	timer_sw_ptr = &do_spi2;		// spi completion checks switches
	return;
}
/* **************************************************************************************
 * static void debouncesw_common(struct SWITCHDEBPARAMS* ptr);
 * @brief	: Link debounce struct to the sw debounce list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */
static void debouncesw_common(struct SWITCHDEBPARAMS* ptr)
{

	/* Be sure the counters are set to count down */
	ptr->topen = ptr->topeni;
	ptr->tclose =ptr->tclosei;

	/* Add to list (the 1st one added has ptr->next = 0) */ 
	ptr->pnext = pdbnchead;	// ptr->pnext pts to previous timer struct in chain
	pdbnchead = ptr;	// Switch checking becomes active when when this instruction completes

	return;
}
/* **************************************************************************************
 * void debouncesw_add(struct SWITCHDEBPARAMS* ptr);
 * @brief	: Link debounce struct to the sw debounce list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */
void debouncesw_add(struct SWITCHDEBPARAMS* ptr)
{
	struct PINCONFIG pin;	// Used for pin configuration
		pin.mode  = GPIO_MODE_INPUT;	// mode: Input
		pin.type  = 0;			// output type: not applicable
		pin.speed = 0;			// speed: not applicable
		pin.pupdn = 0;			// pull up/down:
		pin.afrl  = 0;			// alternate function: not used

	/* Configure I/O pin for this switch. */	
	pin.pupdn = (ptr->updn & 0x3);	// Set pull up/down code
	f4gpiopins_Config( (volatile u32*)ptr->port, ptr->pin, &pin);

	/* Set initial switch based on what we see. */
	ptr->flag = ( GPIO_IDR((u32)ptr->port)  & (1 << ptr->pin));	// Get latest switch status

	/* Initialize struct and add to linked list */
	debouncesw_common(ptr);

	return;
}

/* **************************************************************************************
 * void debouncespisw_add(struct SWITCHDEBPARAMS* ptr);
 * @brief	: Link debounce struct to the sw debounce list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */
void debouncespisw_add(struct SWITCHDEBPARAMS* ptr)
{
	/* Set initial switch based on what we see. */
	ptr->flag = ( *(u8*)ptr->port & (1 << ptr->pin));

	/* Initialize struct and add to linked list */
	debouncesw_common(ptr);

	return;
}

/* =====================================================================================
Switch debouncing update
This routine is called from 'countdowntimers' when the output capture for checking switches
interrupts.
   ===================================================================================== */
int xflag = 0;

static void do_switches(void)
{
	struct SWITCHDEBPARAMS* p = pdbnchead;
	u32 x;
xflag += 1;

	while (p != 0) // When pnext is null there are no more to check
	{ // Here, p points to struct for a switch
		/* Get the latest state of the switch */
		if ( ((u32)p->port & 0xff0000000) >= (u32)0x200000000 )
		{ // Here, address is somewhere in SRAM, so it is an SPI parallel-serial switch
			x = ( GPIO_IDR((u32)p->port)  & (1 << p->pin));	// Get latest switch status
		}
		else
		{ // Here, address is not in SRAM, and presumed to be in the peripheral range
			x = ( *(u8*)p->port & (1 << p->pin));
		}
		if (x != p->flag)	// Has it changed?
		{ // Here, pin differs from the last time we checked.
			if (p->flag == 0) // Which type of transition might we have?
			{ // Here, new is '1', previously was '0'
				if (p->topen == 0) // Has time expired?
				{ // Here yes.  Assume switch is stable
					p->flag = x;			// Flag = switch setting
					p->topen = p->topeni;		// Reset timeout time countdown counter
					/* If there is an address set AND either 0->1 transition, or either transition, call routine */
					if ( (p->func != 0) && (p->cbx != 0x1) )
						(*p->func)(p);	// Go do something
				}
				else
				{ // Here, no. Count down the time
					p->topen -= 1;
				}
			}
			else
			{ // Here, new is '0', previous was '1'
				if (p->tclose == 0)	// Has time expired?
				{ // Here, yes.  Assume switch is stable.
					p->flag = x;			// Flag = switch setting
					p->tclose = p->tclosei;	// Reset timeout countdown counter
					/* If there is an address set AND either 1->0 transition, or either transition, call routine */
					if ((p->func != 0) && (p->cbx != 0x0) )
						(*p->func)(p);	// Go do something
				}
				else
				{
					p->tclose -= 1;
				}
			}
		}
		else
		{ // Here flag and pin are the same, prepare timers for time when there is a change
			p->topen = p->topeni;
			p->tclose =p->tclosei;
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
