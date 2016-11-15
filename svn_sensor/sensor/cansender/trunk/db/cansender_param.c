/******************************************************************************
* File Name          : cansender_param.c
* Date First Issued  : 09/10/2016
* Board              : f103
* Description        : Manually load parmeter table into high flash 
*******************************************************************************/
/*
If the parameter loading from the PC is not implemented, then this routine can be
used to "manually" set parameters.
*/

#include "db/gen_db.h"
#include "../cansender_idx_v_struct.h"

/* 
   Select the parameter array for this specific function isntance 
   from the file generated by the java prorgram & database.  Compile
   and load it as if it were a duly certified and authorized program.

   The .ld file will handle the specification of where this is loaded.

   The command line for compiling needs to have the #define for selecting
   the table from the file 'idx_v_val.c' (which has all the value tables
   for all functions).  This can be accomplished by the following--

make TSELECT=-DTENSION_1a

   which makes it possible to feed the selection into the compile command
   *script*, e.g. ./mm CANSEND_1

*/


__attribute__ ((section(".appparam1"))) // Place in FLASHP1 location

#include "db/idx_v_val.c"	// The source code file with all the functions


