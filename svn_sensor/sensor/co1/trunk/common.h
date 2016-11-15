/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : common_se.h
* Hackerees          : deh
* Date First Issued  : 12/06/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : includes that are common to project
*******************************************************************************/

#ifndef __COMMON_SENSOR
#define __COMMON_SENSOR

/*
ID/arbitration word (p 659 (tx), 662 (rx))
Standard ID: 11 bits
Extended ID: 29 bits

ID Bits [31:27]
Unit -- 5 bits associated with the unit
0   Time sync
1   Sensor 1 (upper sheave)
2   Sensor 2 (lower sheave)
3   Sensor 3 (drive shaft)
4   Sensor 4 (engine)
5   Sensor 5 (level wind)
6   Sensor 6 (Xbee 1)
7   Sensor 7 (Xbee 2)
...
31  

ID Bits [26:21]	6 bits associated with the data (8 bytes)
Data type identification -- 

For the extended ID, the additional 18 bits are used for 
the program load address--
ID Bits [20:3]
Program load addresses, or sequence number for other multi-msg data payloads


*/
/* Interrupt priorities - CAN */
#define NVIC_CAN_RX1_IRQ_PRIORITY		0x10	// Receive FIFO 1 (and related) [time sync]
#define NVIC_CAN_SCE_IRQ_PRIORITY		0x50	// Errors & status change
#define NVIC_USB_HP_CAN_TX_IRQ_PRIORITY		0x50	// Transmit FIFO
#define NVIC_USB_LP_CAN_RX0_IRQ_PRIORITY	0x40	// Receive FIFO 0 (and related)
#define NVIC_TIM7_IRQ_PRIORITY			0x10	// CAN Unitid msg

#define CANBAUDRATE	500000	// Name says it all

/* Number of bits in each field of ID (11 bits) */
#define CAN_UNITID_NUMBITS	5	// Allow 31 unit id's
#define CAN_DATAID_NUMBITS	6	// Allow 64 data types

/* Use extended ID for sending program loading data.  The extended ID carries 29 bits.  The first 
   11 bits are identical in position and use as the with the standard ID.  
   The extension adds bits that are used to specify the 8 byte boundary address for the 8 bytes 
   of program load data.  It can also be used for future sending/receiving data where multiple 
   messages of 8 bytes are required. */
#define CAN_ADDR_NUMBITS	(29-CAN_UNITID_NUMBITS-CAN_DATAID_NUMBITS) // Remaining bits

/* Number of bits to shift field */
#define CAN_UNITID_SHIFT	(32 - CAN_UNITID_NUMBITS)
#define CAN_DATAID_SHIFT	(32 - CAN_UNITID_NUMBITS - CAN_DATAID_NUMBITS)

/* Masks for the ID fields */
#define CAN_UNITID_MASK  ( ((1 << CAN_UNITID_NUMBITS) - 1) << CAN_UNITID_SHIFT  )
#define CAN_DATAID_MASK  ( ((1 << CAN_DATAID_NUMBITS) - 1) << CAN_DATAID_SHIFT  )
#define CAN_ADDR_MASK	 ( ((1 << CAN_ADDR_NUMBITS) - 1) << 3)	// IDE, RTR, Res bits are the lowest order bits 

/* The time sync msg that everybody uses.  Lower numbers have higher arbitration priority. */
#define CAN_RESETALL	((0 << CAN_UNITID_SHIFT) | CAN_RIxR_RTR) 	// Put ALL units to reset and go into startup/program loader
#define CAN_TIMESYNC1	((1 << CAN_UNITID_SHIFT) | CAN_RIxR_RTR) 	// GPS time sync distribution msg (CAN setup for no error retry - NART in CAN_MCR, p 649)
#define CAN_TIMESYNC2	((2 << CAN_UNITID_SHIFT) | CAN_RIxR_RTR) 	// Reserve for multiple time sync's or high priority msg
#define CAN_TIMESYNC3	((3 << CAN_UNITID_SHIFT) | CAN_RIxR_RTR) 	// Reserve for multiple time sync's or high priority msg
#define CAN_TIMESYNC4	((4 << CAN_UNITID_SHIFT) | CAN_RIxR_RTR) 	// Reserve for multiple time sync's or high priority msg


/* Sensor message block of ID start */
#define CAN_MSG_BS	8		// First ID for block of message ID
#define CAN_MSG_BLOCKSIZE	32	// Number of message IDs allowed

/* Except for the CO the units only accept msgs with the following ID 'and' the CAN_REQ_BIT off */
/* These units send msgs with the same ID, but with the CAN_REQ_BIT on */
/* The receiver, receiving a msg with CAN_REQ bit = 0 responds and sends the requested dataid with CAN_REQ = 1 */
/* The receiver, receiving a msg with CAN_REQ bit = 1 disposes of the data in the msg */
#define CAN_UNITID_SE1	 (CAN_MSG_BS + 0)  << CAN_UNITID_SHIFT)	// Sensor unit: Upper sheave shaft encoder
#define CAN_UNITID_SE2	 (CAN_MSG_BS + 1)  << CAN_UNITID_SHIFT)	// Sensor unit: Lower sheave shaft encoder
#define CAN_UNITID_SE3	 (CAN_MSG_BS + 2)  << CAN_UNITID_SHIFT)	// Sensor unit: Drive shaft encoder
#define CAN_UNITID_ENG	 (CAN_MSG_BS + 3)  << CAN_UNITID_SHIFT)	// Sensor unit: Engine 
#define CAN_UNITID_LVL	 (CAN_MSG_BS + 4)  << CAN_UNITID_SHIFT)	// Sensor unit: Level wind
#define CAN_UNITID_XB1	 (CAN_MSG_BS + 5)  << CAN_UNITID_SHIFT)	// Sensor unit: XBee receiver #1
#define CAN_UNITID_XB2	 (CAN_MSG_BS + 6)  << CAN_UNITID_SHIFT)	// Sensor unit: XBee receiver #2
#define CAN_UNITID_DSP	 (CAN_MSG_BS + 7)  << CAN_UNITID_SHIFT)	// Display driver
#define CAN_UNITID_CO	 (CAN_MSG_BS + 8)  << CAN_UNITID_SHIFT)	// CO
#define CAN_UNITID_POD1	 (CAN_MSG_BS + 9)  << CAN_UNITID_SHIFT)	// POD board hack prototype #1
#define CAN_UNITID_PODCO (CAN_MSG_BS + 10) << CAN_UNITID_SHIFT)	// POD board hack for CO prototype

/* Data id, for data within UNITID */
#define CAN_DATAID_CK	 (1 << CAN_DATAID_MASK)	// Program checksum
#define CAN_DATAID_PL	 (2 << CAN_DATAID_MASK)	// Program loading
#define CAN_DATAID_GO	 (3 << CAN_DATAID_MASK)	// "Go ahead: command ends sucessful checksum checking
#define CAN_DATAID_RESET (4 << CAN_DATAID_MASK)	// Software forced RESET
#define CAN_DATAID_ERASE (5 << CAN_DATAID_MASK)	// Command to erase block

// This one covers most cases for a 8 byte payload--
#define CAN_DATAID_CT	(0 << CAN_DATAID_MASK)	
// Units 1,2,3: Shaft counter and computed speed,
// Unit 4:   manifold pressure, and rpm
// Unit 5:   level wind current
// Unit 6,7: Tension Telemetry
// Unit 8:   Four 16 bit display values


/* The following covers all the possible messages in the foregoing.  The unit with the following mask can 
   deal with all messages to/from units */
#define CAN_AGGREGATOR	(CAN_UNITID_MASK | CAN_DATAID_MASK)

/*
Wonderful things we CAN do--

Note: What is the difference between a data frame with zero data bytes and a RTR frame? 
Ans: There are two differences between a Data Frame and a Remote Frame. Firstly the RTR-bit is
transmitted as a dominant bit in the Data Frame and secondly in the Remote Frame there is no Data Field.
i.e., RTR = 0 DOMINANT in data frame, RTR = 1 RECESSIVE in remote frame 

1) Time sync--
   ID is the highest priority.
   All units accept the time sync message.  
   RTR - 1 (i.e. no data payload)
   IDE - 0 (standard 11b ID, i.e. a shortest possible message (44 CAN bit times)) 

2) Data polling--
   The CO (unit initiating the polling) sends a UNITID, with
   DATAID.
   RTR - 1 (i.e. no data payload)
   IDE - 0 (standard ID)

   The accepting unit responds by sending the same UNITID, with
   DATAID.
   RTR - 0 
   IDE - 0
   Data - 8 bytes (which will cover most cases)

3) Program checksum checking--
   The CO, or laptop via the CO, i.e. the unit initiating and checking the checksum,
   sends a UNITID, with DATAID_CK (checksum request).
   RTR - 0 (data payload)
   IDE - 0 (standard ID)
   Data - checksum start & end addresses (two 32b words)

   The accepting unit responds by sending the same UNITID, with
   DATAID and the computed checksum.
   RTR - 0
   IDE - 0 (standard ID)
   Data - checksum  (4 bytes)

3.1) Go-ahead command that ends program checksum checking/program loading

   The CO, or laptop via the CO, i.e. the unit initiating and checking the checksum,
   sends a UNITID, with DATAID_GO
   RTR - 1 (no data payload)
   IDE - 0 (standard ID)

   If in checksum/prog-load loop the accepting unit jumps to the program, otherwise the
   msg is ignored.  No response expected.
   
4) Program loading--

   The CO, or laptop via the CO, i.e. the unit initiating and the program loading,
   sends a UNITID, with DATAID.
   RTR - 0 (data payload)
   IDE - 1 (extended ID with 8 byte boundary address relative to start of flash)
   Data - 8 byte

   The accepting unit writes to flash and responds by sending the same UNITID, with
   DATAID.
   RTR - 1 (no data)
   IDE - 0 (standard ID)

   The CO continues until the program has been loaded.

5) Erase flash

   The CO, or laptop via the CO, i.e. the unit initiating and the program loading,
   sends a UNITID, with DATAID.
   RTR - 0 (no data payload)
   IDE - 1 (extended ID with block address for erasing flash)
   Data - 8 byte

   The accepting unit erases flash block and responds by sending the same UNITID, with
   DATAID.
   RTR - 1 (no data)
   IDE - 0 (standard ID)
 
*/


/* Filter bank assignments (0 - 13) */
#define	CAN_FILTERBANK_TIMESYNC	0	// Standard: All units have the time sync (list mode) in bank 0
#define	CAN_FILTERBANK_UNIT	1	// Standard: All units have their ID mask in bank 1


/* Buffering incoming CAN messages */
union CANDATA	// Unionize for easier cooperation amongst types
{
	unsigned long long ull;
	unsigned int ui[2];
	unsigned short us[4];
	unsigned char uc[8];
};


struct CANRCVBUF
{
	unsigned int id;	// CAN mailbox receive register p 662
	union CANDATA cd;	// Data payload
};


#endif 

