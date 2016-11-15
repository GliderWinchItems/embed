/******************************************************************************
* File Name          : loopctl_print.c
* Date First Issued  : 08/17/2015
* Board              : f103
* Description        : Control loop for yogurt maker--print some things
*******************************************************************************/

//#include "idx_v_struct_print.h"
#include "fpprint.h"
#include "printf.h"
#include "libusartstm32/usartallproto.h"
#include "fpprint.h"
#include "loopctl_print.h"
#include "adcsensor_vcal.h"

/* **************************************************************************************
 * void loopctl_print_hdr(void);
 * @brief	: Print column header
 * ************************************************************************************** */
void loopctl_print_hdr(void)
{
	printf(" looperr ");
	printf("integral ");
	printf("derivative ");
	printf("\n\r");
	USART1_txint_send(); 
	return;
}
/* **************************************************************************************
 * void hrmn(int32_t x);
 * @brief	: Convert secs to hr:mn (toss fractional minutes)
 * @param	: x = secs ct
 * ************************************************************************************** */
void hrmn(int32_t x)
{
	int j = x;
	int sc;
	int mn;
	int hr;
	if (j < 0) j = -j;
	mn = j/60;
	sc = j - (mn * 60);
	hr = mn/60;
	mn = mn - (hr * 60);
	if (x < 0) hr = -hr;
	printf("%3d:%02d:%02d ",hr,mn,sc);
	return;
}
/* **************************************************************************************
 * void loopctl_print(struct LOOPCTL_STATE* p);
 * @brief	: Print values in struct for monitoring 'loopctl' progress
 * @param	: p = pointer to struct with stuff to print
 * ************************************************************************************** */
static uint32_t secs_prev = 0;
extern float  thermf[NUMBERADCCHANNELS_TEN];		// Floats of thermistors readings
void loopctl_print(struct LOOPCTL_STATE* p)
{
	char s[48];
//	int i;

	/* Throttle down to secs */
//$	if (p->secs == secs_prev) return;
	secs_prev = p->secs;

	/* Convert remaining secs to hr:min */
//	hrmn( (int)p->secs0 - (int)p->secs );

	fpformat(s, p->setpoint); printf("%ss ",s);

	printf("%5di ", p->ipwm);

//	for ( i= 0; i < 4; i++)
//	{
//		fpformat(s, thermf[i]); printf("%s ",s);
//	}

	/* Convert remaining secs to hr:min */
	hrmn( (p->secs - p->secsref) );

	fpformat(s, p->looperr); 	printf("%sE ",s);
	fpformat(s, p->proportional); 	printf("%sP ",s);
	fpformat(s, p->integral); 	printf("%sI ",s);
	fpformat(s, p->derivative); 	printf("%sD ",s);

//	fpformat(s,(float)(p->looperr+p->integral+p->derivative)); printf("%s ",s);
	
//	printf("\n\r");
	return;
}
