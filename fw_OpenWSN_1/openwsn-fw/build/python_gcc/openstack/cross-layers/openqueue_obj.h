/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:45:21.243000.
*/
#ifndef __OPENQUEUE_H
#define __OPENQUEUE_H

/**
\addtogroup cross-layers
\{
\addtogroup OpenQueue
\{
*/

#include "Python.h"

#include "opendefs_obj.h"
#include "IEEE802154_obj.h"

//=========================== define ==========================================

#define QUEUELENGTH  10

//=========================== typedef =========================================

typedef struct {
   uint8_t  creator;
   uint8_t  owner;
} debugOpenQueueEntry_t;

//=========================== module variables ================================

typedef struct {
   OpenQueueEntry_t queue[QUEUELENGTH];
} openqueue_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

// admin
void openqueue_init(OpenMote* self);
bool debugPrint_queue(OpenMote* self);
// called by any component
OpenQueueEntry_t* openqueue_getFreePacketBuffer(OpenMote* self, uint8_t creator);
owerror_t openqueue_freePacketBuffer(OpenMote* self, OpenQueueEntry_t* pkt);
void openqueue_removeAllCreatedBy(OpenMote* self, uint8_t creator);
bool openqueue_isHighPriorityEntryEnough(OpenMote* self);
// called by ICMPv6
void openqueue_updateNextHopPayload(OpenMote* self, open_addr_t* newNextHop);
// called by res
OpenQueueEntry_t* openqueue_sixtopGetSentPacket(OpenMote* self);
OpenQueueEntry_t* openqueue_sixtopGetReceivedPacket(OpenMote* self);
uint8_t openqueue_getNum6PResp(OpenMote* self);
uint8_t openqueue_getNum6PReq(OpenMote* self, open_addr_t* neighbor);
void openqueue_remove6PrequestToNeighbor(OpenMote* self, open_addr_t* neighbor);
// called by IEEE80215E
OpenQueueEntry_t* openqueue_macGet6PResponseAndDownStreamPacket(OpenMote* self, open_addr_t* toNeighbor);
OpenQueueEntry_t* openqueue_macGet6PRequestOnAnycast(OpenMote* self, open_addr_t* autonomousUnicastNeighbor);
OpenQueueEntry_t* openqueue_macGetEBPacket(OpenMote* self);
OpenQueueEntry_t* openqueue_macGetKaPacket(OpenMote* self, open_addr_t* toNeighbor);
OpenQueueEntry_t* openqueue_macGetDIOPacket(OpenMote* self);
OpenQueueEntry_t* openqueue_macGetNonJoinIPv6Packet(OpenMote* self, open_addr_t* toNeighbor);
OpenQueueEntry_t* openqueue_macGet6PandJoinPacket(OpenMote* self, open_addr_t* toNeighbor);
/**
\}
\}
*/

#endif
