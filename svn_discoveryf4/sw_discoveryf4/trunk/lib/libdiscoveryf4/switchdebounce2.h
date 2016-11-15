/******************************************************************************
* File Name          : switchdebounce2.h
* Date First Issued  : 01/19/2014
* Board              : Discovery F4
* Description        : Debounce switches
*******************************************************************************/


#ifndef __SW_DEBOUNCE2
#define __SW_DEBOUNCE2

/*
struct SWITCHDEBPARAMS
flag 
  Shows the last debounced status of the switch.  
  Zero = switch pin low; not-zero = switch pin high.
  This can be polled from a mainline.  
topen
  Number of ticks that the switch pin must be stable for a 0-to-1 transition.
  This is working countdown timer
tclose
  Number of ticks that the switch pin must be stable for a 1-to-0 transition.
  This is working countdown timer
topeni
  Intial value for 'topen' (1/2 ms ticks)
tclosei
  Initial value for 'tclose' (1/2 ms ticks)
port
  Address of gpio port for switches connected to gpio pins
  Address of byte of in-memory image of parallel-to-serial (spi) input.
pin
  Pin number withing port (0-31 for gpio), (0-7) for parallel-serial
cbx
  Callback mode.  Callback is made when the pointer 'psw' is not zero (NULL) and
     'flag' (debounced switch status) changes AND --
        cbx = 0; flag goes from not-zero to zero.
        cbx = 1; flag goes from zero to not-zero.
        cbx = 2; flag changes in either direction
func
  Pointer to callback function
pnext
  Pointer to next struct in list.  Set to NULL (zero) on the last struct in list
    (which was the first one added).

Currently there is no editing of the values in the struct for bogus values.

01-19-2014  Addition to handle SPDT switches (using two pins) and split fixed parameters
   from variables so that fixed parameters can be declared 'const' and put in flash.

NOTE: For SPDT switches the NO and NC contacts go to two pins and the software/logic 
takes care the debouncing.  In the struct if 'port2' is not NULL (zero) the setup assumes
it is a SPDT type switch using two port/pins (or in the spi case, two pins, possibly in
different bytes).

*/
/* Fixed parameters for switch */
struct SWITCHDEBPARAMS
{
	u32	topeni;		// Debounce time count for close-to-open
	u32	tclosei;	// Debounce time count for open-to-close
	u32*	port;		// Port, or for spi the pointer to the byte 
	u32	pin;		// Pin on port, or for spi the bit number within byte
	u32*	port2;		// Port, or for spi the pointer to the byte 
	u32	pin2;		// Pin on port, or for spi the bit number within byte
	u32	cbx;		// Callback is made when flag: 0 = 1->0; 1 = 0->1; 2 = either change 
	void 	(*func)(void* psw);	// Pointer to function to call
	u32	updn;		// Use internal pull up/dn or none (0); for spi = 0
};
/* Variables for switch */
struct SWITCHDEBVARS
{
	u32	flag;		// Debounced switch: zero or not-zero
	u32	tbounce;	// Count down timer for timing debounce
	const struct SWITCHDEBPARAMS* pfix; // Pointer to fixed parameters for the switch
	void* pnext;		// Pointer to next struct in list (last = NULL)	
};


/* ************************************************************************************** */
void debouncesw2_init(char *pout, char* pin, int count);
/* @brief	: Initialize for switch debouncing of parallel-serial extension
 * @param	: char *pout = pointer to byte array with bytes to output
 * @param	: char *pin  = pointer to byte array to receive bytes coming in
 * @param	: int count  = byte count of number of write/read cycles
 * ************************************************************************************** */
u32* debouncesw2_add(const struct SWITCHDEBPARAMS* ptr);
/* @brief	: Link debounce struct to the sw debounce list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @return	: NULL = failed. Not NULL = pointer to flag.
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */
u32* debouncespisw2_add(const struct SWITCHDEBPARAMS* ptr);
/* @brief	: Link debounce struct to the sw debounce list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @return	: NULL = failed. Not NULL = pointer to flag.
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */

#endif 

