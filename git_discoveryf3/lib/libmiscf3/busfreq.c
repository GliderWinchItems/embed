/******************************************************************************
* File Name          : busfreq.c
* Date First Issued  : 06/17/2011
* Description        : Global variables for clocksetup.c clockspecifysetup.c, routines that set baud rates
*******************************************************************************/
/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
unsigned int	hclk_freq;	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
unsigned int	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/


