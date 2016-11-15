/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : ADC-ppm.c
* Hackeroo           : deh
* Date First Issued  : 07/21/2011
* Board              : Linux PC
* Description        : Make table for temp v osc ppm adjustment
*******************************************************************************/
/*
gcc ADC-ppm.c -o ADC-ppm -lm	[compile]
./ADC-ppm			[run]
*/
/*
Make a table of ppm adjustments to a 32768 Hz xtal,
given ADC readings

Thermistor equation:
R = Ro * exp(B*(1/T - 1/To));

ADC equations:
ADC count = ADCCOUNT * (R/(R+Rf));

R = (ADCreading * Rf)/(ADCCOUNT - ADCreading);

ADCreading = R * ADCCOUNT / (R + Rf);

Crystal temp compensation--
ppm = -0.034 T^2 
where: T is difference in deg C from "turnpoint" (25 deg C is standard)

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>


#define ADCCOUNT	4095	// Max ADC reading
#define ADCHILIMIT	3269	// Highest ADC reading we deal with
#define ADCLOLIMIT	924	// Lowest ADC reading we deal with
#define SIZEOFTABLE	(ADCHILIMIT-ADCLOLIMIT)	// Number of ADC steps
#define LOTEMP		0.0	// Temp (deg C) for highest R
#define HITEMP		60.0	// Temp (deg C) for lowest R
#define TURNPOINTTEMP	25	// Temp (deg C) for xtal turnpoint
#define KTURNPOINT	273+TURNPOINTTEMP	// Kelvin temp of turnpoint
#define FREQVTEMP	-.034	// Crystal freq deviation v temp^2
#define SCALE		100	// Scale ppm to integer
/******************************************************************************/
unsigned short adcppm_cal (unsigned short usAdc, short sCal);
/* @brief	: Convert adc reading to osc ppm error * 100
 * @param	: usAdc: 12b adc reading
 * @param	: sCal: adc adjustment for calibration 
 * @return	: ppm * 100
*******************************************************************************/
extern const unsigned short usPPMtbloffset;	/* Lowest ADC covered by table */
extern const unsigned short usPPMtablesize;		/* Size of table */

double dX,dXX,dA,dP;
float fA,fP;
int nA,nP,nE;
int nPx;
int nMin,nMax;
int nSq,nDiff;




int n;

FILE *fp1,*fp2;		/* File to be saved */


int main (void)
{
	int i;
	char buf[256];
	char buf1[256];


	if ( (fp1 = fopen("adcppmX.c","w")) == NULL)
	{
		printf ("fp1--File open failed\n"); return 1;
	}

	if ( (fp2 = fopen("gps_adc_v_ppm1.dat","r")) == NULL)
	{
		printf ("fp2--File open failed\n"); return 1;
	}
	
	/* Output in a form suitable for inclusion in a "c" routine */

	sprintf (buf,"\n\n\n/* Osc freq error [negative] (ppm * %4u) versus ADC reading (offset) */\n", SCALE);
	fwrite (buf,strlen(buf),1,fp1);	 	write (STDOUT_FILENO,buf,strlen(buf));

	sprintf (buf,"\n\n\nconst unsigned short usPPMtbloffset	= %5u;		/* Lowest ADC covered by table */\n", ADCLOLIMIT);
	fwrite (buf,strlen(buf),1,fp1);
	write (STDOUT_FILENO,buf,strlen(buf));

 	sprintf (buf,"const unsigned short usPPMtablesize		= %5u;		/* Size of table */\n",SIZEOFTABLE);
	fwrite (buf,strlen(buf),1,fp1);	
	write (STDOUT_FILENO,buf,strlen(buf));


	sprintf (buf,"const unsigned short sPPMtable[%u] = { \n",SIZEOFTABLE);
	fwrite (buf,strlen(buf),1,fp1);	
	write (STDOUT_FILENO,buf,strlen(buf));

	fgets(buf,180,fp2);	// Header line
	i = 0;	nMin = 1000000; nMax = -10000000;
	while ( fgets(buf1,180,fp2) )
	{
		sscanf(buf1,"%f %f",&fA,&fP); dA=fA;dP=fP;
		nA = fA + .5;
		nP = fP*100;
		nPx = adcppm_cal(nA,-35);
		nPx = (nPx*92)/100;
		nE = -175;
		if ((nP-nPx-nE) < nMin) nMin = (nP-nPx-nE);
		if ((nP-nPx-nE) > nMax) nMax = (nP-nPx-nE);
		sprintf (buf,"%4d %6d %6d %6d %6d\n",i,nA,nP,nPx,nP-nPx-nE);
//		fwrite (buf,strlen(buf),1,fp1);	
		write (STDOUT_FILENO,buf,strlen(buf));
		i +=1;
	}
	printf("Max %6d Min %6d\n",nMax,nMin);
	printf ("};\n"); fprintf (fp1,"};\n");
	fclose (fp1);

	return 0;
}


