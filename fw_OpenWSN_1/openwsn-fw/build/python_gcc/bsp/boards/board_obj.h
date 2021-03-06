/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:45:07.128000.
*/
#ifndef __BOARD_H
#define __BOARD_H

/**
\addtogroup BSP
\{
\addtogroup board
\{

\brief Cross-platform declaration "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "Python.h"

#include "board_info.h"
#include "toolchain_defs.h"

//=========================== define ==========================================

typedef enum {
   DO_NOT_KICK_SCHEDULER = 0,
   KICK_SCHEDULER        = 1,
} kick_scheduler_t;

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void board_init(OpenMote* self);
void board_sleep(OpenMote* self);
void board_reset(OpenMote* self);

/**
\}
\}
*/

#endif
