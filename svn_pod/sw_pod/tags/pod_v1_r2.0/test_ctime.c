/* test_ctime.c -- checking out linux time routines and epoch shifting
deh
10-13-2011 

A nice command line to compile and execute
gcc test_ctime.c -o test_ctime -lm && ./test_ctime

*/

#include <time.h>
#include <stdio.h>
#include <math.h>


int main (void)
{
	time_t tt,tx;
	struct tm t,t2;

/* POD epoch */
	t.tm_sec =	 0;
	t.tm_min = 	 0;
	t.tm_hour =	 0;
	t.tm_mday =	 13;
	t.tm_mon =	 9;
	t.tm_year =	 2011-1900;	

	tt = mktime (&t);	// Convert to time_t format
	
	printf ("\n\n%10u  %8.2f bits %s",tt, log(tt)/log(2),ctime(&tt));

/* Year 2038 problem */
	t2.tm_sec =	 59;
	t2.tm_min = 	 59;
	t2.tm_hour =	 23;
	t2.tm_mday =	 31;
	t2.tm_mon =	  9;
	t2.tm_year =	 2028-1900;	

	tx = mktime (&t2);	// Convert to time_t format
	
	printf ("%10u  %8.2f bits %s",tx, log(tx)/log(2), ctime(&tx));
	printf ("%10u  %8.2f bits in the difference between the above two date/times\n",tx-tt,log(tx-tt)/log(2));


	time(&tt);	
	printf ("%10u  %8.2f bits %s\n",tt, log(tt)/log(2), ctime(&tt));

	struct tm *pt;
	pt = gmtime(&tt);
	printf ("\nCurrent time in tm struct form\n%4u year\n%4u mon\n%4u mday\n%4u hour\n%4u min\n%4u sec\n",pt->tm_year,pt->tm_mon,pt->tm_mday,pt->tm_hour,pt->tm_min,pt->tm_sec);


	return 0;

}
