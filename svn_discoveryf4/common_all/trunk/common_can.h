/******************************************************************************
* File Name          : common_can.h
* Date First Issued  : 12/27/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : includes "common" for CAN for all sensor programs
*******************************************************************************/
/*
10/16/2013 Copy of svn_sensor/common_all/trunk/common_can.h (svn_sensor rev 259)
*/

/*
Note: '#define CANOPEN' below separates two sections.  The first is the original 
 bit assignment scheme.  The 2nd is for CANOpen compatibility.  The first section
 may no longer compile/work correctly.  When the changes for CANOpen compatibility
 were there was no testing the reverting to the original scheme compiles and works
 correctly.
*/

#ifndef __COMMON_CAN_SENSOR
#define __COMMON_CAN_SENSOR

#include "../../../../svn_common/trunk/common_can.h"

#ifdef SKIPTHEFOLLOWINGCODECOMMON_CANF4

#include "libopencm3/cm3/common.h"
#include "common_misc.h"

#define APPORG	0x08004000
#define APPJUMP (APPORG + 4)

/*
ID/arbitration word (p 659 (tx), 662 (rx))
Standard ID: 11 bits
Extended ID: 29 bits

ID Bits [31:27]
Unit -- 5 bits associated with the unit
    GPS (time sync, date/time/tick distribution)
1   Sensor 1 (upper sheave) (shaft counter, speed)
2   Sensor 2 (lower sheave) (shaft counter, speed)
3   Sensor 3 (drive shaft)  (shaft counter, speed)
4   Sensor 4 (engine) (manifold pressure, RPM, throttle position)
5   Sensor 5 (level wind) (motor current)
6   Sensor 6 (Xbee 1) (tension, signal level)
7   Sensor 7 (Xbee 2) (tension, signal level)
8   Logger   
...
31  

ID Bits [26:21]	6 bits associated with the data (8 bytes)
Data type identification -- 

For the extended ID, the additional 18 bits are used for 
the program load address--
ID Bits [20:3]
Program load addresses, or sequence number for other multi-msg data payloads
*/
/* Interrupt priorities - lower numbers are higher priority.  */
#define TIM4_PRIORITY				0x20	//   ### Higher than CAN_RX1 (FIFO 1)
#define NVIC_CAN_RX1_IRQ_PRIORITY		0x30	//   Receive FIFO 1 (and related) [time sync]
#define ADC1_2_PRIORITY_FOTO			0x40	//   Interrupt priority for ADC1 or ADC2 interrupt
#define SYSTICK_PRIORITY_SE 			0X50	//   ### Lower than CAN_RX1 (FIFO 1)
#define TIM4_PRIORITY_SE			0x60	//   RPM sensors for engine.
#define NVIC_FSMC_PRIORITY			0x70	// * ### Lower than SYSTICK (ADC filtering after DMA interrupt)
#define NVIC_CAN_SCE_IRQ_PRIORITY		0x70	//   Errors & status change
#define NVIC_USB_HP_CAN_TX_IRQ_PRIORITY		0x70	//   Transmit FIFO
#define NVIC_USB_LP_CAN_RX0_IRQ_PRIORITY	0x70	//   Receive FIFO 0 (and related)
#define DMA1_CH1_PRIORITY			0x80	//   DMA 1/2 and end of buffer interrupt for adc
#define NVIC_I2C1_EV_IRQ_PRIORITY		0x90	// * Post CAN FIFO 1 interrupt processing
#define NVIC_I2C1_ER_IRQ_PRIORITY		0xa0	// * Post CAN FIFO 0 interrupt processing
#define NVIC_I2C2_EV_IRQ_PRIORITY 		0xb0	//   Post
#define NVIC_I2C2_ER_IRQ_PRIORITY		0xe0	// * Post can_buff interrupt processing

#define CANBAUDRATE	500000	// Name says it all

#define CAN_IDE	0x4	// IDE bit in CAN msg
#define CAN_RTR 0x2	// RTR bit in CAN msg

/* ============================================================================================================================= */
#define CANOPEN		// CAN Open compatibiilty #define's
#ifndef CANOPEN
/* =============================================================================================================================
   Original, pre-CAN Open compatibility #defines
   ============================================================================================================================= */

/* Number of bits in each standard ID field (11 bits) */
#define CAN_UNITID_NUMBITS	5	// Allow 31 unit id's
#define CAN_DATAID_NUMBITS	3	// Allow 8 data types (for each unit id) packet/logging
#define CAN_EXTRID_NUMBITS	3	// Extra bits in ID not packed/logged

/* Use extended ID for sending program loading data.  The extended ID carries 29 bits.  The first 
   11 bits are identical in position used with the standard ID.  
   The extension adds bits that are used to specify the 8 byte boundary address for the 8 bytes 
   of program load data.  It can also be used for future sending/receiving data where multiple 
   messages of 8 bytes are required. */
#define CAN_ADDR_NUMBITS	(29-CAN_UNITID_NUMBITS-CAN_DATAID_NUMBITS) // Remaining bits

/* Number of bits to shift field--scale bit = 0, so ID goes in low two bytes of 32b filter register  */
#define CAN_UNITID_SHIFT	(32 - CAN_UNITID_NUMBITS)

/* Number of bits to shift for DATAID */
#define CAN_DATAID_SHIFT (32 - (CAN_UNITID_NUMBITS + CAN_DATAID_NUMBITS) ) 

/* Number of bits to shift for EXTRID */
#define CAN_EXTRID_SHIFT (32 - (CAN_UNITID_NUMBITS + CAN_DATAID_NUMBITS + CAN_EXTRID_NUMBITS) ) 

#define CAN_PKTID_SHIFT (CAN_DATAID_SHIFT)

/* Mask for the fields--scale bit = 0, so mask goes in high two bytes of 32b filter register */
#define CAN_UNITID_MASK  ( ((1 << CAN_UNITID_NUMBITS) - 1) << (32 - CAN_UNITID_NUMBITS) )

/* Mask id field: data to be logged */
#define CAN_DATAID_MASK	 ( ((1 << CAN_DATAID_NUMBITS) - 1 ) << CAN_DATAID_SHIFT )

/* Mask to look at extra ID bits */
#define CAN_EXTRID_MASK  ( ((1 << CAN_EXTRID_NUMBITS) - 1 ) << CAN_EXTRID_SHIFT )

/* Combined data ID mask */
#define (CAN_DATAID_MASK | CAN_EXTRID_MASK)

/* High priority msgs.  (Lower numbers have higher arbitration priority.)  Filter match index in 'x' after '//' */
#define CAN_RESETALL	((u32)(0 << CAN_UNITID_SHIFT) ) 	// '0' Cause units to reset and go into startup/program loader
#define CAN_TIMESYNC1	((u32)(1 << CAN_UNITID_SHIFT) ) 	// '1' GPS time sync distribution msg (CAN setup for no error retry - NART in CAN_MCR, p 649)
#define CAN_TIMESYNC2	((u32)(2 << CAN_UNITID_SHIFT) ) 	// '2' Reserve for multiple time sync's or high priority msg
#define CAN_TIMESYNC3	((u32)(3 << CAN_UNITID_SHIFT) ) 	// '3' Reserve for multiple time sync's or high priority msg

/* Data ID's (0 - 7) for high priority IDs */
#define CAN_DATAID_MASTERRESET		0	// Execute a software forced RESET
#define CAN_DATAID_WAITDELAYRESET	1	// Reset the wait delay in the first level loader (after RESET) (to prevent sending CAN msgs)

/* Except for the CO the units only accept msgs with the following ID 'and' the CAN_REQ_BIT off */
/* These units send msgs with the same ID, but with the CAN_REQ_BIT on */
/* The receiver, receiving a msg with CAN_REQ bit = 0 responds and sends the requested dataid with CAN_REQ = 1 */
/* The receiver, receiving a msg with CAN_REQ bit = 1 disposes of the data in the msg */

/* The following msgs with these 5 bit UNITD will be logged by the logger */
#define CAN_MSG_NUMIDLOGGED	20	// Number of IDs in block that will be logged
#define CAN_MSG_BS		4	// Block Start: First ID for block of message ID
#define CAN_UNITID_SE1	  ((u32)(CAN_MSG_BS + 0)  << CAN_UNITID_SHIFT)	// Sensor unit: Engine 
#define CAN_UNITID_SE2	  ((u32)(CAN_MSG_BS + 1)  << CAN_UNITID_SHIFT)	// Sensor unit: Drive shaft encoder
#define CAN_UNITID_SE3	  ((u32)(CAN_MSG_BS + 2)  << CAN_UNITID_SHIFT)	// Sensor unit: Lower sheave shaft encoder
#define CAN_UNITID_SE4	  ((u32)(CAN_MSG_BS + 3)  << CAN_UNITID_SHIFT)	// Sensor unit: Upper sheave shaft encoder
#define CAN_UNITID_LVL	  ((u32)(CAN_MSG_BS + 4)  << CAN_UNITID_SHIFT)	// Sensor unit: Level wind
#define CAN_UNITID_XB1	  ((u32)(CAN_MSG_BS + 5)  << CAN_UNITID_SHIFT)	// Sensor unit: XBee receiver #1
#define CAN_UNITID_XB2	  ((u32)(CAN_MSG_BS + 6)  << CAN_UNITID_SHIFT)	// Sensor unit: XBee receiver #2
#define CAN_UNITID_DSP	  ((u32)(CAN_MSG_BS + 7)  << CAN_UNITID_SHIFT)	// Display driver/console
#define CAN_UNITID_OLICAW ((u32)(CAN_MSG_BS + 8)  << CAN_UNITID_SHIFT)	// CAW's Olimex board
#define CAN_UNITID_POD6   ((u32)(CAN_MSG_BS + 9)  << CAN_UNITID_SHIFT)	// POD board sensor prototype ("6" marked on board)

#define CAN_MSG_BE	(CAN_MSG_BS + CAN_MSG_NUMIDLOGGED - 1)	// Block End:   Last ID for block of message ID

#define CAN_MSG_TOTALLOGGED	(CAN_MSG_NUMIDLOGGED * CAN_DATAID_NUMBITS)	// This many different UNITID w DATA types loggable

#define CAN_UNITID_GPS	(29  << CAN_UNITID_SHIFT)	//	GPS sentences are logged and identified by a fake CAN unit id

/* The following will not have their msgs logged */
#define CAN_UNITID_CO	  ((u32)(CAN_MSG_BE + 1)  << CAN_UNITID_SHIFT)	// CO
#define CAN_UNITID_POD1	  ((u32)(CAN_MSG_BE + 2)  << CAN_UNITID_SHIFT)	// POD board hack prototype #1
#define CAN_UNITID_CO_OLI ((u32)(CAN_MSG_BE + 3)  << CAN_UNITID_SHIFT)	// Olimex CO
#define CAN_UNITID_GPSDT  ((u32)(CAN_MSG_BE + 4)  << CAN_UNITID_SHIFT)	// GPS Date/Time/tick distribution
#define CAN_UNITID_GATE1  ((u32)(CAN_MSG_BE + 5)  << CAN_UNITID_SHIFT)	// PC<->CAN bus gateway
#define CAN_UNITID_LDR    ((u32)(CAN_MSG_BE + 6)  << CAN_UNITID_SHIFT)	// Fixed loader (when no valid eeprom unitid is present)

/* Data ID's (0 - 7) for units */
#define CAN_DATAID_POD6_SHAFT			0
#define CAN_DATAID_POD6_SPEED			1
#define CAN_DATAID_SE1_THROTTLEandTHERMISTOR	0
#define CAN_DATAID_SE1_PRESSUREandRPM		1
#define CAN_DATAID_SEN_COUNTERandSPEED		0

#define CAN_DATAID_ERROR2			6
#define CAN_DATAID_ERROR1			7
	
/* Extra data ID's (0 - 7) for units */
// Program loading--
#define CAN_EXTRID_DATA_NULL		0	// Not defined, since CAN_DATAID = 0 could include CAN_EXTRID = 0 
#define CAN_EXTRID_DATA_RD		1	// Read data (dlc holds byte ct: 1-8)(response payload bytes 1-8: start at current address)
#define CAN_EXTRID_DATA_WR		2	// Write data dlc holds byte ct: 1-8)(
//#define CAN_EXTRID_DATA_CMD		CAN_TYPE_XXX2	// 13 Command (1st byte of payload holds the command)
#define CAN_EXTRID_DATA_CMD		0	// CMD and basic CAN ID are the same

	/* Command codes--bytes 2-8 hold a 4 byte address, (or something else of dear value) */		
	#define LDR_SET_ADDR	1	// 5 Set address pointer (not FLASH) (bytes 2-5):  Respond with last written address.
	#define LDR_SET_ADDR_FL	2	// 5 Set address pointer (FLASH) (bytes 2-5):  Respond with last written address.
	#define LDR_CRC		3	// 8 Get CRC: 2-4 = count; 5-8 = start address 
	#define LDR_ACK		4	// 1 ACK: Positive acknowledge (Get next something)
	#define LDR_NACK	5	// 1 NACK: Negative acknowledge (So? How do we know it is wrong?)
	#define LDR_JMP		6	// 5 Jump: to address supplied (bytes 2-5)
	#define LDR_WRBLK	7	// 1 Done with block: write block with whatever you have.
	#define LDR_RESET	8	// 1 RESET: Execute a software forced RESET
	#define LDR_XON		9	// 1 Resume sending
	#define LDR_XOFF	10	// 1 Stop sending
// **** MORE ?



/* Never-use--16b Mask|ID that will never be used/assigned (for filling an unassigned odd filter bank register) */
#define CAN_NEVERUSEID	(0xffffffff)	// Requires all bits to to match, for msg id 65535.

// This one covers most cases for a 8 byte payload--
#define CAN_DATAID_CT	(0 << CAN_DATAID_MASK)	

/* Unit    (number data types) */
// Units 2,3,4: (2) Shaft counter and computed speed,
// Unit 1:      (3) manifold pressure, rpm, and throttle position
// Unit 5:      (1) level-wind motor current
// Unit 6,7:    (2) Tension Telemetry
// Unit 8:      (4) Four 16 bit display values


/* The following covers all the possible messages in the foregoing.  The unit with the following mask can 
   deal with all messages to/from units */
#define CAN_AGGREGATOR	(CAN_UNITID_MASK | CAN_DATAID_MASK)

/* Filter number  (0 - 13) */
#define	CAN_FILTERBANK_RESETALL		0	// Standard: All units have the reset msg  		bank 0 reg 1
#define	CAN_FILTERBANK_TIMESYNC1	1	// Standard: All units have the first time sync 	bank 0 reg 2
#define	CAN_FILTERBANK_TIMESYNC2	2	// Standard: All units have the alternate time sync	bank 1 reg 1
#define	CAN_FILTERBANK_TIMESYNC3	3	// Standard: All units have the another alternate sync	bank 1 reg 2
#define	CAN_FILTERBANK_UNIT		4	// Standard: All units have their ID mask  		bank 2 reg 1

/* ============================================================================================================================= */
#else
/* =============================================================================================================================
   CAN Open compatibility #defines
   ============================================================================================================================= */

/* Number of bits in each standard ID field (11 bits).  CAN Open splits these into two main fields, of OD and msg type. */
// 32 bit ID word
//  pppp xxxx xxxz zzzz zzzz zzzz zzzz zirt
//
//  p ( 4) = priority group
//  x ( 7) = unit id group
//    (11) = total bits: standard CAN id
//  z (18) = extended id
//    (29) = total bits: extended id
//
//  i (1) = IDE bit
//          1 = extended CAN id
//          0 
//            if z's are 0, extended CAN id
//            if z's are not 0, a non-CAN-bus id
//   r (1) = RTR bit
//   t (1) = transmit bit, not used for CAN msg
//
//

/* Seven Unit ID bits (0 - 6) */
#define CAN_UNITID_NUMBITS	7	// Allow 127 unit id's (0 not a unit id)

/* Four Type bits (7 - 10) */
#define CAN_DATATYPE_NUMBITS	4	// Upper bits 
#define CAN_DATAID_NUMBITS	CAN_DATATYPE_NUMBITS

#define CAN_TYPE_NMT		0	// 
#define CAN_TYPE_EanT		1	// Emergency TX (w unitid), sync TX (unitd = 0)
#define CAN_TYPE_XXX1		2	// Unassigned for CANOpen
#define CAN_TYPE_PDO1tx		3	// 
#define CAN_TYPE_PDO1rx		4	// 
#define CAN_TYPE_PDO2tx		5	// 
#define CAN_TYPE_PDO2rx		6	// 
#define CAN_TYPE_PDO3tx		7	// 
#define CAN_TYPE_PDO3rx		8	// 
#define CAN_TYPE_PDO4tx		9	// 
#define CAN_TYPE_PDO4rx		10	// 
#define CAN_TYPE_SDOtx		11	//
#define CAN_TYPE_SDOrx		12	// 
#define CAN_TYPE_XXX2		13	// Unassigned for CANOpen
#define CAN_TYPE_		14	// NMT tx + unit id
#define CAN_TYPE_LSS		15	// Includes LSS

/* Special cases */
#define CAN_LSStx	0x734
#define CAN_LSSrx	0x735

/* Data types to be logged (if unitid is in the loggable range) */
#define ifloggabledatatype(a) if((a>=CAN_TYPE_XXX1)&&(a<=CAN_TYPE_PDO4rx)) // See 'co1_Olimex/can_log.c'

/* Extended addressing adds low order bits in the ID that can be used. */
#define CAN_EXTID_NUMBITS	(29-CAN_UNITID_NUMBITS-CAN_DATAID_NUMBITS) // Remaining bits

/* Number of bits to shift field to position in stm32 32b register.  */
#define CAN_UNITID_SHIFT	(32 - CAN_UNITID_NUMBITS - CAN_DATATYPE_NUMBITS)	// Unit ID
#define CAN_DATATYPE_SHIFT 	(32 - CAN_DATATYPE_NUMBITS) // Data type
#define CAN_DATAID_SHIFT	CAN_DATATYPE_SHIFT	// Alias
#define CAN_PKTID_SHIFT 	CAN_DATATYPE_SHIFT	// Alias
#define CAN_EXTRID_SHIFT	CAN_DATATYPE_SHIFT	// Alias

/* Mask for the filter fields--scale bit = 0 (16b), so mask goes in high two bytes of 32b filter register */
#define CAN_UNITID_MASK  ( ((1 << CAN_DATATYPE_NUMBITS) - 1) << (CAN_UNITID_SHIFT) )

/* Mask id field: data to be logged */
#define CAN_DATATYPE_MASK	 ( ((1 << CAN_DATATYPE_NUMBITS) - 1 ) << CAN_DATATYPE_SHIFT )
#define CAN_EXTRID_MASK		CAN_DATATYPE_MASK	// Alias
#define CAN_DATAID_MASK		CAN_DATATYPE_MASK	// Alias

/* High priority msgs.  (Lower numbers have higher arbitration priority.)  Filter match index in 'x' after '//' */
#define CAN_RESETALL	((u32)(0 << CAN_UNITID_SHIFT) ) 	// '0' Cause units to reset and go into startup/program loader
#define CAN_TIMESYNC1	((u32)(1 << CAN_UNITID_SHIFT) ) 	// '1' GPS time sync distribution msg (CAN setup for no error retry - NART in CAN_MCR, p 649)
#define CAN_TIMESYNC2	((u32)(2 << CAN_UNITID_SHIFT) ) 	// '2' Reserve for multiple time sync's or high priority msg
#define CAN_TIMESYNC3	((u32)(3 << CAN_UNITID_SHIFT) ) 	// '3' Reserve for multiple time sync's or high priority msg
#define CAN_SQUELCH	CAN_TIMESYNC3	// Stop sending msgs

/* Data ID's (0 - 7) for high priority IDs */
#define CAN_DATAID_MASTERRESET		(CAN_TYPE_PDO4rx << CAN_DATATYPE_SHIFT)	// Execute a software forced RESET

// The wait delay msg: dlc = 8; ui[0] ticks to squelch sending; ui[1] CAN id to NOT squelch 
#define CAN_DATAID_WAITDELAYRESET	(CAN_TYPE_PDO1tx << CAN_DATATYPE_SHIFT)	// Reset the wait delay in the first level loader (after RESET) (to prevent sending CAN msgs)
#define CAN_WAITDELAYMAX	(10*2048)	// Number of 2048 per sec ticks to squelch sending CAN msgs

/* Loggable ID's */
/* Except for the CO the units only accept msgs with the following ID 'and' the CAN_REQ_BIT off */
/* The following are 'loggable' ID's.  The logger, logs all msgs with an ID that matches the following. */
/* The unit with the following ID accepts msgs with that ID, plus any high priority msg */
/* These units send msgs with the same ID, but with the CAN_REQ_BIT on */
/* The receiver, receiving a msg with CAN_REQ bit = 0 responds and sends the requested dataid with CAN_REQ = 1 */
/* The receiver, receiving a msg with CAN_REQ bit = 1 disposes of the data in the msg */

/* The following msgs with these 5 bit UNITD will be logged by the logger */
#define CAN_MSG_NUMIDLOGGED	20	// Number of IDs in block that will be logged
#define CAN_MSG_BS		4	// Block Start: First ID for block of message ID
#define CAN_UNITID_SE1	  ((u32)(CAN_MSG_BS + 0)  << CAN_UNITID_SHIFT)	// Sensor unit: Engine 
#define CAN_UNITID_SE2	  ((u32)(CAN_MSG_BS + 1)  << CAN_UNITID_SHIFT)	// Sensor unit: Drive shaft encoder
#define CAN_UNITID_SE3	  ((u32)(CAN_MSG_BS + 2)  << CAN_UNITID_SHIFT)	// Sensor unit: Lower sheave shaft encoder
#define CAN_UNITID_SE4	  ((u32)(CAN_MSG_BS + 3)  << CAN_UNITID_SHIFT)	// Sensor unit: Upper sheave shaft encoder
#define CAN_UNITID_LVL	  ((u32)(CAN_MSG_BS + 4)  << CAN_UNITID_SHIFT)	// Sensor unit: Level wind
#define CAN_UNITID_XB1	  ((u32)(CAN_MSG_BS + 5)  << CAN_UNITID_SHIFT)	// Sensor unit: XBee receiver #1
#define CAN_UNITID_XB2	  ((u32)(CAN_MSG_BS + 6)  << CAN_UNITID_SHIFT)	// Sensor unit: XBee receiver #2
#define CAN_UNITID_DSP	  ((u32)(CAN_MSG_BS + 7)  << CAN_UNITID_SHIFT)	// Display driver/console
#define CAN_UNITID_OLICAW ((u32)(CAN_MSG_BS + 8)  << CAN_UNITID_SHIFT)	// CAW's Olimex board
#define CAN_UNITID_POD6   ((u32)(CAN_MSG_BS + 9)  << CAN_UNITID_SHIFT)	// POD board sensor prototype ("6" marked on board)
#define CAN_UNITID_OLI2   ((u32)(CAN_MSG_BS + 10)  << CAN_UNITID_SHIFT)	// Logger: sensor board w ublox gps & SD card
#define CAN_UNITID_SE5	  ((u32)(CAN_MSG_BS + 11)  << CAN_UNITID_SHIFT)	// Sensor unit: Upper sheave shaft encoder


#define CAN_MSG_BE	(CAN_MSG_BS + CAN_MSG_NUMIDLOGGED - 1)	// Block End:   Last ID for block of message ID

/* See ../svn_sensor/testing/boardtestcan/co1_Olimex/can_log.c */
#define CAN_MSG_TOTALLOGGED	(CAN_MSG_NUMIDLOGGED * CAN_DATAID_NUMBITS)	// This many different UNITID w DATA types loggable

#define CAN_UNITID_GATE	(28  << CAN_UNITID_SHIFT)	//	A 'fake' CAN unit id for intercepting PC sent msgs to the gateway unit
#define CAN_UNITID_GPS	(29  << CAN_UNITID_SHIFT)	//	GPS sentences are logged and identified by a 'fake' CAN unit id

/* The following will not have their msgs logged */
#define CAN_UNITID_CO	  ((u32)(CAN_MSG_BE + 1)  << CAN_UNITID_SHIFT)	// CO
#define CAN_UNITID_POD1	  ((u32)(CAN_MSG_BE + 2)  << CAN_UNITID_SHIFT)	// POD board hack prototype #1
#define CAN_UNITID_CO_OLI ((u32)(CAN_MSG_BE + 3)  << CAN_UNITID_SHIFT)	// Olimex CO
#define CAN_UNITID_GPSDT  ((u32)(CAN_MSG_BE + 4)  << CAN_UNITID_SHIFT)	// GPS Date/Time/tick distribution
#define CAN_UNITID_GATE1  ((u32)(CAN_MSG_BE + 5)  << CAN_UNITID_SHIFT)	// PC<->CAN bus gateway
#define CAN_UNITID_GATE2  ((u32)(CAN_MSG_BE + 6)  << CAN_UNITID_SHIFT)	// PC<->CAN bus gateway
#define CAN_UNITID_LDR    ((u32)(CAN_MSG_BE + 7)  << CAN_UNITID_SHIFT)	// Fixed loader (when no valid eeprom unitid is present)
#define CAN_UNITID_LDR_Oli ((u32)(CAN_MSG_BE + 8)  << CAN_UNITID_SHIFT)	// Fixed loader (when no valid eeprom unitid is present)

/* Data ID's for units */
#define CAN_DATAID_POD6_SHAFT			CAN_TYPE_PDO1tx	// ../svn_sensor/sensor/se1/trunk/tick_pod6.c, /testing/boardtestcan/se1/tick_pod6.c,/sensor/pod6/trunk/tick_pod6.c
#define CAN_DATAID_POD6_SPEED			CAN_TYPE_PDO1rx	// ../svn_sensor/sensor/se1/trunk/tick_pod6.c, /testing/boardtestcan/se1/tick_pod6.c,/sensor/pod6/trunk/tick_pod6.c

#define CAN_DATAID_SE1_THROTTLEandTHERMISTOR	CAN_TYPE_PDO1tx	// ../svn_sensor/sw_f103/trunk/lib/libdevicestm32/adcsensor_eng.c
#define CAN_DATAID_SE1_PRESSUREandRPM		CAN_TYPE_PDO1rx	// ../svn_sensor/sw_f103/trunk/lib/libdevicestm32/adcsensor_eng.c

#define CAN_DATAID_SEN_COUNTERandSPEED		CAN_TYPE_PDO1tx	// ../svn_sensor/sw_f103/trunk/lib/libdevicestm32/adcsensor_foto.c
#define CAN_DATAID_ERROR2			CAN_TYPE_PDO2tx	// ../svn_sensor/sw_f103/trunk/lib/libdevicestm32/adcsensor_foto.c
#define CAN_DATAID_ERROR1			CAN_TYPE_PDO2rx	// ../svn_sensor/sw_f103/trunk/lib/libdevicestm32/adcsensor_foto.c

#define CAN_DATAID_LAT_LONG			CAN_TYPE_SDOtx	// CAN_UNITID_CO_OLI Lattitude and longitude
#define CAN_DATAID_HEIGHT			CAN_TYPE_SDOrx	// CAN_UNITID_CO_OLI Height

/* Data id (hence priority) used with 29b addresses */
#define CAN_FOTO_HISTO				CAN_TYPE_XXX2	//  ../svn_sensor/sw_f103/trunk/lib/libdevicestm32/adcsensor_foto_h.c
/* OR the following with the unit id to give a 32b id for xmission, containing a 29b address (i.e. IDE bit is ON). */
#define CAN_DATAID29_1	( (CAN_TYPE_XXX2 << CAN_DATAID_SHIFT) | (0x1 << 3)  )	// 1st code
#define CAN_DATAID29_2	( (CAN_TYPE_XXX2 << CAN_DATAID_SHIFT) | (0x2 << 3)  )	// 2nd code
#define CAN_DATAID29_3	( (CAN_TYPE_XXX2 << CAN_DATAID_SHIFT) | (0x3 << 3)  )	// 3rd code
#define CAN_DATAID29_4	( (CAN_TYPE_XXX2 << CAN_DATAID_SHIFT) | (0x4 << 3)  )	// 4th code
#define CAN_DATAID29_5	( (CAN_TYPE_XXX2 << CAN_DATAID_SHIFT) | (0x5 << 3)  )	// ...etc...
	
/* Extra data ID's (0 - 7) for units */
// Program loading--
#define CAN_EXTRID_DATA_RD		CAN_TYPE_SDOtx	// 11 Read data (dlc holds byte ct: 1-8)(response: 1-8 bytes starting at current address)
#define CAN_EXTRID_DATA_WR		CAN_TYPE_SDOrx	// 12 Write data dlc holds byte ct: 1-8)(response ACK/NAK command)
#define CAN_EXTRID_DATA_CMD		CAN_TYPE_XXX2	// 13 Command (1st byte of payload holds the command)

#ifdef NO_DERBY_DATABASE_DEFINES
	/* Command codes--byte 1 = code; bytes 2-8 as defined by command  */		
	#define LDR_SET_ADDR	1	// 5 Set address pointer (not FLASH) (bytes 2-5):  Respond with last written address.
	#define LDR_SET_ADDR_FL	2	// 5 Set address pointer (FLASH) (bytes 2-5):  Respond with last written address.
	#define LDR_CRC		3	// 8 Get CRC: 2-4 = count; 5-8 = start address 
	#define LDR_ACK		4	// 1 ACK: Positive acknowledge (Get next something)
	#define LDR_NACK	5	// 1 NACK: Negative acknowledge (So? How do we know it is wrong?)
	#define LDR_JMP		6	// 5 Jump: to address supplied (bytes 2-5)
	#define LDR_WRBLK	7	// 1 Done with block: write block with whatever you have.
	#define LDR_RESET	8	// 1 RESET: Execute a software forced RESET
	#define LDR_XON		9	// 1 Resume sending
	#define LDR_XOFF	10	// 1 Stop sending
	#define LDR_FLASHSIZE	11	// 1 Get flash size; bytes 2-3 = flash block size (short)
	#define LDR_ADDR_OOB	12	// 1 Address is out-of-bounds
	#define LDR_DLC_ERR	13	// 1 Unexpected DLC
	#define LDR_FIXEDADDR	14	// 5 Get address of flash with fixed loader info (e.g. unique CAN ID)
	#define LDR_RD4		15	// 5 Read 4 bytes at address (bytes 2-5)
	#define LDR_APPOFFSET	16	// 5 Get address where application begins storing.
	#define LDR_HIGHFLASHH	17	// 5 Get address of beginning of struct with crc check and CAN ID info for app
	#define LDR_HIGHFLASHP	18	// 8 Get address and size of struct with app calibrations, parameters, etc.
	#define LDR_ASCII_SW	19	// 2 Switch mode to send printf ASCII in CAN msgs
	#define LDR_ASCII_DAT	20	// 3-8  1=line position;2-8=ASCII chars
	#define LDR_WRVAL_PTR	21	// 2-8 Write: 2-8=bytes to be written via address ptr previous set.
		#define	LDR_WRVAL_PTR_SIZE	7	// Write data payload size
	#define LDR_WRVAL_AI	22	// 8 Write: 2=memory area; 3-4=index; 5-8=one 4 byte value
	#define LDR_SQUELCH	23	// 8 Send squelch sending tick ct: 2-8 count
#endif

/* Never-use--16b Mask|ID that will never be used/assigned (for filling an unassigned odd filter bank register) */
#define CAN_NEVERUSEID	(0xffffffff)	// Requires all bits to to match, for msg id 65535.

// This one covers most cases for a 8 byte payload--
#define CAN_DATAID_CT	(0 << CAN_DATAID_MASK)	

/* Unit    (number data types) */
// Units 2,3,4: (2) Shaft counter and computed speed,
// Unit 1:      (3) manifold pressure, rpm, and throttle position
// Unit 5:      (1) level-wind motor current
// Unit 6,7:    (2) Tension Telemetry
// Unit 8:      (4) Four 16 bit display values


/* The following covers all the possible messages in the foregoing.  The unit with the following mask can 
   deal with all messages to/from units */
#define CAN_AGGREGATOR	(CAN_UNITID_MASK | CAN_DATATYPE_MASK)

/* Filter number  (0 - 13) */
#define	CAN_FILTERBANK_RESETALL		0	// Standard: All units have the reset msg  		bank 0 reg 1
#define	CAN_FILTERBANK_TIMESYNC1	1	// Standard: All units have the first time sync 	bank 0 reg 2
#define	CAN_FILTERBANK_TIMESYNC2	2	// Standard: All units have the alternate time sync	bank 1 reg 1
#define	CAN_FILTERBANK_TIMESYNC3	3	// Standard: All units have the another alternate sync	bank 1 reg 2
#define	CAN_FILTERBANK_UNIT		4	// Standard: All units have their ID mask  		bank 2 reg 1



/* ============================================================================================================================= */
#endif
/* ============================================================================================================================= */

/* Buffering incoming CAN messages */
union CANDATA	// Unionize for easier cooperation amongst types
{
	unsigned long long ull;
	signed long long   sll;
	u32 	   ui[2];
	u16 	   us[4];
	u8 	   uc[8];
	u8	   u8[8];
	s32        si[2];
	s16        ss[4];
	s8         sc[8];
};
struct CANRCVBUF		// Combine CAN msg ID and data fields
{ //                               offset  name:     verbose desciption
	u32 id;			// 0x00 CAN_TIxR: mailbox receive register ID p 662
	u32 dlc;		// 0x04 CAN_TDTxR: time & length p 660
	union CANDATA cd;	// 0x08,0x0C CAN_TDLxR,CAN_TDLxR: Data payload (low, high)
};
struct CANRCVTIMBUF		// CAN data plus timer ticks
{
	union LL_L_S	U;	// Linux time, offset, in 1/64th ticks
	struct CANRCVBUF R;	// CAN data
};
struct CANRCVSTAMPEDBUF
{
	union LL_L_S	U;	// Linux time, offset, in 1/64th ticks
	u32 id;			// 0x00 CAN_TIxR: mailbox receive register ID p 662
	u32 dlc;		// 0x04 CAN_TDTxR: time & length p 660
	union CANDATA cd;	// 0x08,0x0C CAN_TDLxR,CAN_TDLxR: Data payload (low, high)	
};

struct PP
{
	char 	*p;
	u32	ct;
};

#define GPSSAVESIZE	80	// Max size of saved GPS line (less than 256)
struct GPSPACKETHDR
{
	union LL_L_S	U;	// Linux time, offset, in 1/64th ticks
	u32 		id;	// Fake msg ID for non-CAN messages, such as GPS
	u8 c[GPSSAVESIZE];	// 1st byte is count; remaining bytes are data
};

union CANPC
{
	struct CANRCVBUF	can;		// Binary msg
	u8 c[sizeof(struct CANRCVBUF)+2];	// Allow for chksum w longest msg
};

struct CANPCWRAP	// Used with gateway (obsolete except for old code)
{
	union CANPC can;
	u32	chk;
	u8	*p;
	u8	prev;
	u8	c1;
	s16 	ct;
};

/* The following are used in the PC program and USART1_PC_gateway.  (Easier to find them here.) */

#define PC_TO_GATEWAY_ID	(' ')	// Msg to/from PC is for the gateway unit
#define PC_TO_CAN_ID		0x0	// Msg to/from PC is for the CAN bus

#define CAN_PC_FRAMEBOUNDARY	'\n'	// Should be a value not common in the data
#define CAN_PC_ESCAPE		0X7D	// Should be a value not common in the data

#define CHECKSUM_INITIAL	0xa5a5	// Initial value for computing checksum

#define ASCIIMSGTERMINATOR	'\n'	// Separator for ASCII/HEX msgs

#define PCTOGATEWAYSIZE	48	// (Note keep align 4 to allow casts to ints)
/* Compressed msg  */
struct PCTOGATECOMPRESSED
{
	u8 	cm[PCTOGATEWAYSIZE/2];	// seq + id + dlc + payload bytes + jic spares
	u8*	p;			// Pointer into cm[]
	s16	ct;			// Byte count of compressed result (not including checksum)
	u8	seq;			// Message count
	u8	chk;			// Checksum
};


struct PCTOGATEWAY	// Used in PC<->gateway asc-binary conversion & checking
{
	char asc[PCTOGATEWAYSIZE];	// ASCII "line" is built here
//	u8	*p;			// Ptr into buffer
	char	*pasc;			// Ptr into buffer
	u32	chk;			// Checksum
	s16	ct;			// Byte ct (0 = msg not complete; + = byte ct)
	s16	ctasc;			// ASC char ct
	u8	prev;			// Used for binary byte stuffing
	u8	seq;			// Sequence number (CAN msg counter)
	u8	mode_link;		// PC<->gateway mode (binary, ascii, ...)
	struct PCTOGATECOMPRESSED cmprs; // Easy way to make call to 'send'
};

/* ------------ PC<->gateway link MODE selection -------------------------------------- */
#define MODE_LINK	2	// PC<->gateway mode: 0 = binary, 1 = ascii, 2 = ascii-gonzaga


#ifndef STRUCT_SCLOFF
#define STRUCT_SCLOFF
struct SCLOFF
{
	float	offset;	
	float	scale;
};
#endif

 // Thermistor parameters for converting ADC readings to temperature
 #ifndef STRUCT_THERMPARAM
 #define STRUCT_THERMPARAM
 struct THERMPARAM
 {   //                   	   default values    description
	float B;		//	3380.0	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	float RS;		//	10.0	// Series resistor, fixed (K ohms)
	float R0;		//	10.0	// Thermistor room temp resistance (K ohms)
	float TREF;		//	298.0	// Reference temp for thermistor
	struct SCLOFF	os;	//      0.0,1.0	// Therm temp correction offset	1.0 Therm correction scale
 };
 #endif

#endif
#endif 

