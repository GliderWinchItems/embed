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


#define B	3380	// Thermistor constant
#define P	0.034	// Xtal freq v temp^2 constant
#define RT	10	// Ro thermistor resistance (K @ 25 deg C)
#define RF	7.15	// Fixed resistor resistance
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

double dR[SIZEOFTABLE];	// Resistance of thermistor for ADC reading
double dT[SIZEOFTABLE];	// Temp for ADC reading
double dP[SIZEOFTABLE];	// Crystal deviation in PPM 

int n;

FILE *fp1,*fp2;		/* File to be saved */

/* Compute thermistor resistance, given the temperature (deg C) */
double dRx(double *pdDegC)
{
	return RT*exp( B * ((1.0/(273+*pdDegC)) - 1.0/(KTURNPOINT)) );
	
}

double dTe(double *pRx)
{
	double dRxx,dTx;
	/* Compute the temperature that corresponds to the thermistor resistance */
	n = 0;
	for (dTx = LOTEMP; n < 1000000; dTx += (HITEMP-LOTEMP)/100000 )
	{
		dRxx = dRx(&dTx);
		if (dRxx < *pRx)
		{
			return dTx;
			break;
		}
		n +=1;
	}
	return 999.0;
}


int main (void)
{
	int i,j;
	double a,b;
	double dRx1,dRx2,dRdelta;
	double dTlo,dThi;
	char buf[180];


	if ( (fp1 = fopen("adcppm.c","w")) == NULL)
	{
		printf ("fp1--File open failed\n"); return 1;
	}

	printf ("\n\n IDX  ADC   Rtherm (K)   n      Temp(C)      PPM      PPM\n");
	for (i = 0; i<SIZEOFTABLE; i++)
	{
		/* Compute thermistor resistance versus allowed ADC readings */
		a = ADCCOUNT; b = ADCLOLIMIT+i;
		dR[i] = (b * RF)/(a - b);
		/* Compute the temperature that corresponds to the thermistor resistance */
		dT[i] = dTe(&dR[i]);
		/* Compute ppm for this temperature */
		dP[i] = FREQVTEMP * (dT[i]-TURNPOINTTEMP) * (dT[i]-TURNPOINTTEMP);
		j = -((SCALE * dP[i])-.5);
		printf ("%4i %4i %10.4f %7i %10.4f %10.4f %6i\n",i,i+ADCLOLIMIT,dR[i],n,dT[i],dP[i],j);
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


	for (i = 0; i<SIZEOFTABLE; i++)
	{
		j = -SCALE * dP[i];
		sprintf (buf,"%6u,	// %5u %10.4f\n", j,i,dT[i]);
		fwrite (buf,strlen(buf),1,fp1);	
//		write (STDOUT_FILENO,buf,strlen(buf));

	}
	printf ("};\n"); fprintf (fp1,"};\n");
	fclose (fp1);

	/* Output ADC versus temperature for inclusion in a "c" routine */
	if ( (fp2 = fopen("adcthermtmp.c","w")) == NULL)
	{
		printf ("fp2--File open failed\n"); return 1;
	}

#define TMPSCALEFACTOR	100	// Scale upwards the degree C reading to better fit a 'short'

	sprintf (buf,"\n\n\n/* Temperature (deg C * %4u) versus ADC reading (offset) */\n", TMPSCALEFACTOR);
	fwrite (buf,strlen(buf),1,fp1);	 	write (STDOUT_FILENO,buf,strlen(buf));


	sprintf (buf,"\n\n\nconst unsigned short usTHERMtbloffset	= %5u;		/* Lowest ADC covered by table */\n", ADCLOLIMIT);
	fwrite (buf,strlen(buf),1,fp1);	 	write (STDOUT_FILENO,buf,strlen(buf));

 	sprintf (buf,"const unsigned short usTHERMtablesize		= %5u;		/* Size of table */\n",SIZEOFTABLE);
	fwrite (buf,strlen(buf),1,fp1);		write (STDOUT_FILENO,buf,strlen(buf));


	sprintf (buf,"const unsigned short sTHERMtable[%u] = { \n",SIZEOFTABLE);
	fwrite (buf,strlen(buf),1,fp1);		write (STDOUT_FILENO,buf,strlen(buf));

	
	for (i = 0; i<SIZEOFTABLE; i++)
	{
		sprintf (buf, "%6u,	// %5u %10.4f\n",(unsigned int)(dT[i]*TMPSCALEFACTOR+.5),i,dT[i]);
		fwrite (buf,strlen(buf),1,fp1);	
//		write (STDOUT_FILENO,buf,strlen(buf));
	}

	printf ("};\n"); fprintf (fp1,"};\n");
	fclose (fp2);
	printf ("\n\nEND");

	return 0;
}


