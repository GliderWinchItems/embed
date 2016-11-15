/******************************************************************************
* File Name          : tension_a_printf.c
* Date First Issued  : 05/24/2015
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/
#include "tension_a_printf.h"
#include "libusartstm32/usartallproto.h"
#include "fmtprint.h"

/* **************************************************************************************
 * void tension_a_printf(struct TENSIONLC* pten);
 * @brief	: Print the values
 * @param	: pten = pointer to struct with the values 
 * ************************************************************************************** */
void tension_a_printf(struct TENSIONLC* pten)
{
int i = 0;
printf("TENSION_a: values: pointer = %08X\n\r",(int)pten);
// NOTE: fmtprint is a fixed format conversion to setup at string to print %8.3f
printf("%2d	%d	%s\n\r",   i + 0, (unsigned int)pten->size,     "  0 Number of elements in the following list");
printf("%2d	%d	%s\n\r",   i + 1, (unsigned int)pten->crc,      "  1 Tension_1: CRC for tension list");
printf("%2d	%d	%s\n\r",   i + 2, (unsigned int)pten->version,  "  2 Version number");
printf("%2d	%d	%s\n\r",   i + 3, (unsigned int)pten->ad.offset,"  3 TENSION_AD7799_1_OFFSET,	Tension: AD7799 offset");
fmtprint(i+ 4, pten->ad.scale * 100, "  4 TENSION_AD7799_1_SCALE,	Tension: AD7799 #1 Scale (convert to kgf)");
fmtprint(i+ 5, pten->ad.tp[0].B,     "  5 TENSION_THERM1_CONST_B,	Tension: Thermistor1 param: constant B");
fmtprint(i+ 6, pten->ad.tp[0].R0,    "  6 TENSION_THERM1_R_SERIES,	Tension: Thermistor1 param: Series resistor, fixed (K ohms)");
fmtprint(i+ 7, pten->ad.tp[0].RS,    "  7 TENSION_THERM1_R_ROOMTMP,	Tension: Thermistor1 param: Thermistor room temp resistance (K ohms)");
fmtprint(i+ 8, pten->ad.tp[0].TREF,  "  8 TENSION_THERM1_REF_TEMP,	Tension: Thermistor1 param: Reference temp for thermistor");
fmtprint(i+ 9, pten->ad.tp[0].offset,"  9 TENSION_THERM1_TEMP_OFFSET,	Tension: Thermistor1 param: Thermistor temp offset correction (deg C)");
fmtprint(i+10, pten->ad.tp[0].scale, " 10 TENSION_THERM1_TEMP_SCALE,	Tension: Thermistor1 param: Thermistor temp scale correction");
fmtprint(i+11, pten->ad.tp[1].B,     " 11 TENSION_THERM2_CONST_B,	Tension: Thermistor2 param: constant B");
fmtprint(i+12, pten->ad.tp[1].RS,    " 12 TENSION_THERM2_R_SERIES,	Tension: Thermistor2 param: Series resistor, fixed (K ohms)");
fmtprint(i+13, pten->ad.tp[1].R0,    " 13 TENSION_THERM2_R_ROOMTMP,	Tension: Thermistor2 param: Thermistor room temp resistance (K ohms)");
fmtprint(i+14, pten->ad.tp[1].TREF,  " 14 TENSION_THERM2_REF_TEMP,	Tension: Thermistor2 param: Thermistor temp offset correction (deg C)");
fmtprint(i+15, pten->ad.tp[1].offset," 15 TENSION_THERM2_TEMP_OFFSET,	Tension: Thermistor2 param: Thermistor temp offset correction (deg C)");
fmtprint(i+16, pten->ad.tp[1].scale, " 16 TENSION_THERM2_TEMP_SCALE,	Tension: Thermistor2 param: Thermistor temp scale correction");
fmtprint(i+17, pten->ad.comp_t1[0],  " 17 TENSION_THERM1_LC_COEF_0,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset)");
fmtprint(i+18, pten->ad.comp_t1[1],  " 18 TENSION_THERM1_LC_COEF_1,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale)");
fmtprint(i+19, pten->ad.comp_t1[2],  " 19 TENSION_THERM1_LC_COEF_2,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2)");
fmtprint(i+20, pten->ad.comp_t1[3],  " 20 TENSION_THERM1_LC_COEF_3,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3)");
fmtprint(i+21, pten->ad.comp_t2[0],  " 21 TENSION_THERM2_LC_COEF_0,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset)");
fmtprint(i+22, pten->ad.comp_t2[1],  " 22 TENSION_THERM2_LC_COEF_1,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale)");
fmtprint(i+23, pten->ad.comp_t2[2],  " 23 TENSION_THERM2_LC_COEF_2,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2)");
fmtprint(i+24, pten->ad.comp_t2[3],  " 24 TENSION_THERM2_LC_COEF_3,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3)");
printf("%2d	%d	%s\n\r",i+25, (unsigned int)pten->hbct,          " 25 TENSION_HEARTBEAT_CT	Tension: hbct: Heart-Beat Count of time ticks between autonomous msgs");
printf("%2d	%d	%s\n\r",i+26, (unsigned int)pten->drum,          " 26 TENSION_DRUM_NUMBER	Tension: drum: Drum system number for this function instance");
printf("%2d	0x%02X	%s\n\r",i+27, (unsigned int)pten->f_pollbit,     " 27 TENSION_DRUM_FUNCTION_BIT	Tension: bit: f_pollbit: Drum system poll 1st byte bit for function instance");
printf("%2d	0x%02X	%s\n\r",i+28, (unsigned int)pten->p_pollbit,     " 28 TENSION_DRUM_POLL_BIT	Tension: bit: p_pollbit: Drum system poll 2nd byte bit for this type of function");
printf("%2d	0x%08X	%s\n\r",i+29, (unsigned int)pten->cid_ten_msg,   " 29 CANID_MSG_TENSION_a 	Tension_a: 29 CANID: can msg tension");
printf("%2d	0x%08X	%s\n\r",i+30, (unsigned int)pten->cid_ten_poll,  " 30 CANID_MSG_TIME_POLL 	Tension_a: 30 CANID: MC: Time msg/Group polling");
printf("%2d	0x%08X	%s\n\r",i+31, (unsigned int)pten->cid_gps_sync,  " 31 CANID_HB_TIMESYNC   	Tension_a: 31 CANID: GPS time sync distribution msg");
printf("%2d	0x%08X	%s\n\r",i+32, (unsigned int)pten->cid_heartbeat, " 32 CANID_HB_TENSION_a   	Tension_a: 32 Heartbeat");
printf("%2d	0x%08X	%s\n\r",i+33, (unsigned int)pten->cid_tst_ten_a, " 33 CANID_TST_TENSION_a, 	Tension_a: 33 Test");
printf("%2d	%d	%s\n\r",i+34, (unsigned int)pten->iir[0].k,      " 34 TENSION_a_IIR_POLL_K 	Tension_a: IIR Filter factor: divisor sets time constant: reading for polled msg");
printf("%2d	%d	%s\n\r",i+35, (unsigned int)pten->iir[0].scale,  " 35 TENSION_a_IIR_POLL_SCALE Tension_a: IIR Filter scale : upscaling (due to integer math): for polled msg");
printf("%2d	%d	%s\n\r",i+36, (unsigned int)pten->iir[1].k,      " 36 TENSION_a_IIR_HB_K IR Filter factor: divisor sets time constant: reading for heart-beat msg");
printf("%2d	%d	%s\n\r",i+37, (unsigned int)pten->iir[1].scale,  " 37 TENSION_a_IIR_HB_SCALE 	Tension_a: IIR Filter scale : upscaling (due to integer math): for heart-beat msg");
printf("%2d	%d	%s\n\r",i+38, (unsigned int)pten->useme,         " 38 TENSION_a_USEME 	Tension_a: Function instance bits. 0x1 = first ad7799; 0x3 - two AD779");
printf("%2d	%d	%s\n\r",i+39, (unsigned int)pten->iir_z_recal.k, " 39 TENSION_a_IIR_Z_RECAL_K Tension_a: IIR Filter factor: divisor sets time constant: zero recalibration");
printf("%2d	%d	%s\n\r",i+40, (unsigned int)pten->iir_z_recal.scale," 40 TENSION_a_IIR_Z_RECAL_SCALE 	Tension_a: IIR Filter scale : upscaling: zero recalibration");
printf("%2d	%d	%s\n\r",i+41, (unsigned int)pten->z_recal_ct,    " 41 TENSION_a_Z_RECAL_CT 	Tension_a: ADC conversion counts between zero recalibrations");
fmtprint(i+42, pten->limit_hi,       " 42 TENSION_a_LIMIT_HI	Tension_a: Exceeding this limit (+) means invalid reading");
fmtprint(i+43, pten->limit_lo,       " 43 TENSION_a_LIMIT_LO	Tension_a: Exceeding this limit (-) means invalid reading");
printf("%2d	0x%08X	%s\n\r",i+44, (unsigned int)pten->code_CAN_filt[0], " 44 Tension: CANID 1 for setting up CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+45, (unsigned int)pten->code_CAN_filt[1], " 45 Tension: CANID 2 for setting up CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+46, (unsigned int)pten->code_CAN_filt[2], " 46 Tension: CANID 3 for setting up CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+47, (unsigned int)pten->code_CAN_filt[3], " 47 Tension: CANID 4 for setting up CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+48, (unsigned int)pten->code_CAN_filt[4], " 48 Tension: CANID 5 for setting up CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+49, (unsigned int)pten->code_CAN_filt[5], " 49 Tension: CANID 6 for setting up CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+50, (unsigned int)pten->code_CAN_filt[6], " 50 Tension: CANID 7 for setting up CAN hardware filter");
printf("%2d	0x%08X	%s\n\r",i+51, (unsigned int)pten->code_CAN_filt[7], " 51 Tension: CANID 8 for setting up CAN hardware filter");

	USART1_txint_send(); 
	return;
}
