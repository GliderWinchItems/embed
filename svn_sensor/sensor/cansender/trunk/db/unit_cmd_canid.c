/******************************************************************************
* File Name          : unit_cmd_canid.c
* Date First Issued  : 05/12/2016
* Board              : f103
* Description        : Load command id table into high flash for a CAN unit
*******************************************************************************/
/*
If the parameter loading from the PC is not implemented, then this routine can be
used to "manually" set parameters.
*/

#include "db/gen_db.h"
#include "../cansender_idx_v_struct.h"
#include "../../../../../svn_common/trunk/common_highflash.h"

/* 
   The .ld file will handle the specification of where this is loaded.


make TSELECT=-DCAN_UNIT_12

   which makes it possible to feed the selection into the compile command
   *script*, e.g. ./mm TENSION_1a TENSION_1b CAN_UNIT_12

*/

__attribute__ ((section(".appparam0a"))) // Place in FLASHP0a location

#include "db/idx_v_val.c"	// The source code file with all the functions


