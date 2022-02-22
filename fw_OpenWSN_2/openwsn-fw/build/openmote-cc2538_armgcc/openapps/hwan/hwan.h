#ifndef __HWAN_H
#define __HWAN_H

/**
\addtogroup AppUdp
\{
\addtogroup hwan
\{
*/

#include "opentimers.h"
#include "openudp.h"

//=========================== define ==========================================

#define hwan_PERIOD_MS 5000 // 5초마다

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
    opentimers_id_t     timerId;   ///< periodic timer which triggers transmission
    uint16_t             counter;  ///< incrementing counter which is written into the packet
    uint16_t              period;  ///< hwan packet sending period>
    udp_resource_desc_t     desc;  ///< resource descriptor for this module, used to register at UDP stack
    bool      busySendinghwan;  ///< TRUE when busy sending an hwan
} hwan_vars_t;

//=========================== prototypes ======================================

void hwan_init(void);
void hwan_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void hwan_receive(OpenQueueEntry_t* msg);
/**
\}
\}
*/

#endif

