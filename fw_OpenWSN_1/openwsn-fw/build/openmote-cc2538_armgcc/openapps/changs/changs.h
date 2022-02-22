#ifndef __CHANGS_H
#define __CHANGS_H

/**
\addtogroup AppUdp
\{
\addtogroup changs
\{
*/

#include "opentimers.h"
#include "openudp.h"

//=========================== define ==========================================

#define CHANGS_PERIOD_MS 5000 // 5초마다

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
    opentimers_id_t     timerId;   ///< periodic timer which triggers transmission
    uint16_t             counter;  ///< incrementing counter which is written into the packet
    uint16_t              period;  ///< changs packet sending period>
    udp_resource_desc_t     desc;  ///< resource descriptor for this module, used to register at UDP stack
    bool      busySendingChangs;  ///< TRUE when busy sending an changs
} changs_vars_t;

//=========================== prototypes ======================================

void changs_init(void);
void changs_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void changs_receive(OpenQueueEntry_t* msg);
/**
\}
\}
*/

#endif

