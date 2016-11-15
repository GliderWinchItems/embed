#ifndef __MY_MAIN_H_
#define	__MY_MAIN_H_

/* What follows is the interface for a minimum programing environment
 * for the F4DISCOVERY board.  The environment includes _read(...)
 * and _write(...) functions so scanf and printf work.  You have a main
 * function and you also have a timer function called periodically so 
 * you can setup timers.
 *
 * The scanf/printf functions work by plugging the top USB interface
 * on the F4DISCOVERY board into a computer and using your favorite
 * terminal program connect to the appropriate device ("/dev/ttyACM0"
 * on Linux).
 *
 * Start by creating a clone of STM32F4_USB_CDC. The only file you should
 * have to modify is <STM32F4_USB_CDC/src/my_main.c>.  
 */
 
/* my_main() -- Gets called from the internal main() after basic
 * initialization (including USB initialization) has been done.
 */
int my_main(void);

/* my_ticker() -- Gets called back at the fixed rate of MY_TICKER_RATE
 * callbacks/sec.
 */
#define	MY_TICKER_RATE	1000
void my_ticker(void);

#endif
