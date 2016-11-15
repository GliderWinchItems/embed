
/* Inline CLZ patterned after --
 http://balau82.wordpress.com/2011/05/17/inline-assembly-instructions-in-gcc/ 
*/

static inline __attribute__((always_inline))
int arm_clz(int x) 
{
  int d;
  asm ("CLZ %[Rd], %[Rm]" : [Rd] "=r" (d) : [Rm] "r" (x) );
  return d;
}



