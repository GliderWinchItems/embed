/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : ad7799_packetize.c
* Hacker	     : deh
* Date First Issued  : 09/04/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Buffering/unbuffering AD7799 filtered readings
*******************************************************************************/


#include "p1_common.h"


/* Address this routine will go to upon completion */
void 	(*rtc_ad7799_packetizedone_ptr)(void);// Address of function to call to go to for further handling under RTC interrupt

/* Array of tension packet buffers */
struct PKT_AD7799 ad7799_buf[PKT_AD7799_BUF_CT];

/* Indices for filling packets under interrupt */
static unsigned short usPktIdx;		// Index of entry within a packet
static unsigned short usIntIdx;		// Index for packet being filled (interrupt)

/* Indices for retrieving buffered packets */
static unsigned short usMainIdx;	// Index for packet being emptied (main's write to sd)
static unsigned short usMonIdx;		// Index for packet being emptied (monitor)
static unsigned short usShutIdx;	// Index for packet being emptied (shutdown reset)

unsigned short usAD7799filterFlagPrev;	// Filter flag increments; this is the previous count

/******************************************************************************
 * void ad7799_packetize_init(void);
 * @brief	: Set id into array (so we don't have to do it each time later)
 ******************************************************************************/
void ad7799_packetize_init(void)
{
	int i;
	for (i = 0; i < PKT_AD7799_BUF_CT; i++)
	{
		ad7799_buf[i].id = PKT_AD7799_ID;
	}
	return;
}
/******************************************************************************
 * void ad7799_packetize_add(void);
 * @brief	: Add a AD7799 reading to the packet buffer (enter from RTC interrupt chain)
 ******************************************************************************/
void ad7799_packetize_add(void)
{
//char vv[32]; unsigned int nn;

	if (usAD7799filterFlag != usAD7799filterFlagPrev)	// Is there a filtered reading ready?
	{ // Here yes.  Packetize the readings
		usAD7799filterFlagPrev = usAD7799filterFlag;	// Update ready flag counter

		if (usPktIdx == 0)	// Are we about to fill the first tension reading slot?
		{ // Here, yes.  Add the RTC tick counter to the packet
			ad7799_buf[usIntIdx].U.ull =  strAlltime.SYS.ull;	// Add extended linux format time
// Debugging: tick phase
//nn = strAlltime.SYS.ull & 2047;
//sprintf(vv,"%4u %4u\n\r", (nn >> 5), (nn & 31) );
//USART1_txint_puts(vv); USART1_txint_send();
		}
		/* Add filtered tension.  Scaling takes care of the filter gain */
		ad7799_buf[usIntIdx].tension[usPktIdx] =llAD7799_out >> 18; // Add the tension reading

		usPktIdx++;		// Advance index of entries within a packet
		if (usPktIdx >= PKT_AD7799_TENSION_SIZE)// End of this packet?
		{ // Here, yes.
			usPktIdx = 0;			// Reset index for tension readings
			usIntIdx++;			// Advance to next packet
			if (usIntIdx >= PKT_AD7799_BUF_CT)// End of packet buffers?
			{ // Here, yes.
				usIntIdx = 0;		// Reset packet buffer index to beginning
			}
		}
	}
	/* If the address for the next function is not set, simply return. */
	if (rtc_ad7799_packetizedone_ptr != 0)	// Having no address for the following is bad.
		(*rtc_ad7799_packetizedone_ptr)();	// Go do something (e.g. poll the AD7799)
	return;
}
/******************************************************************************
 * static struct PKT_PTR ad7799_packetize_get_x(unsigned short *pusIdx);
 * @brief	: Get pointer & count to the buffer to be drained.
 * @param	: Pointer to index into packets (usMainIdx, usMonIdx, usShutIdx)
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
static struct PKT_PTR ad7799_packetize_get_x(unsigned short *pusIdx)
{
	struct PKT_PTR pp = {0,0};		// This holds the return values

	if (*pusIdx == usIntIdx) return pp;	// Return showing no new data
	pp.ptr = (char *)&ad7799_buf[*pusIdx]; 	// Set pointer
	pp.ct  = sizeof (struct PKT_AD7799);	// Set count
	*pusIdx += 1;				// Advance index for main
	if (*pusIdx >= PKT_AD7799_BUF_CT) 	// Wrap-around?
	{ // Here, yes.
		*pusIdx = 0;			// Reset index to beginning.
	}	
	return pp;				// Return with pointer and count
}

/******************************************************************************
 * struct PKT_PTR ad7799_packetize_get_main(void);
 * @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR ad7799_packetize_get_main(void)
{
	return	ad7799_packetize_get_x(&usMainIdx);		
}
/******************************************************************************
 * struct PKT_PTR ad7799_packetize_get_monitor(void);
 * @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR ad7799_packetize_get_monitor(void)
{
	return	ad7799_packetize_get_x(&usMonIdx);
}
/******************************************************************************
 * struct PKT_PTR ad7799_packetize_get_shutdown(void);
 * @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR ad7799_packetize_get_shutdown(void)
{
	return	ad7799_packetize_get_x(&usShutIdx);		
}

