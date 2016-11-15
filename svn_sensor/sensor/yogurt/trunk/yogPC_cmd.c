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
#include "yogurt_idx_v_struct.h"
#include "fpprint.h"
#include "settables_print.h"



static void yogPC_cmd_htcl(struct HEATCOOL* phtcl, struct USARTLB* ps);
static int yogPC_cmd_kb(struct USARTLB* ps);
/* **************************************************************************************
 * void yogPC_cmd_poll(void);
 * @brief	: Deal with PC incoming lines
 * ************************************************************************************** */
void yogPC_cmd_poll(void)
{
	int ret;
	struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer

	strlb = USART2_rxint_getlineboth();	// Get both char count and pointer
	/* Check if a line is ready. */
	if (strlb.p == (char*)0) return;
	// Here we have a pointer to the line and a char count
	printf("ECHO: (ct%d) %s\n\r",strlb.ct,strlb.p);USART2_txint_send();
	if (strlb.ct < 2) return;
	switch (*(strlb.p + 0))
	{
	case 'p': // Set Pasteur phase
	case 'P':  
		if (strlb.ct > 2)
		{ // Here, a multi-char command
			yogPC_cmd_htcl(&thm.htcl[0], &strlb);	// Pointer to Pasteur heat/cool params
			
		}
		else
		{ // Here a single char command
			loopctl_setstate(1);
			printf("Set pasteurization heat up [1]\n\r");
		}
		break;

	case 'f': // Set Ferment phase
	case 'F': 
		if (strlb.ct > 2)
		{ // Here, a multi-char command
			yogPC_cmd_htcl(&thm.htcl[1], &strlb);	// Pointer to Ferment heat/cool params
			
		}
		else
		{ // Here a single char command
			loopctl_setstate(11);
			printf("Set fermenation heat up [11]\n\r");
		}
		break;

	case 'e': // Set Ferment phase, skip fast heatup
	case 'E': //  w stabilize delay
		loopctl_setstate(122);
		printf("Ferment phase, start control loop\n\r");
		break;

	case 'x': // Go to idle 
	case 'X':  loopctl_setstate(0);
		printf("\n\rSet idle (OFF) state [0]\n\r\n\r");
		settables_print_menu();
		break;

	case 'c': // Set current phase to cool down
	case 'C':  
		ret = loopctl_getstate();
		if (ret > 10)
		{ // Here in Ferment phase.  Set cool down
			loopctl_setstate(39);
			printf("Set fermenation cooling state [39]\n\r");
		}
	else
		{ // Here in Pasteur phase.  Set cool down
			loopctl_setstate(31);
			printf("Set pasteurization cool down [31]\n\r");
		}
		break;

	case 'd': // Set delay time
	case 'D':
		ret = yogPC_cmd_kb(&strlb);
		if (ret < 0) break;
		thm.delay = ret;
		break;

	default: // Here, hapless Op w fat fingers
		printf("PC: only P, F, E, X, C (either uc or lc) valid\n\r");
		break;
	}
	USART2_txint_send();
	settables_print();
	return;
}
/* **************************************************************************************
 * static void yogPC_cmd_htcl(struct YOGURTTHERMS* phtcl, struct USARTLB* ps);
 * @brief	: 
 * ************************************************************************************** */
static void yogPC_cmd_htcl(struct HEATCOOL* phtcl, struct USARTLB* ps)
{
	char* p = ps->p;
	char s[32];
	float varf;
	int vari;
	int ret;

	switch (*(p+1)) 
	{
	case 's': // Setpoint: heat-to temperature
		sscanf((p+2),"%d", &vari);
		printf("%d ",vari);
		if ((vari < 40) || (vari > 205))
		{
			printf("Heat-to value %d (deg F) is too small or too big\n\r",vari);
			break;
		}
		varf = vari;
		fpformat(s, varf); printf("%s ",s);
		printf("Heat-to value now: %s (deg F)\n\r",s);
		phtcl->heat = varf;
		break;

	case 'c': // Setpoint: cool-down-to temperature
		sscanf((p+2),"%d", &vari);
		printf("%d ",vari);
		if ((vari < 40) || (vari > 120))
		{
			printf("Cool-down value %d (deg F) is less than 40 or greater than 120\n\r",vari);
			break;
		}
		varf = vari;
		fpformat(s, varf);
		printf("Cool-down-to value now: %s (deg F)\n\r",s);
		phtcl->cool = varf;
		break;


	case 'd':
		ret = yogPC_cmd_kb(ps);
		if (ret < 0) break;
		phtcl->dur = (float)ret/3600.0;
		break;

	case 'g':
	case 'k':
	default:
		printf("2nd character %c not on list-- s, c, d, g, k\n\r",*(p+1));
		break;

	}
	USART2_txint_send();	
	return;
}
/* **************************************************************************************
 * static int yogPC_cmd_kb(struct USARTLB* ps);
 * @brief	: 
 * ************************************************************************************** */
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
