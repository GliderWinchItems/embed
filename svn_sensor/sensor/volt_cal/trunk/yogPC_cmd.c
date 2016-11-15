/******************************************************************************
* File Name          : yogPC_cmd.C
* Date First Issued  : 08/22/2015
* Board              : f103
* Description        : Serial input PC commands
*******************************************************************************/
/*
This is used for the F103 since the launchpad compiler was not working for 
floating pt printf (which works for the 'F4).
*/
#include <stdio.h>
#include <string.h>
#include "printf.h"
#include "libusartstm32/usartallproto.h"
#include "loopctl.h"
#include "vcal_idx_v_struct.h"
#include "fpprint.h"
//#include "settables_print.h"


#ifdef USEyogPC_cmd_kb	// Skip for now
static int yogPC_cmd_kb(struct USARTLB* ps);
#endif

/* **************************************************************************************
 * void yogPC_cmd_poll(void);
 * @brief	: Deal with PC incoming lines
 * ************************************************************************************** */
void yogPC_cmd_poll(void)
{
//	int ret;
	struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer

	strlb = USART1_rxint_getlineboth();	// Get both char count and pointer
	/* Check if a line is ready. */
	if (strlb.p == (char*)0) return;
	// Here we have a pointer to the line and a char count
	printf("ECHO: (ct%d) %s\n\r",strlb.ct,strlb.p);USART1_txint_send();
	if (strlb.ct < 2) return;
	switch (*(strlb.p + 0))
	{
	case 'p': // Set Pasteur phase
	case 'P':  

		break;

	case 'f': // Set Ferment phase
	case 'F': 

		break;


	default: // Here, hapless Op w fat fingers
		printf("PC: only P, F, E, X, C (either uc or lc) valid\n\r");
		break;
	}
	USART1_txint_send();
	return;
}
/* **************************************************************************************
 * static void yogPC_cmd_htcl(struct USARTLB* ps);
 * @brief	: 
 * ************************************************************************************** */

#ifdef USEyogPC_cmd_htcl	// Skip for now

static void yogPC_cmd_htcl(struct USARTLB* ps)
{
	char* p = ps->p;
//	char s[32];
	int vari;

	switch (*(p+1)) 
	{
	case 's':
		sscanf((p+2),"%d", &vari);
		printf("%d ",vari);
		break;

	case 'c':
		sscanf((p+2),"%d", &vari);
		break;


	default:
		printf("2nd character %c not on list-- s, c, d, g, k\n\r",*(p+1));
		break;

	}
	USART1_txint_send();	
	return;
}
#endif
/* **************************************************************************************
 * static int yogPC_cmd_kb(struct USARTLB* ps);
 * @brief	: 
 * ************************************************************************************** */

#ifdef USEyogPC_cmd_kb	// Skip for now

static int yogPC_cmd_kb(struct USARTLB* ps)
{
	char* ptmp;
	int kbin_hr;
	int kbin_mn;

	if (ps->ct < 7)
	{
		printf("Setdelay: %d is not enough chars\n\r", ps->ct);
		printf(" E.g.: d 12:45 (for 12 hours, 45 minutes)\n\r");
	}
	sscanf(ps->p+2,"%d", &kbin_hr);
	ptmp = strchr(ps->p+2,':');
	if (ptmp == NULL )
	{
		printf(" No colon after hrs found\n\r");
		return -1;
	}
	sscanf((ps->p+2),"%d", &kbin_hr);	// Get hours
	ptmp++;	// Step next char following ':'
	sscanf(ptmp, "%d", &kbin_mn);		// Get minutes
	printf("Delay: %d:%02d (hr:mn)\n\r", kbin_hr,kbin_mn); // Echo
	/* Do some sanity checks. */
	if ((kbin_hr > 12)  || (kbin_hr < 0))
	{
		printf("Hours greater than 12, or negative: %d\n\r",kbin_hr);
		return -1;
	}
	if ((kbin_mn < 0) || (kbin_mn > 42000))
	{
		printf("Minutes unreasonably large or negative: %d\n\r",kbin_mn);
		return -1;
	}
	/* Set the delay time. */
	return (kbin_hr * 3600 + kbin_mn * 60); // return in secs
}
#endif

