/******************************************************************************
* File Name          : startup.c
* Date First Issued  : 09/15/2010
* Description        : Initialization of static variables
*******************************************************************************/

/* start & end addresses for the .data section. (see .ld file) */
extern unsigned char __data_section_start; 	/* Beginning of data section in RAM */
extern unsigned char __data_image_end;		/* End of data section in RAM */
extern unsigned char __data_image;		/* Beginning of data section in ROM */

/* start & end addresses for the .bss section. (see .ld file) */
extern unsigned char _start_of_bss;
extern unsigned char _end_of_bss;

/* External function prototypes ----------------------------------------------*/
extern int main(void);                /* Application's main function */
extern void SystemInit(void);         /* STM's system init? */


void Reset_Handler(void)
{
  unsigned char *pX, *pY;

  /* Copy initial values for static variables (from flash to SRAM) */

  pX = &__data_image;		/* (from) Beginning of data image in ROM */
  pY = &__data_section_start; 	/*   (to) Beginning of data in RAM */
  while ( pY < &__data_image_end ) *pY++ = *pX++;

  pX = &_start_of_bss;
  /* Zero fill the .bss section. */
  while ( pX < &_end_of_bss) *pX++ = 0;

  /* Call the application's entry point.*/
  main();

x: goto x;	// Hang loop in a style for all you FORTRAN programmers

}




