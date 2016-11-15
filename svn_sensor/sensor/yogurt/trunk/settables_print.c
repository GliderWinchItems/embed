/******************************************************************************
* File Name          : settables_print.c
* Date First Issued  : 09/07/2015
* Board              : f103
* Description        : yogurt maker--print things PC can set
*******************************************************************************/

#include "idx_v_struct_print.h"
#include "fpprint.h"
#include "printf.h"
#include "libusartstm32/usartallproto.h"
#include "fpprint.h"
#include "settables_print.h"
#include "loopctl_print.h"

/* **************************************************************************************
 * void settables_print(void);
 * @brief	: Print values for items that can be set from PC
 * ************************************************************************************** */
void settables_print(void)
{
	char s[48];
	printf("\n\r     Pastuerization   Fermentation\n\r");

	printf ("Heat-to ");
	fpformat(s, thm.htcl[0].heat); 	printf(" %s ",s);
	fpformat(s, thm.htcl[1].heat); 	printf("  %s \n\r",s);

	printf("Cool-to ");
	fpformat(s, thm.htcl[0].cool); 	printf(" %s ",s);
	fpformat(s, thm.htcl[1].cool); 	printf("  %s \n\rDuration @ temp (hr:mn:sc)\n\r\t",s);

	hrmn((thm.htcl[0].dur * 3600.0)); printf("   ");
	hrmn((thm.htcl[1].dur * 3600.0));

	printf("\n\rDelay start of Pasteurization: ");
	hrmn(thm.delay);

	printf("\n\r\n\r");
	return;
}
/* **************************************************************************************
 * void settables_print_menu(void);
 * @brief	: Print menu for PC keyboard input
 * ************************************************************************************** */
void settables_print_menu(void)
{
	printf("Single char commands (upper or lower case)\n\r");
	printf("F = Start Fermentation phase\n\r");
	printf("P = Start Pasteurization phase\n\r");
	printf("D = Delay start of P phase (D hh:mm for hour: minute)\n\r");
	printf("E = Start Fermentation, skip stabilization time delay\n\r");
	printf("Two char commands, First char is P or F\n\r");
	printf(" s = New heat-to setpoint temperature (deg F), integer\n\r");
	printf(" c = New cool-to setpoint temperature (deg F), integer\n\r");
	printf(" d = New duration at-temperature (hh:mm for hours:minutes)\n\r\n\r");
	return;
}
