/******************************************************************************
* File Name          : tension_b_param.c
* Date First Issued  : 04/08/2015
* Board              : f103
* Description        : Initialize tension function struct with default values &
*                    : define table of index/id versus pointer to struct
*******************************************************************************/
/*
If the parameter loading from the PC is not implemented, then this routine can be
used to "manually" set parameters.
*/

#include "db/gen_db.h"
#include "tension_function.h"
#include "tension_idx_v_struct.h"

__attribute__ ((section(".appparam2"))) // Place in flash location
#include "idx_v_val.c"

