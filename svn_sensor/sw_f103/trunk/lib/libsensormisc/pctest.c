/*
pctest.c // Test pay_flt_cnv routines
01/15/2015

To compile and run use the following--
cd ~/svn_sensor/sw_f103/trunk/lib/libsensormisc
gcc pctest.c -o pctest ../../../../../svn*f4/common_all/trunk/pay_flt_cnv.c -Wall && ./pctest 1.2345678
*/
#include "/home/deh/svn_discoveryf4/common_all/trunk/pay_flt_cnv.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	float val;

	if ((argc < 2) || (argc > 2))
	{
		printf ("Just one argument: the value to be tested, e.g 1.2345678\n"); return -1;
	}
	sscanf (argv[1],"%f", &val);
	printf("\n program sees value as: %f\n",val);

	int i;
	float f = val;
	float f2 = 0.0;
	uint8_t pay[8];
	for (i = 0; i < 8; i++) pay[i] = 0;

	/* Do 3/4 float conversion check. */
	printf("\n3-QTR FLOAT CONVERSION CHECK\n");

	printf ("    input: %20.10f %08X \n",f,*(uint32_t*)&f);

	/* Convert float to 3 byte payload. */
	floattopay3qtrfp(&pay[0],f);	

	/* Display payload. */
	printf ("3/4 payload:");
	for (i = 0; i < 8; i++)
	{
		printf("%02x ", pay[i]);
	}

	/* Convert 3 byte payload to float. */
	f2 = pay3qtrfptofloat(&pay[0]);
	printf("\n   output: %e %08X ",f2,*(uint32_t*)&f2);
/* ------------------------------------------------------------------- */
	/* Do half-float conversion check */
	for (i = 0; i < 8; i++) pay[i] = 0;
	printf("\n\nHALF-FLOAT CONVERSION CHECK\n");
	f = val;	
	printf ("    input: %20.10f %08X \n",f,*(uint32_t*)&f);

	/* Convert float to 2 byte payload. */
	floattopayhalffp(&pay[0],f);

	/* Display payload. */
	printf ("1/2 payload:");	
	for (i = 0; i < 8; i++)
	{
		printf("%02x ", pay[i]);
	}
	printf ("\n");

	/* Convert 2 byte payload to float. */
	f2 = payhalffptofloat(&pay[0]);

	printf ("output: %e %08X \n\n",f2,*(uint32_t*)&f2);
/* ------------------------------------------------------------------- */
	/* Do single precision float conversion check */
	for (i = 0; i < 8; i++) pay[i] = 0;
	printf("SINGLE PRECISION FLOAT CONVERSION CHECK\n");
	f = val;	
	printf ("    input: %20.10f %08X \n",f,*(uint32_t*)&f);

	/* Convert float to 2 byte payload. */
	floattopaysinglefp(&pay[0],f);

	/* Display payload. */
	printf ("SINGLE payload:");	
	for (i = 0; i < 8; i++)
	{
		printf("%02x ", pay[i]);
	}
	printf ("\n");

	/* Convert 2 byte payload to float. */
	f2 = paysinglefptofloat(&pay[0]);

	printf ("output: %e %08X \n\n",f2,*(uint32_t*)&f2);

	return 0;
}
