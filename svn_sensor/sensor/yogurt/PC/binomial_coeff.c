/******************************************************************************
* File Name          : binomial_coeff.c
* Date First Issued  : 08/30/2015
* Board              : Linux PC
* Description        : Compute binomial coefficient
*******************************************************************************/
/*
http://rosettacode.org/wiki/Evaluate_binomial_coefficients#C
gcc binomial_coeff.c -o binomial_coeff -Wall -lm

*/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <stdio.h>
#include <limits.h>
 
/* We go to some effort to handle overflow situations */
 
static unsigned long gcd_ui(unsigned long x, unsigned long y) {
  unsigned long t;
  if (y < x) { t = x; x = y; y = t; }
  while (y > 0) {
    t = y;  y = x % y;  x = t;  /* y1 <- x0 % y0 ; x1 <- y0 */
  }
  return x;
}
 
unsigned long binomial(unsigned long n, unsigned long k) {
  unsigned long d, g, r = 1;
  if (k == 0) return 1;
  if (k == 1) return n;
  if (k >= n) return (k == n);
  if (k > n/2) k = n-k;
  for (d = 1; d <= k; d++) {
    if (r >= ULONG_MAX/n) {  /* Possible overflow */
      unsigned long nr, dr;  /* reduced numerator / denominator */
      g = gcd_ui(n, d);  nr = n/g;  dr = d/g;
      g = gcd_ui(r, dr);  r = r/g;  dr = dr/g;
      if (r >= ULONG_MAX/nr) return 0;  /* Unavoidable overflow */
      r *= nr;
      r /= dr;
      n--;
    } else {
      r *= n--;
      r /= d;
    }
  }
  return r;
}
 
int main() {
#define N 16	
    unsigned long M = (N-1)/2;
    unsigned long m = (N-3)/2;
    unsigned long bm1;
    unsigned long bm2;
    unsigned long den;
    unsigned long cat;
	int k;
    for (k = 0; k < M; k++)
    {	
    	bm1 = binomial(2*m, m-k+1);
	bm2 = binomial(2*m, m-k-1);
 	den =  (1 << (2*m+1) );
  	cat = (bm1 - bm2)/den;
	printf("%2d: ",k);
	    printf("%5lu ", den);
	    printf("%5lu ", bm1);
	    printf("%5lu ", bm2);
	    printf("%5lu ", bm1-bm2);
	printf("\n");
    }
    return 0;
}
