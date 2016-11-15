/* Comparison of PC to fputest.c on STM32 */
/*
08/24/2014
gcc ctest.c -o ctest -lm && ./ctest
*/

#include <math.h>
#include <stdio.h>

int main(void)
{
	volatile unsigned int t0,t1,t2,t3;
volatile unsigned int tdly;
#define NLOOP	10
float x = 0;
long double xx = 0;
float r;
long double rr;
int z;
int i;
float xf = 0;
#define DTWTIME 1

	printf("ct        arg       atanf     ticks         atan      ticks\n");
	for (i = 0; i < NLOOP+1; i++) 
	{
		for (tdly = 0; tdly < 4000000; tdly++);
		t0 = DTWTIME;
		r = atanf(x);
		t1 = DTWTIME;

		t2 = DTWTIME;
xx = x;
		rr = atanl(xx);	
		t3 = DTWTIME;

		printf("%2d %13.11f %11.9f %5d   %23.14Lf %5d\n",i+1, x, r,(t1-t0),rr,(t3-t2) );
		x  += 0.125;
//		xx += (1<<i)*0.01;
	}
	
	x = 0; xx = 0;
	printf("ct        arg       tanf      ticks          tan      ticks\n");
	for (i = 0; i < NLOOP+1; i++) 
	{
		for (tdly = 0; tdly < 4000000; tdly++);
		t0 = DTWTIME;
		r = tanf(x);
		t1 = DTWTIME;

		t2 = DTWTIME;
xx = x;
		rr = tanl(xx);	
		t3 = DTWTIME;

		printf("%2d %13.9f %11.9f %5d   %23.21Lf %5d\n",i+1, x, r,(t1-t0),rr,(t3-t2) );
		x  += (((3.1415926535897932)/2)/NLOOP);
//		xx += (((3.14159265358979323)/2)/NLOOP);
	}
	xf=0;xx=0;
	printf("\nct        arg       sinf     ticks         sin      ticks\n");
	for (i = 0; i < NLOOP+1; i++) 
	{
		for (tdly = 0; tdly < 1000000; tdly++);
		t0 = DTWTIME;
		r = sinf(xf);
		t1 = DTWTIME;

		t2 = DTWTIME;
xx = xf;
		rr = sin(xx);	
		t3 = DTWTIME;

		printf("%2d %13.11f %11.9f %5d  %13.11Lf %16.14f %5d %7.3e\n\r",i+1, xf, r,(t1-t0),xx,rr,(t3-t2), (rr-r) );
		xf  += (((3.1415926535897932)/4)/NLOOP);
		xx += (((3.14159265358979323)/4)/NLOOP);
	}


 return 0;

}
