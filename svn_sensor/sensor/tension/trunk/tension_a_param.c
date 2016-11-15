/******************************************************************************
* File Name          : tension_a_param.c
* Date First Issued  : 04/08/2015, 05/12/2016
* Board              : f103
* Description        : Manually load parmeter table into high flash
*******************************************************************************/
/*
If the parameter loading from the PC is not implemented, then this routine can be
used to "manually" set parameters.
*/

#include "db/gen_db.h"
#include "tension_function.h"
#include "tension_idx_v_struct.h"

__attribute__ ((section(".appparam1"))) // Place in flash location
#include "idx_v_val.c"


