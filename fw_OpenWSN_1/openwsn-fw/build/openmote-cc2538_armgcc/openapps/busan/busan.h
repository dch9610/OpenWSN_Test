#ifndef __BUSAN_H
#define __BUSAN_H

/**
\addtogroup AppUdp
\{
\addtogroup busan
\{
*/

#include "opentimers.h"
#include "openudp.h"

//=========================== define ==========================================

#define busan_PERIOD_MS 5000 // 5초마다

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
    opentimers_id_t     timerId;   ///< periodic timer which triggers transmission
    uint16_t             counter;  ///< incrementing counter which is written into the packet
    uint16_t              period;  ///< busan packet sending period>
    udp_resource_desc_t     desc;  ///< resource descriptor for this module, used to register at UDP stack
    bool      busySendingbusan;  ///< TRUE when busy sending an busan
} busan_vars_t;

//=========================== prototypes ======================================

void busan_init(void);
void busan_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void busan_receive(OpenQueueEntry_t* msg);
/**
\}
\}
*/

#endif

