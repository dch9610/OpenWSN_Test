/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:45:12.106000.
*/
#ifndef __CSTORM_H
#define __CSTORM_H

/**
\addtogroup AppUdp
\{
\addtogroup cstorm
\{
*/

#include "Python.h"

#include "opencoap_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   opentimers_id_t     timerId;
   uint16_t             period;   ///< inter-packet period (in ms)
   bool      busySendingCstorm;   ///< TRUE when busy sending cstorm packet
} cstorm_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void cstorm_init(OpenMote* self);

/**
\}
\}
*/

#endif

