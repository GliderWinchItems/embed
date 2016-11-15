/******************************************************************************
* File Name          : idx_v_struct_print.c
* Date First Issued  : 08/21/2015
* Board              : f103
* Description        : Print values with idx and struct (for debugging/monitoring)
*******************************************************************************/

#include "idx_v_struct_print.h"
#include "fpprint.h"
#include "printf.h"
#include "libusartstm32/usartallproto.h"

/* **************************************************************************************
 * void idx_v_struct_print(struct YOGURTTHERMS* p);
 * @brief	: Print values of struct after init from flat table
 * @param	: p = pointer to struct that has been init'd
 * ************************************************************************************** */
void idx_v_struct_print(struct YOGURTTHERMS* p)
{
	int i,j;
	printf("\n\rSTRUCT AFTER INITIALIZATION\n\r");
// --------------- Copied from yogurt.c ----------------------------------------------------
i = 1;
printf("%02d	%d	%s\n\r",   i++ , (unsigned int)p->size,"Number of elements in the following list");
printf("%02d	%d	%s\n\r",   i++ , (unsigned int)p->crc,"Tension_1: CRC for this list");
printf("%02d	%d	%s\n\r",   i++ , (unsigned int)p->version,"Version number");
for (j = 0; j < 4; j++)
{
printf(" Thermistor %d\n\r",j+1); 
fmtprint(i++ , p->tp[j].B,	"YOGURT_THERM1_CONST_B,	Yogurt_1: Thermistor param: constant B");
fmtprint(i++ , p->tp[j].R0,	"YOGURT_THERM1_R_SERIES,	Yogurt_1: Thermistor param: Series resistor, fixed (K ohms)");
fmtprint(i++ , p->tp[j].RS,	"YOGURT_THERM1_R_ROOMTMP,	Yogurt_1: Thermistor param: Thermistor room temp resistance (K ohms)");
fmtprint(i++ , p->tp[j].TREF,	"YOGURT_THERM1_REF_TEMP,	Yogurt_1: Thermistor param: Reference temp for thermistor");
fmtprint(i++ , p->tp[j].poly[0],"YOGURT_THERM1_LC_COEF_0,	Yogurt_1: Thermistor param: Load-Cell polynomial coefficient 0 (offset)");
fmtprint(i++ , p->tp[j].poly[1],"YOGURT_THERM1_LC_COEF_1,	Yogurt_1: Thermistor param: Load-Cell polynomial coefficient 1 (scale)");
fmtprint(i++ , p->tp[j].poly[2],"YOGURT_THERM1_LC_COEF_2,	Yogurt_1: Thermistor param: Load-Cell polynomial coefficient 2 (x^2)");
fmtprint(i++ , p->tp[j].poly[3],"YOGURT_THERM1_LC_COEF_3,	Yogurt_1: Thermistor param: Load-Cell polynomial coefficient 3 (x^3)");
}
printf(" Heat/Duration/Cool-to-temperature\n\r");
for (j = 0; j < 2; j++)
{
printf(" Group %d\n\r", j+1);
fmtprint(i++, p->htcl[j].heat,	" Set-point heating temperature (deg F)");
fmtprint(i++, p->htcl[j].dur,	" Hours at temperature");
fmtprint(i++, p->htcl[j].cool,	" End-point for cooling (deg F)");
}
printf(" Thermistor mapping\n\r");
printf("%02d	%d	%s\n\r",   i++ , (unsigned int)p->thmidx_shell,  " Thermistor number for shell temp (0 - 3)");
printf("%02d	%d	%s\n\r",   i++ , (unsigned int)p->thmidx_pot,    " Thermistor number for center of pot temp (0 - 3)");
printf("%02d	%d	%s\n\r",   i++ , (unsigned int)p->thmidx_airin,  " Thermistor number for air inlet temp (0 - 3)");
printf("%02d	%d	%s\n\r",   i++ , (unsigned int)p->thmidx_airout, " Thermistor number for air outlet temp (0 - 3)");

printf(" PID CONTROL LOOP COEFFICIENTS\n\r");
fmtprint(i++ , p->p,"YOGURT_1 Yogurt: Control loop: Proportional coefficient");
fmtprint(i++ , p->i,"YOGURT_1 Yogurt: Control loop: Integral coefficient ");
fmtprint(i++ , p->d,"YOGURT_1 Yogurt: Control loop: Derivative coefficient");

printf(" CAN IDs\n\r");
printf("%02d	0x%02X	%s\n\r",   i++ , (unsigned int)p->cid_yog_cmd,	" 36 YOGURT_CMD_YOGURT_1	Yogurt_1: CANID: cid_yog_cmd: Yogurt maker parameters");
printf("%02d	0x%02X	%s\n\r",   i++ , (unsigned int)p->cid_yog_msg,	" 37 YOGURT_MSG_YOGURT_1	Yogurt_1: CANID: cid_yog_msg: Yogurt maker msgs");
printf("%02d	0x%08X	%s\n\r",   i++ , (unsigned int)p->cid_yog_hb,	" 38 YOGURT_HB_YOGURT_1	Yogurt_1: CANID: cid_yog_hb: Yogurt maker heart-beats");

printf(" CONTROL CUTOFF\n\r");
fmtprint(i++ , p->heat_km_p,	" 50 YOGURT_1_HEATCONSTANT_KM_P	stored heat constant Pasteur phase");
fmtprint(i++ , p->heat_km_f,	" 51 YOGURT_1_HEATCONSTANT_KM_M	stored heat constant Ferment phase");
fmtprint(i++ , p->integrate_a,	" 52 YOGURT_1_INTEGRATEINIT_A		integrator initialization, a of  a + b*x");
fmtprint(i++ , p->integrate_b,	" 53 YOGURT_1_INTEGRATEINIT_B		integrator initialization, b of  a + b*x");
printf("%02d	%d	%s\n\r",   i++ , (unsigned int)p->stabilize_p,	" 54 YOGURT_1_STABILIZETIMEDELAY_P	time delay for temperature stabilization, Pasteur");
printf("%02d	%d	%s\n\r",   i++ , (unsigned int)p->stabilize_f,	" 55 YOGURT_1_STABILIZETIMEDELAY_F	time delay for temperature stabilization, Ferment");

if ((p->thmidx_shell == p->thmidx_pot)   || (p->thmidx_shell == p->thmidx_airin) || (p->thmidx_shell == p->thmidx_airout) \
 || (p->thmidx_pot   == p->thmidx_airin) || (p->thmidx_pot   == p->thmidx_airout)  \
 || (p->thmidx_airin == p->thmidx_airout) )
printf(" ####### ERROR THERMISTOR MAPPING HAS DUPLICATES ##########\n\r");

if ((p->thmidx_shell  > 3)   || (p->thmidx_shell  == 0) \
 || (p->thmidx_pot    > 3)   || (p->thmidx_pot    == 0) \
 || (p->thmidx_airin  > 3)   || (p->thmidx_airin  == 0) \
 || (p->thmidx_airout > 3)   || (p->thmidx_airout == 0) )
printf(" ####### ONE OR MORE THERMISTOR MAPPINGS OUT OF RANGE ##########\n\r");

USART2_txint_send(); 
// -------------------------------------------------------------------------------------------
	return;
}

