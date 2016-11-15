/******************************************************************************
* File Name          : can_hub.c
* Date First Issued  : 04/19/2015
* Board              : stm32
* Description        : hub-server for CAN functions
*******************************************************************************/
/*
This routine passes incoming CAN msgs from the CAN hardware, and each function added,
and distributes them to each function and and the CAN hardware, except for the 
connection/port that the msg came in on.


Useage:

// struct CANRCVBUF* pcan;

// Initialization (for each function)
struct CANHUB* myport1 = can_hub_add_funct();
...

// Polling loop starts here  (see "CAN_poll_loop.c")
do
{
  can_hub_begin(); // Load CAN hardware msgs into tension queue
  // Get incoming msgs
  pcan = can_hub_get(myport1); // Get ptr to CAN msg
  if (pcan != 0) // Check for no msgs available
  { // Here, we have a msg (from CAN hardware, or other functions)
     tension_function_poll(pcan); // Do something with the msg
     //   and if it involves sending a msg, e.g. pmsg points to it,
     //   then in the 'tension_function_poll(pcan)' function there would
     //   be something like the following to send the msg--
	 can_hub_send(pmsg, myport1); // Send msg
  }
  
  // Repeat above code sequence for each function.

} while (can_hub_end() != 0); // Repeat if 

return;
*/

#include <malloc.h>
#include "can_hub.h"
#include "libusartstm32/nvicdirect.h"
#include "can_driver.h"

extern struct CAN_CTLBLOCK* pctl0;	// Pointer to CAN1 control block (set during init)

static uint32_t something_added = 0;	// 0 = no msgs added to hub buffers

static struct CANHUB* portptr = NULL;	// Pointer to latest link struct

/******************************************************************************
 * struct CANHUB* can_hub_add_func(void);
 * @brief	: Add a function port 
 * @return	: Return pointer to the port buffer/struct; zero/NULL = failed.
 ******************************************************************************/
struct CANHUB* can_hub_add_func(void)
{
	struct CANHUB* p;

	p = (struct CANHUB*)malloc( sizeof (struct CANHUB) );
	if (p == 0) return 0;	// Returned: failed.

	/* Set buffer pointers */
	p->pPut = &p->can[0];		// Pointer for adding msgs to circular queue
	p->pGet = &p->can[0];		// Pointer for removing msgs from queue
	p->pEnd = &p->can[CAN_HUB_BUFFSIZE];	// End pointer remains constant

	/* Set linking of list */
	/* Add to list (the 1st one added has ptr->next = 0) */ 
	p->pnext = portptr;
	portptr = p;
	return p;
}
/******************************************************************************
 * static void can_hub_add(CANRCVBUF* pcan, struct CANHUB* pptr);
 * @brief	: Add a msg to a can hub port buffer
 * @param	: pcan = pointer to CAN msg
 * @param	: pptr = pointer to the struct holding the buffer
 ******************************************************************************/
static void can_hub_add(struct CANRCVBUF* pcan, struct CANHUB* pptr)
{
	*pptr->pPut = *pcan;		// Copy CAN msg into buffer
	pptr->pPut++;
	if (pptr->pPut >= pptr->pEnd) pptr->pPut = &pptr->can[0]; // Wrap check
	return;
}
/******************************************************************************
 * static void can_hub_addall_but(CANRCVBUF* pcan, struct CANHUB* pptr);
 * @brief	: Add CAN msg to all local buffers except the one with 'pptr'
 * @param	: pcan = pointer to CAN msg
 * @param	: pptr = pointer to the "but" in  "all but"
 ******************************************************************************/
static void can_hub_addall_but(struct CANRCVBUF* pcan, struct CANHUB* pptr)
{
	struct CANHUB* p = portptr;	// Point to last in the list.
	
	while (p != NULL)	// Step through the list
	{
		if (p != pptr)
		{ // Here, the port is not "us"
			can_hub_add(pcan, p);	// Add CAN msg to port buffer
		}
		p = p->pnext;
	}
	return;
}
/******************************************************************************
 * void can_hub_begin(void);
 * @brief	: Add any CAN msgs coming in from CAN hardware to local buffers
 ******************************************************************************/
struct CANRCVBUF test0;
struct CANRCVBUF test1;

void can_hub_begin(void)
{
	struct CANRCVBUF* pcan0;	// Lower priority (most common)
	struct CANRCVBUF* pcan1;	// High priority

	/* Low priority CAN msgs. */
	while ((pcan0=can_driver_peek0(pctl0)) != 0)
	{ // Here, we have a msg
		can_hub_addall_but(pcan0, 0);	// Add to all local buffers
test0 = *pcan0;
		can_driver_toss0(pctl0);	// Release original msg
	}

	/* High priority CAN msgs. */
	while((pcan1=can_driver_peek1(pctl0)) != 0)
	{ // Here, we have a msg
		can_hub_addall_but(pcan1, 0);	// Add to all local buffers
test1 = *pcan1;
		can_driver_toss1(pctl0);	// Release original msg
	}

	/* The above additions get handled by all the following functions.
	    but the following functions may add some msgs and another pass 
	    may be needed. */
	something_added = 0;		// Show no local msgs have been added.

	return;
}
/******************************************************************************
 * void can_hub_send(struct CANRCVBUF* pcan, struct CANHUB* pptr);
 * @brief	: Send CAN msg into hub
 * @param	: pcan = pointer to CAN msg to be sent
 * @param	: pprt = hub port buffer struct pointer
 ******************************************************************************/
void can_hub_send(struct CANRCVBUF* pcan, struct CANHUB* pptr)
{
	can_hub_addall_but(pcan, pptr);	// Add to other functions except "us"
	can_driver_put(pctl0,pcan,4,0);	// Add/send to CAN driver
	return;
}
/******************************************************************************
 * struct CANRCVBUF* can_hub_get( struct CANHUB* pptr);
 * @brief	: Send a CAN msg
 * @param	: pprt = hub port buffer struct pointer
 * @return	: pointer to CAN msg struct; 0 = no msgs in buffer
 ******************************************************************************/
struct CANRCVBUF* can_hub_get( struct CANHUB* pptr)
{
	struct CANRCVBUF* p;

	if  (pptr->pPut == pptr->pGet) return 0;	// Return: no msgs
	p = (pptr->pGet)++;				// Save pointer to msg
	if  (pptr->pGet >= pptr->pEnd) pptr->pGet = &pptr->can[0]; // Wrap check
	return p;
}
/******************************************************************************
 * uint32_t can_hub_end(void);
 * @brief	: return if any msgs added
 * @return	: 0 = none added; 1 = might need another pass
 ******************************************************************************/
uint32_t can_hub_end(void)
{
	return something_added;
}

