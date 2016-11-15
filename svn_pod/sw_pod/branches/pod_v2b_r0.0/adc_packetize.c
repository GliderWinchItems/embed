/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : adc_packetize.c
* Hacker	     : deh
* Date First Issued  : 09/06/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Buffering/unbuffering adc averaged readings
*******************************************************************************/
/*
This routine filters with decimation adc readings and builds a packet in a buffer along with the
time stamp (in the form of the rtc tickcounter) that is later written to the SD card.

The accelerometer data are handled differently than the battery & thermistor data.  The former are 
filtered with the same type of filter as used with the tension data (measured with the AD7799), 
however the number of bits for one ADC reading is 12 whereas the AD7799 generates a 24 bit reading.  
The filtering algoritm adds 18 bits to the largest input reading.  Consequently, the arithmetic 
for filtering the adc readings will fit into a 32 bit (long, or int) word, whereas as the AD7799 
data requires a 'long long'.

The output rate of the accelerometer data is 16 per sec, arrived at by filter/decimation of 16, followed
by filter/decimation by 8, thus dividing the 2048/sec sampling rate by 128 to give 16 outputs per second.  

The battery and thermistor readings are averaged.  The battery and thermistor data are slowly changing so 
there is no need for a fast output rate.  Given the simplicty of computing an average the data can 
be accumulated to a maximum of 2^20 which would give an output every 512 seconds.  Accumulation of 
65536 readings (followed by a shift right of 16 bits) gives an average with an ouput rate of once 
every 16 seconds, which is a reasonable rate for this data.

The accelerometer data is circular buffered so that any delay in the main loop writing packets to the
SD card will not overrun incoming accelerometer data.  Indices for main and interrupt are used.
Since the data output rate is so slow for the battery and thermistor a single buffer is used.

The filtering of the accelerometer data can be signed.  However, the 9-9-2011 implementation does not
build in the zero offset to that data, so it is only positive.

The battery and thermistor data is not signed.

The adc data is generated continoulsy by the DMA once 'adcpod' is initialized.  The rate is much higher
than the polling rate that executes this routine.  Hence, this routine merely picks up a recent reading
out of the buffer that is being loaded by the DMA with the adc readings.

Note: the thermistor data in this routine is not associated with the 32 KHz time-base versus temperature
adjustment.

*/
/*
Key to locating file with source code--
@1 = svn_pod/sw_pod/trunk/pod_v1/p1_common.h
@2 = svn_pod/sw_stm32/trunk/lib/libsupporstm32/cic_L64_D32.h


*/

#include "p1_common.h"

#include "../../../sw_stm32/trunk/devices/adcpod.h"

/* Address 'adc_packetize_add' routine will go to upon completion */
void 	(*adc_filterdone_ptr)(void);// Address of function to call to go to for further handling under RTC interrupt

/* Packet with accelerometer data that will written to the SD Card */
struct PKT_ACCEL accel_pktbuf[PKT_ACCEL_BUF_CT];
unsigned volatile short	usAccelpktctr;		// Incremented each time a 

/* Array with the latest x,y,z values for monitoring purposes */
static int nAccel_mon_buf[NUMBERACCELREADINGSPKT];

/* Indices for filling accelerometer packets under interrupt */
static volatile unsigned short usIntIdx;		// Index for packet being filled (interrupt)
static volatile unsigned short usPktIdx;		// Index for entries within packet

/* Indices for retrieving buffered accelerometer packets */
static volatile unsigned short usMainIdx;	// Index for packet being emptied (main's write to sd)
static volatile unsigned short usMonIdx;		// Index for array buffer being emptied (monitor)

/* These hold the values for CIC filtering of accelerometer readings */
static struct CICLN2M3 strCIC_A[3];		// Accelerometer X, Y, Z (@2)
static struct CICLN2M3 strCIC_B[3];		// Accelerometer X, Y, Z (@2)
static volatile short usDiscard = ACCELDISCARDNUMBER;	// Initial discard counter for accelerometer readings

/* Packet with battery & thermistor temp that will written to the SD Card */
static struct PKT_BATTMP batttemp_pktbuf;	//
static unsigned short	usBattmppktctr;		// Incremented each time a packet is ready

/* Packet with battery & thermistor temp that is used with PC monitoring */
static struct PKT_BATTMP batttemp_monbuf;	//
static volatile unsigned short	usBattmpmonctr;		// Incremented each time a packet is ready

/* These are compared against 'usBattmpmonctr' to time access to 'batttemp_monbuf' by various nefarious actors */
static volatile unsigned short	usBattmpmonctrprev;	// Previous count 
static volatile unsigned short	usBattmptickctr;	// Previous count 
static volatile unsigned short	usBattmpshutctr;	// Previous count 

/* These hold the values for Averaging battery and thermistor
[0] - Thermister
[1] - Bottom battery cell
[2] - Top of battery */
static volatile int adc_average[NUMBERADCREADINGSPKT];	// Beware of 32b accumulation when changing averaging length
static volatile int nAdcaveragectr;		// Count number accumulated

static volatile int adc_average_monitor[NUMBERADCREADINGSPKT];	// Beware of 32b accumulation when changing averaging length
static volatile int nAdcaveragectrmonitor;	// Count number accumulated for monitoring


static int adc_average_tickadjust;// Beware of 32b accumulation when changing averaging length
static int nAdcaveragectrtickadjust;	// Count number accumulated for monitoring

volatile unsigned int   uiThermistor_tickadjust;	// Scaled adjustment for thermistor reading
volatile unsigned int	uiThermistoradcflag;		// Increments when thermistor reading ready for tickadjust
volatile unsigned int	uiThermistoradcflagPrev;	// Increments when thermistor reading ready for tickadjust
/******************************************************************************
 * void adc_packetize_init(void);
 * @brief	: Set id into array (so we don't have to do it each time later)
 ******************************************************************************/
void adc_packetize_init(void)
{
	int i;
	for (i = 0; i < PKT_ACCEL_BUF_CT; i++)		// Fill buffer of accelerometer packets
	{
		accel_pktbuf[i].id = ID_ACCEL;	// Setup ID for accelerometer packet
	}
	batttemp_pktbuf.id = ID_BATTTEMP;		// Setup ID for battery|thermistor packet
	batttemp_monbuf.id = ID_BATTTEMP;		// Setup ID for battery|thermistor packet

	for (i = 0; i < NUMBERACCELREADINGSPKT; i++)	// Setup decimation counts for filtering
	{
		strCIC_A[i].usDecimateNum = ACCELDECIMATE_A;	// First stage
		strCIC_B[i].usDecimateNum = ACCELDECIMATE_B;	// Second stage
	}

	return;
}
/******************************************************************************
 * void adc_packetize(void);
 * @brief	: Add a ADC reading to the packet buffer
 ******************************************************************************/
void adc_packetize(void)
{

/* ------------------------- Accelerometer readings -------------------------------------- */
	/* Throw out the initial readings so that it doesn't give an impulse to the filter */
	if (usDiscard > 0) 
	{
		usDiscard -=1;
	}
	else
	{
		/* Fetch recent ADC readings (don't worry about which double buffer to read */
		/* The input to the filter is the last good reading from the ADC */
		strCIC_A[0].nIn = strADC1dr.in[0][3];	// PC 0 ADC12 -IN10	Accelerometer X	
		strCIC_A[1].nIn = strADC1dr.in[0][4];	// PC 1 ADC12 -IN11	Accelerometer Y
		strCIC_A[2].nIn = strADC1dr.in[0][5];	// PC 2 ADC12 -IN12	Accelerometer Z
	
		/* Filter each of these readings */	
		cic_filter_l_N2_M3(&strCIC_A[0]);	// Do three stage cascade integrator comb filter X-axis (@2)
		cic_filter_l_N2_M3(&strCIC_A[1]);	// Do three stage cascade integrator comb filter Y-axis (@2)
		cic_filter_l_N2_M3(&strCIC_A[2]);	// Do three stage cascade integrator comb filter Z-axis (@2)
		
		/* Since we do all three of these together we only need to check for one ready flag */
		if (strCIC_A[0].usFlag > 0)	// Is a filtered/decimated output ready? (@2)
		{
			strCIC_A[0].usFlag = 0;	// Reset flag

			/* The output of the first filter is the input for the second filter */
			strCIC_B[0].nIn = strCIC_A[0].lout >> ACCELADCGAIN_A;	// PC 0 ADC12 -IN10	Accelerometer X	
			strCIC_B[1].nIn = strCIC_A[1].lout >> ACCELADCGAIN_A;	// PC 1 ADC12 -IN11	Accelerometer Y
			strCIC_B[2].nIn = strCIC_A[2].lout >> ACCELADCGAIN_A;	// PC 2 ADC12 -IN12	Accelerometer Z
	
			/* Filter each of these readings */	
			cic_filter_l_N2_M3(&strCIC_B[0]);	// Do three stage cascade integrator comb filter X-axis (@2)
			cic_filter_l_N2_M3(&strCIC_B[1]);	// Do three stage cascade integrator comb filter Y-axis (@2)
			cic_filter_l_N2_M3(&strCIC_B[2]);	// Do three stage cascade integrator comb filter Z-axis (@2)


			if (strCIC_B[0].usFlag > 0)	// Is a filtered/decimated output ready? (@2)
			{ // Here, yes.  Go set it up in a packet for logging
				strCIC_B[0].usFlag = 0;	// Reset flag
	
				if (usPktIdx == 0)	// Are we about to fill the first tension reading slot?
				{ // Here, yes.  Add the RTC tick counter to the packet
					accel_pktbuf[usIntIdx].U.ull =  strAlltime.SYS.ull;	// Add extended linux format time

				}
				/* Add filtered tension.  Scaling takes care of the filter gain */
				accel_pktbuf[usIntIdx].adc[0][usPktIdx] = strCIC_B[0].lout >> ACCELADCGAIN_B; // Accelerometer X	
				accel_pktbuf[usIntIdx].adc[1][usPktIdx] = strCIC_B[1].lout >> ACCELADCGAIN_B; // Accelerometer Y
				accel_pktbuf[usIntIdx].adc[2][usPktIdx] = strCIC_B[2].lout >> ACCELADCGAIN_B; // Accelerometer Z
	
				accel_pktbuf[usIntIdx].U.ull = strAlltime.SYS.ull;	// Setup time stamp

				nAccel_mon_buf[0] = strCIC_B[0].lout >> ACCELADCGAIN_B; // Accelerometer X
				nAccel_mon_buf[1] = strCIC_B[1].lout >> ACCELADCGAIN_B; // Accelerometer Y
				nAccel_mon_buf[2] = strCIC_B[2].lout >> ACCELADCGAIN_B; // Accelerometer Z

				
				usMonIdx += 1;	// Show mainline polling loop the monitoring data are ready.
	
				usPktIdx++;		// Advance index for entries within a packet
				if (usPktIdx >= PKT_ACCEL_GRP_SIZE)// End of entries?
				{ // Here, yes. 
					usPktIdx = 0;			// Reset index
					usIntIdx++;			// Advance to next packet
					if (usIntIdx >= PKT_ACCEL_BUF_CT)// End of packet buffers?
					{ // Here, yes.
						usIntIdx = 0;		// Reset packet buffer index to beginning
					}
				}
			}
		}
	}

/* ------------------------- Battery & Thermistor readings -------------------------------- */
	/* Accumulate readings for Thermistor, Bottom Cell, Top cell of battery */

	/* Accumulate readings for packet that writes to SD card */
	adc_average[0] += strADC1dr.in[0][0];	// Thermistor on 32 KHz xtal
	adc_average[1] += strADC1dr.in[0][1];	// Bottom cell of battery
	adc_average[2] += strADC1dr.in[0][2];	// Top of battery	

	nAdcaveragectr += 1;				// Accumulation counter
	if (nAdcaveragectr >= ADCNUMBERINAVERAGE)	// Accumulated enough?
	{ // Here, yes.  Scale to less than 16 bits and store as a short
		nAdcaveragectr = 0;				// Accumulation counter

		batttemp_pktbuf.U.ull =  strAlltime.SYS.ull;	// Add extended linux format time

		batttemp_pktbuf.adc[0] =  adc_average[0] >> (ADCAVERAGEORDER-ADCAVERAGESCALING);// Thermistor on 32 KHz xtal
		batttemp_pktbuf.adc[1] =  adc_average[1] >> (ADCAVERAGEORDER-ADCAVERAGESCALING);// Bottom cell of battery	
		batttemp_pktbuf.adc[2] =  adc_average[2] >> (ADCAVERAGEORDER-ADCAVERAGESCALING);// Top of battery

		/* Zero out accumulators to prepare for next average */
		adc_average[0] = 0;
		adc_average[1] = 0;
		adc_average[2] = 0;

		usBattmppktctr += 1;			// Flag that new data are ready
	}

	/* Accumulate readings for Thermistor, Bottom Cell, Top cell of battery for monitoring */
	adc_average_monitor[0] += strADC1dr.in[0][0];	// Thermistor on 32 KHz xtal
	adc_average_monitor[1] += strADC1dr.in[0][1];	// Bottom cell of battery
	adc_average_monitor[2] += strADC1dr.in[0][2];	// Top cell of battery	

	nAdcaveragectrmonitor += 1;				// Accumulation counter
	if (nAdcaveragectrmonitor >= (1<<ADCORDERINAVERAGEMONITOR))	// Accumulated enough 
	{
		nAdcaveragectrmonitor  = 0;			// Accumulation counter

		/* Scale to make average, and save in a buffer for monitoring */
		batttemp_monbuf.U.ull = strAlltime.SYS.ull;	// Add extended linux format time

		batttemp_monbuf.adc[0] =  adc_average_monitor[0] >> (ADCORDERINAVERAGEMONITOR);	// Thermistor on 32 KHz xtal
		batttemp_monbuf.adc[1] =  adc_average_monitor[1] >> (ADCORDERINAVERAGEMONITOR);	// Bottom cell of battery	
		batttemp_monbuf.adc[2] =  adc_average_monitor[2] >> (ADCORDERINAVERAGEMONITOR);	// Top cell of batter

		/* Zero out accumulators to prepare for next average */
		adc_average_monitor[0] = 0;
		adc_average_monitor[1] = 0;
		adc_average_monitor[2] = 0;

		usBattmpmonctr  += 1;			// Flag that new data are ready
	}

	/* Accumulate an average of the thermistor for 'tickadjust.c' use */
	adc_average_tickadjust += strADC1dr.in[0][0];	// Thermistor on 32 KHz xtal
	nAdcaveragectrtickadjust += 1;		// Count number accumulated
	if (nAdcaveragectrtickadjust >= (1 << ADCTICKADJUSTAVER_ORDER) )	// Accumulated enough 
	{
		nAdcaveragectrtickadjust = 0;	// Reset averaging counter
		uiThermistor_tickadjust = (adc_average_tickadjust >> ADCTICKADJUSTAVER_ORDER) ; // Scale and save reading for 'tickadjust.c'
		adc_average_tickadjust = 0;	// Reset accumulator for next averaging
		uiThermistoradcflag += 1;	// Show 'tickadjust.c' reading is ready
	}


	/* Call another routine in the "chain" if the address has been set up */
	if (adc_filterdone_ptr != 0)		// Having no address for the following is bad.
		(*adc_filterdone_ptr)();	// Go do something

	return;		// Here, end of chain
}
/******************************************************************************
 * struct PKT_PTR adc_packetize_get_accel(void);
 * @brief	: Get pointer & count to the buffer to be drained--accelerometer readings
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR adc_packetize_get_accel(void)
{
	struct PKT_PTR pp = {0,0};	// This holds the return values

	if (usMainIdx == usIntIdx) return pp;	// Return showing no new data
	
	/* Here, there is data buffered */
	pp.ptr = (char *)&accel_pktbuf[usMainIdx]; 	// Set pointer
	pp.ct  = sizeof (struct PKT_ACCEL);		// Set count
	usMainIdx++;				// Advance index for main
	if (usMainIdx >= PKT_ACCEL_BUF_CT) 	// Wrap-around?
	{ // Here, yes.
		usMainIdx = 0;			// Reset index to beginning.
	}	
	return pp;				// Return with pointer and count
}
/******************************************************************************
 * int * adc_packetize_get_accel_monitor(void);
 * @brief	: Get pointer & count to the buffer to be drained--accelerometer readings
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
int * adc_packetize_get_accel_monitor(void)
{
	if (usMonIdx == 0) return 0;	// Return showing no new data
	usMonIdx = 0;
	return &nAccel_mon_buf[0];	// Return pointer
}
/******************************************************************************
 * struct PKT_PTR adc_packetize_get_battmp_monitor(void);
 * @brief	: Get pointer & count to the buffer to be drained--battery & thermistor readings
 ******************************************************************************/
struct PKT_PTR adc_packetize_get_battmp_monitor(void)
{
	struct PKT_PTR pp = {0,0};	// This holds the return values

	/* Is there a new packet ready? */
	if (usBattmpmonctr == usBattmpmonctrprev) return pp; // Return no.
	usBattmpmonctrprev = usBattmpmonctr;

	/* Here, the packet has been loaded with new data */
	pp.ptr = (char *)&batttemp_monbuf;	// Set pointer
	pp.ct  = sizeof (struct PKT_BATTMP);	// Set count
	return pp;				// Return with pointer and count 
}
/******************************************************************************
 * struct PKT_PTR adc_packetize_get_battmp_shutdown(void);
 * @brief	: Get pointer & count to the buffer to be drained--battery & thermistor readings
 ******************************************************************************/
struct PKT_PTR adc_packetize_get_battmp_shutdown(void)
{
	struct PKT_PTR pp = {0,0};	// This holds the return values

	/* Is there a new packet ready? */
	if (usBattmpmonctr == usBattmpshutctr) return pp; // Return no.
	usBattmpshutctr = usBattmpmonctr;

	/* Here, the packet has been loaded with new data */
	pp.ptr = (char *)&batttemp_monbuf;	// Set pointer
	pp.ct  = sizeof (struct PKT_BATTMP);	// Set count
	return pp;				// Return with pointer and count 
}
/******************************************************************************
 * struct PKT_PTR adc_packetize_get_battmp(void);
 * @brief	: Get pointer & count to the buffer to be drained--battery & thermistor readings
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR adc_packetize_get_battmp(void)
{
	struct PKT_PTR pp = {0,0};	// This holds the return values

	/* Is there a new packet ready? */
	if (usBattmppktctr == 0) return pp; // Return no.
	usBattmppktctr = 0;

	/* Here, there the packet has been loaded with new data */
	pp.ptr = (char *)&batttemp_pktbuf;		// Set pointer
	pp.ct  = sizeof (struct PKT_BATTMP);	// Set count
	return pp;				// Return with pointer and count 
	
}
/******************************************************************************
 * struct PKT_PTR adc_packetize_get_battmp_tickadujst(void);
 * @brief	: Get pointer & count to the buffer to be drained--battery & thermistor readings
 ******************************************************************************/
struct PKT_PTR adc_packetize_get_battmp_tickadujst(void)
{
	struct PKT_PTR pp = {0,0};	// This holds the return values

	/* Is there a new packet ready? */
	if (usBattmpmonctr == usBattmptickctr) return pp; // Return no.
	usBattmptickctr = usBattmpmonctr;

	/* Here, the packet has been loaded with new data */
	pp.ptr = (char *)&batttemp_monbuf;	// Set pointer
	pp.ct  = sizeof (struct PKT_BATTMP);	// Set count
	return pp;				// Return with pointer and count 
}

