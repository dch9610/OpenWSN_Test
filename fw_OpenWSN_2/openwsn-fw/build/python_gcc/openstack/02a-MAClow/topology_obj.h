/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:45:29.989000.
*/
#ifndef __TOPOLOGY_H
#define __TOPOLOGY_H

/**
\addtogroup MAClow
\{
\addtogroup topology
\{
*/

#include "Python.h"

#include "opendefs_obj.h"
#include "IEEE802154_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

bool topology_isAcceptablePacket(OpenMote* self, ieee802154_header_iht* ieee802514_header);

/**
\}
\}
*/

#endif
