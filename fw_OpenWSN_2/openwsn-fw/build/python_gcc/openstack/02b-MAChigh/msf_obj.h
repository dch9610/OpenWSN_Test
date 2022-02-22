/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:45:18.300000.
*/
#ifndef __MSF_H
#define __MSF_H

/**
\addtogroup MAChigh
\{
\addtogroup msf
\{
*/

#include "Python.h"

#include "opendefs_obj.h"
#include "opentimers_obj.h"
//=========================== define ==========================================

#define IANA_6TISCH_SFID_MSF    0
#define CELLOPTIONS_MSF         CELLOPTIONS_TX
#define NUMCELLS_MSF            1

#define MAX_NUMCELLS                   64
#define LIM_NUMCELLSUSED_HIGH          48
#define LIM_NUMCELLSUSED_LOW           16

#define HOUSEKEEPING_PERIOD          30000 // miliseconds
#define QUARANTINE_DURATION            300 // seconds
#define WAITDURATION_MIN             30000 // miliseconds
#define WAITDURATION_RANDOM_RANGE    30000 // miliseconds

//=========================== typedef =========================================

typedef struct {
    bool                 f_hashCollision;
    uint8_t              numAppPacketsPerSlotFrame;
    uint8_t              backoff;
    uint8_t              numCellsPassed;
    uint8_t              numCellsUsed;
    opentimers_id_t      housekeepingTimerId;
    uint16_t             housekeepingPeriod;
    opentimers_id_t      waitretryTimerId;
    bool                 waitretry;
} msf_vars_t;

//=========================== module variables ================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

// admin
void msf_init(OpenMote* self);
void msf_appPktPeriod(OpenMote* self, uint8_t numAppPacketsPerSlotFrame);
uint8_t msf_getsfid(OpenMote* self);
bool msf_candidateAddCellList(OpenMote* self, 
    cellInfo_ht* cellList,
    uint8_t requiredCells
);
bool msf_candidateRemoveCellList(OpenMote* self, 
    cellInfo_ht* cellList,
    open_addr_t* neighbor,
    uint8_t requiredCells
);
// called by schedule
void msf_updateCellsPassed(OpenMote* self, open_addr_t* neighbor);
void msf_updateCellsUsed(OpenMote* self, open_addr_t* neighbor);
// called by icmpv6rpl, where parent changed
void msf_trigger6pClear(OpenMote* self, open_addr_t* neighbor);

uint16_t msf_hashFunction_getSlotoffset(OpenMote* self, uint16_t moteId);
uint8_t msf_hashFunction_getChanneloffset(OpenMote* self, uint16_t moteId);

void msf_setHashCollisionFlag(OpenMote* self, bool isCollision);
bool msf_getHashCollisionFlag(OpenMote* self);
/**
\}
\}
*/

#endif
