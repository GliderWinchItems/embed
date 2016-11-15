/******************************************************************************
* File Name          : startup_my.c
* Date First Issued  : 09/15/2010
* Description        : Initialization of static variables
*******************************************************************************/

/* start address for the initialization values of the .data section.
defined in linker script */
extern unsigned char  _sidata
/* start address for the .data section. defined in linker script */
extern unsigned char  _sdata
/* end address for the .data section. defined in linker script */
extern unsigned char  _edata

/* start & end addresses for the .bss section. (see .ld file) */
extern unsigned char  _sbss
extern unsigned char  _ebss

/* stack used for SystemInit_ExtMemCtl; always internal RAM used */


/* External function prototypes ----------------------------------------------*/
extern int main(void);                /* Application's main function */
extern void SystemInit(void);         /* STM's system init? */


void Reset_Handler(void)
{
  unsigned char *pX, *pY;

  /* Copy initial values for static variables (from flash to SRAM) */

  pX = &_sidata;		/* (from) Beginning of data image in ROM */
  pY = &_sdata; 	/*   (to) Beginning of data in RAM */
  while ( pY < &_edata ) *pY++ = *pX++;

  pX = &_sbss;
  /* Zero fill the .bss section. */
  while ( pX < &_ebss) *pX++ = 0;

  /* Call the application's entry point.*/
  main();

x: goto x;	// Hang loop in a style for all you FORTRAN programmers

}




