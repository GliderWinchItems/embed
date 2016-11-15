/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "stm32f4xx_conf.h"		/* Define GPIO stuff */

#include "my_main.h"

// Private variables
static volatile uint32_t time_var1, time_var2;

// Private function prototypes
static void calculation_test();
static void echo_test(void);
static void Delay(volatile uint32_t nCount);
static void speed_test();

/******************************************************************************/
int my_main(void)
{
	switch( 2 )
	{
		case 0: echo_test();		break;
		case 1: calculation_test();	break;
		case 2: speed_test();		break;
	}
	return 0;
}

void my_ticker()
{
	if (time_var1) {
		time_var1--;
	}

	time_var2++;
}

/******************************************************************************/
static void echo_test(void)
{
	int _read(int, char *, int);
	int _write(int, char *, int);
	char mybuf[100];
	int i;
	
	for(i=0; 1==1; i+=1)
	{
		if(_read(0, mybuf, 1))
		{
			if(*mybuf != '\r')
				_write(0, mybuf, 1);
			else
				_write(0, "\r\n", 2);

			if(i & 1)
				GPIO_ResetBits(GPIOD, GPIO_Pin_12);
			else
				GPIO_SetBits(GPIOD, GPIO_Pin_12);
		}
	}
}

static void calculation_test()
{
	float a = 1.001;
	int pass;
	double pi = 3.1415926535897932384626433;

	for(pass=0; 1==1; pass+=1)
	{
		GPIO_SetBits(GPIOD, GPIO_Pin_12);
		Delay(MY_TICKER_RATE/10);
		GPIO_ResetBits(GPIOD, GPIO_Pin_12);

		time_var2 = 0;
		for (int i = 0;i < 1000000;i++) {
			a += 0.01 * sqrtf(a);
		}

		printf("Time:  %lu ms\n\r", time_var2);
		printf("Pass:  %i\n\r", pass);
		printf("Value: %.5f\n\r", a);
		printf("exp(pi) - pi == %23.20f\r\n", exp(pi) - pi);
		printf("\r\n");
	}
}

static void speed_test()
{
	char c;
	char* tmsg = "0123456789ABCDEFGHIJKLMNOQRSTUVWXYabcedefhijklmnopqrstluvwxyz 0123456789ABCDEFGHIJKLMNOQRSTUVWXYabcedefhijklmnopqrstluvwxyz\n\r";
	volatile unsigned int i;
	unsigned int j = 0;
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)
	unsigned int t_old = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
	unsigned int t_dif;
	unsigned int t_new;
	unsigned int t_d0,t_d1;

	while (1)
	{
		t_new =	*(volatile unsigned int *)0xE0001004;
		t_dif = t_new-t_old;
		if (t_dif >= 16800000){t_old = t_new; c = '*';} else {c = '.';}
		printf("%c %6i %6i %s",c, j++, (t_d1-t_d0), tmsg);
		t_d0 =	*(volatile unsigned int *)0xE0001004 + 120000;
		while ((int)(t_d0 - *(volatile unsigned int *)0xE0001004) > 0) ;
		t_d1 =	*(volatile unsigned int *)0xE0001004;

	}
	
}

/* Delay a number of systick cycles (1ms)
 */
static void Delay(volatile uint32_t nCount)
{
	time_var1 = nCount;
	while(time_var1){};
}
