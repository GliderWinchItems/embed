/******************************************************************************
* File Name          : can_log.h
* Date First Issued  : 08/09/2016
* Board              : Sensor
* Description        : Logging function
*******************************************************************************/
/* 

*/

#ifndef __CAN_LOG
#define __CAN_LOG

#include "common.h"	// Such as 'u32'
#include "common_can.h"	// CAN ID|mask layout and structs
#include "logger_idx_v_struct.h"


struct LOGGERFUNCTION
{
	struct	LOGGERLC logger_s;
	struct CANHUB* phub_logger;	// Pointer: CAN hub buffer
	void* pparamflash;		// Pointer to flash area with flat array of parameters
	uint32_t* pcanid_cmd_logger;	// Pointer into high flash for command can id
	uint32_t hb_t;			// tim9 tick counter for next heart-beat CAN msg
	uint32_t hbct_ticks;		// logger.hbct (ms) converted to timer ticks
	uint8_t status_byte;		// Reading status byte	
};

/******************************************************************************/
int can_log_init(void);
/* @brief 	: A bit of initialization
 * @return	: 0 = success
 *		: -997 can-hub addition failed
 *		: -998 did not find function code in table
 *		: -999 unreasonable table size
*******************************************************************************/
int can_log_poll (struct CANRCVBUF* pcan, struct LOGGERFUNCTION* p);
/* @brief 	: Buffer msgs for SD logging and trigger lower level SD write
 * @param	: pcan = pointer to struct with CAN msgcan_hub_add_func
 * @param	: p = pointer to logger function struct with params and more
 * @return	: 0 = No outgoing msgs sent; 1 = one or more msgs were sent
*******************************************************************************/
struct CANRCVSTAMPEDBUF* monitor_m_canbuf_get(void);
/* @brief 	: See if buffer with CAN msg is ready for sending to PC
 * @return	: pointer = 0 for data not ready, otherwise points buffer
*******************************************************************************/
u32 monitor_n_canbuf_get(void);
/* @brief 	: See if buffer with CAN id is ready for sending to PC
 * @return	: 0 = none, otherwise holds ID (ID == 0 not assigned)
*******************************************************************************/
void can_log_trigger_logging(void);
/* @brief 	: Set the low level interrupt to execute sd card write
*******************************************************************************/
 void noncanlog_add(u32 id, char* ppay, int count);
/* @brief 	: Add a non-CAN item to be buffered for logging only (call from mainline only)
 * @param	: p = pointer to string with gps sentence (starting with '$')
 * @param	: id = CAN id
 * @param	: ppay = payload pointer
 * @param	: count = number of bytes in payload
*******************************************************************************/

/*####################################################################################### */
void can_log_puttime(struct CANRCVBUF *p, unsigned long long ull);	
/* brief Set up CAN time sync msg for logging. Buffer once per sec msg with time
 * Enter here under a high priority interrupt from Tim4_pod_se.c
 * param	: p = pointer to time sync msg (64/sec)
 * param	: ull = system linux time (<<6)
 *####################################################################################### */

/* Running count of CAN msgs written to SD card */
extern u32 canlogct;

/* Pointer to functions to be executed under a low priority interrupt */
extern void 	(*logpriority_ptr)(void);	// Address of function to call forced interrupt

/* Unrecoverable SD write error */
extern int sdlog_error;

/* Holds parameters and associated computed values and readings. */
extern struct LOGGERFUNCTION logger_f;

#endif 



