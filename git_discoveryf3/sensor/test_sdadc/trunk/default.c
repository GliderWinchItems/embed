/******************************************************************************
* File Name          : default.c
* Date First Issued  : 02/06/2016
* Board              : Discovery F3 w F373 processor
* Description        : nvic interrupt traps with code save
*******************************************************************************/


#include "libmiscf3/panic_ledsDf3.h"

#define PANIC_CODE 5	// Code to pass on to panic_ledsxxx.c
void panic_leds_local(unsigned int panic_code)
{
	panic_ledsDf3(panic_code);
}

unsigned int Default_HandlerCode = 0x999;	// Holds nvic interrupt number

void Default_Handler00(void) { Default_HandlerCode =  0; panic_leds_local(PANIC_CODE); }
void Default_Handler01(void) { Default_HandlerCode =  1; panic_leds_local(PANIC_CODE); }
void Default_Handler02(void) { Default_HandlerCode =  2; panic_leds_local(PANIC_CODE); }
void Default_Handler03(void) { Default_HandlerCode =  3; panic_leds_local(PANIC_CODE); }
void Default_Handler04(void) { Default_HandlerCode =  4; panic_leds_local(PANIC_CODE); }
void Default_Handler05(void) { Default_HandlerCode =  5; panic_leds_local(PANIC_CODE); }
void Default_Handler06(void) { Default_HandlerCode =  6; panic_leds_local(PANIC_CODE); }
void Default_Handler07(void) { Default_HandlerCode =  7; panic_leds_local(PANIC_CODE); }
void Default_Handler08(void) { Default_HandlerCode =  8; panic_leds_local(PANIC_CODE); }
void Default_Handler09(void) { Default_HandlerCode =  9; panic_leds_local(PANIC_CODE); }
void Default_Handler10(void) { Default_HandlerCode = 10; panic_leds_local(PANIC_CODE); }
void Default_Handler11(void) { Default_HandlerCode = 11; panic_leds_local(PANIC_CODE); }
void Default_Handler12(void) { Default_HandlerCode = 12; panic_leds_local(PANIC_CODE); }
void Default_Handler13(void) { Default_HandlerCode = 13; panic_leds_local(PANIC_CODE); }
void Default_Handler14(void) { Default_HandlerCode = 14; panic_leds_local(PANIC_CODE); }
void Default_Handler15(void) { Default_HandlerCode = 15; panic_leds_local(PANIC_CODE); }
void Default_Handler16(void) { Default_HandlerCode = 16; panic_leds_local(PANIC_CODE); }
void Default_Handler17(void) { Default_HandlerCode = 17; panic_leds_local(PANIC_CODE); }
void Default_Handler18(void) { Default_HandlerCode = 18; panic_leds_local(PANIC_CODE); }
void Default_Handler19(void) { Default_HandlerCode = 19; panic_leds_local(PANIC_CODE); }
void Default_Handler20(void) { Default_HandlerCode = 20; panic_leds_local(PANIC_CODE); }
void Default_Handler21(void) { Default_HandlerCode = 21; panic_leds_local(PANIC_CODE); }
void Default_Handler22(void) { Default_HandlerCode = 22; panic_leds_local(PANIC_CODE); }
void Default_Handler23(void) { Default_HandlerCode = 23; panic_leds_local(PANIC_CODE); }
void Default_Handler24(void) { Default_HandlerCode = 24; panic_leds_local(PANIC_CODE); }
void Default_Handler25(void) { Default_HandlerCode = 25; panic_leds_local(PANIC_CODE); }
void Default_Handler26(void) { Default_HandlerCode = 26; panic_leds_local(PANIC_CODE); }
void Default_Handler27(void) { Default_HandlerCode = 27; panic_leds_local(PANIC_CODE); }
void Default_Handler28(void) { Default_HandlerCode = 28; panic_leds_local(PANIC_CODE); }
void Default_Handler29(void) { Default_HandlerCode = 29; panic_leds_local(PANIC_CODE); }
void Default_Handler30(void) { Default_HandlerCode = 30; panic_leds_local(PANIC_CODE); }
void Default_Handler31(void) { Default_HandlerCode = 31; panic_leds_local(PANIC_CODE); }
void Default_Handler32(void) { Default_HandlerCode = 32; panic_leds_local(PANIC_CODE); }
void Default_Handler33(void) { Default_HandlerCode = 33; panic_leds_local(PANIC_CODE); }
void Default_Handler34(void) { Default_HandlerCode = 34; panic_leds_local(PANIC_CODE); }
void Default_Handler35(void) { Default_HandlerCode = 35; panic_leds_local(PANIC_CODE); }
void Default_Handler36(void) { Default_HandlerCode = 36; panic_leds_local(PANIC_CODE); }
void Default_Handler37(void) { Default_HandlerCode = 37; panic_leds_local(PANIC_CODE); }
void Default_Handler38(void) { Default_HandlerCode = 38; panic_leds_local(PANIC_CODE); }
void Default_Handler39(void) { Default_HandlerCode = 39; panic_leds_local(PANIC_CODE); }
void Default_Handler40(void) { Default_HandlerCode = 40; panic_leds_local(PANIC_CODE); }
void Default_Handler41(void) { Default_HandlerCode = 41; panic_leds_local(PANIC_CODE); }
void Default_Handler42(void) { Default_HandlerCode = 42; panic_leds_local(PANIC_CODE); }
void Default_Handler43(void) { Default_HandlerCode = 43; panic_leds_local(PANIC_CODE); }
void Default_Handler44(void) { Default_HandlerCode = 44; panic_leds_local(PANIC_CODE); }
void Default_Handler45(void) { Default_HandlerCode = 45; panic_leds_local(PANIC_CODE); }
void Default_Handler46(void) { Default_HandlerCode = 46; panic_leds_local(PANIC_CODE); }
void Default_Handler47(void) { Default_HandlerCode = 47; panic_leds_local(PANIC_CODE); }
void Default_Handler48(void) { Default_HandlerCode = 48; panic_leds_local(PANIC_CODE); }
void Default_Handler49(void) { Default_HandlerCode = 49; panic_leds_local(PANIC_CODE); }
void Default_Handler50(void) { Default_HandlerCode = 50; panic_leds_local(PANIC_CODE); }
void Default_Handler51(void) { Default_HandlerCode = 51; panic_leds_local(PANIC_CODE); }
void Default_Handler52(void) { Default_HandlerCode = 52; panic_leds_local(PANIC_CODE); }
void Default_Handler53(void) { Default_HandlerCode = 53; panic_leds_local(PANIC_CODE); }
void Default_Handler54(void) { Default_HandlerCode = 54; panic_leds_local(PANIC_CODE); }
void Default_Handler55(void) { Default_HandlerCode = 55; panic_leds_local(PANIC_CODE); }
void Default_Handler56(void) { Default_HandlerCode = 56; panic_leds_local(PANIC_CODE); }
void Default_Handler57(void) { Default_HandlerCode = 57; panic_leds_local(PANIC_CODE); }
void Default_Handler58(void) { Default_HandlerCode = 58; panic_leds_local(PANIC_CODE); }
void Default_Handler59(void) { Default_HandlerCode = 59; panic_leds_local(PANIC_CODE); }
void Default_Handler60(void) { Default_HandlerCode = 60; panic_leds_local(PANIC_CODE); }
void Default_Handler61(void) { Default_HandlerCode = 61; panic_leds_local(PANIC_CODE); }
void Default_Handler62(void) { Default_HandlerCode = 62; panic_leds_local(PANIC_CODE); }
void Default_Handler63(void) { Default_HandlerCode = 63; panic_leds_local(PANIC_CODE); }
void Default_Handler64(void) { Default_HandlerCode = 64; panic_leds_local(PANIC_CODE); }
void Default_Handler65(void) { Default_HandlerCode = 65; panic_leds_local(PANIC_CODE); }
void Default_Handler66(void) { Default_HandlerCode = 66; panic_leds_local(PANIC_CODE); }
void Default_Handler67(void) { Default_HandlerCode = 67; panic_leds_local(PANIC_CODE); }
void Default_Handler68(void) { Default_HandlerCode = 68; panic_leds_local(PANIC_CODE); }
void Default_Handler69(void) { Default_HandlerCode = 69; panic_leds_local(PANIC_CODE); }
void Default_Handler70(void) { Default_HandlerCode = 70; panic_leds_local(PANIC_CODE); }
void Default_Handler71(void) { Default_HandlerCode = 71; panic_leds_local(PANIC_CODE); }
void Default_Handler72(void) { Default_HandlerCode = 72; panic_leds_local(PANIC_CODE); }
void Default_Handler73(void) { Default_HandlerCode = 73; panic_leds_local(PANIC_CODE); }
void Default_Handler74(void) { Default_HandlerCode = 74; panic_leds_local(PANIC_CODE); }
void Default_Handler75(void) { Default_HandlerCode = 75; panic_leds_local(PANIC_CODE); }
void Default_Handler76(void) { Default_HandlerCode = 76; panic_leds_local(PANIC_CODE); }
void Default_Handler77(void) { Default_HandlerCode = 77; panic_leds_local(PANIC_CODE); }
void Default_Handler78(void) { Default_HandlerCode = 78; panic_leds_local(PANIC_CODE); }
void Default_Handler79(void) { Default_HandlerCode = 79; panic_leds_local(PANIC_CODE); }
void Default_Handler80(void) { Default_HandlerCode = 80; panic_leds_local(PANIC_CODE); }
void Default_Handler81(void) { Default_HandlerCode = 81; panic_leds_local(PANIC_CODE); }
