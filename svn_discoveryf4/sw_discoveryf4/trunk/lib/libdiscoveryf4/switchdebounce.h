/******************************************************************************
* File Name          : switchdebounce.h
* Date First Issued  : 12/28/2013
* Board              : Discovery F4
* Description        : Debounce switches
*******************************************************************************/


#ifndef __SW_DEBOUNCE
#define __SW_DEBOUNCE

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
*/

struct SWITCHDEBPARAMS
{
	u32	flag;		// Debounced switch: zero or not-zero
	u32	topen;		// Working counter for timing open debounce
	u32	tclose;		// Working counter for timing close debounce
	u32	topeni;		// Debounce time count for close-to-open
	u32	tclosei;	// Debounce time count for open-to-close
	u32*	port;		// Port, or for spi the pointer to the byte 
	u32	pin;		// Pin on port, or for spi the bit number within byte
	u32	cbx;		// Callback is made when flag: 0 = 1->0; 1 = 0->1; 2 = either change 
	void 	(*func)(void* psw);	// Pointer to function to call
	u32	updn;		// Use internal pull up/dn or none (0); for spi = 0
	void* pnext;		// Pointer to next struct	
};

/* ************************************************************************************** */
void debouncesw_init(char *pout, char* pin, int count);
/* @brief	: Initialize for switch debouncing of parallel-serial extension
 * @param	: char *pout = pointer to byte array with bytes to output
 * @param	: char *pin  = pointer to byte array to receive bytes coming in
 * @param	: int count  = byte count of number of write/read cycles
 * ************************************************************************************** */
void debouncesw_add(struct SWITCHDEBPARAMS* ptr);
/* @brief	: Link debounce struct to the sw debounce list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */
void debouncespisw_add(struct SWITCHDEBPARAMS* ptr);
/* @brief	: Link debounce struct to the sw debounce list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */

#endif 

