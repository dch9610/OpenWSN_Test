#ifndef __CHANG_H
#define __CHANG_H

/**
\addtogroup AppUdp
\{
\addtogroup chang
\{
*/

#include "opentimers.h"
#include "openudp.h"

//=========================== define ==========================================

#define chang_PERIOD_MS 5000 // 5초마다

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
    opentimers_id_t     timerId;   ///< periodic timer which triggers transmission
    uint16_t             counter;  ///< incrementing counter which is written into the packet
    uint16_t              period;  ///< chang packet sending period>
    udp_resource_desc_t     desc;  ///< resource descriptor for this module, used to register at UDP stack
    bool      busySendingchang;  ///< TRUE when busy sending an chang
} chang_vars_t;

//=========================== prototypes ======================================

void chang_init(void);
void chang_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void chang_receive(OpenQueueEntry_t* msg);
/**
\}
\}
*/

#endif

