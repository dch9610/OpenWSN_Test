/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-20 09:54:34.948000.
*/
#ifndef __CJOIN_H
#define __CJOIN_H

/**
\addtogroup AppUdp
\{
\addtogroup cjoin
\{
*/
#include "Python.h"

#include "opendefs_obj.h"
#include "opencoap_obj.h"
#include "openoscoap_obj.h"
//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
    coap_resource_desc_t     desc;
    opentimers_id_t          timerId;
    bool                     isJoined;
    oscoap_security_context_t context;
    uint8_t medType;
} cjoin_vars_t;

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void cjoin_init(OpenMote* self);
void cjoin_schedule(OpenMote* self);
bool cjoin_getIsJoined(OpenMote* self);
void cjoin_setIsJoined(OpenMote* self, bool newValue);
void cjoin_setJoinKey(uint8_t *key, uint8_t len);

/**
\}
\}
*/

#endif
