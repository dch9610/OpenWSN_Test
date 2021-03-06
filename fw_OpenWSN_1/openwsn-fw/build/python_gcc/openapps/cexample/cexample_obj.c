/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:46:47.213000.
*/
/**
\brief An example CoAP application.
*/

#include "opendefs_obj.h"
#include "cexample_obj.h"
#include "opencoap_obj.h"
#include "openqueue_obj.h"
#include "packetfunctions_obj.h"
#include "openserial_obj.h"
#include "openrandom_obj.h"
#include "scheduler_obj.h"
//#include "ADC_Channel.h"
#include "idmanager_obj.h"
#include "IEEE802154E_obj.h"
#include "schedule_obj.h"
#include "icmpv6rpl_obj.h"

//=========================== defines =========================================

/// inter-packet period (in ms)
#define CEXAMPLEPERIOD  10000
#define PAYLOADLEN      40

const uint8_t cexample_path0[] = "ex";

//=========================== variables =======================================

// declaration of global variable _cexample_vars_ removed during objectification.

//=========================== prototypes ======================================

owerror_t cexample_receive(OpenMote* self,  OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen);

void cexample_timer_cb(OpenMote* self, opentimers_id_t id);
void cexample_task_cb(OpenMote* self);
void cexample_sendDone(OpenMote* self, OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void cexample_init(OpenMote* self) {

    // prepare the resource descriptor for the /ex path
    (self->cexample_vars).desc.path0len             = sizeof(cexample_path0)-1;
    (self->cexample_vars).desc.path0val             = (uint8_t*)(&cexample_path0);
    (self->cexample_vars).desc.path1len             = 0;
    (self->cexample_vars).desc.path1val             = NULL;
    (self->cexample_vars).desc.componentID          = COMPONENT_CEXAMPLE;
    (self->cexample_vars).desc.securityContext      = NULL;
    (self->cexample_vars).desc.discoverable         = TRUE;
    (self->cexample_vars).desc.callbackRx           = &cexample_receive;
    (self->cexample_vars).desc.callbackSendDone     = &cexample_sendDone;


 opencoap_register(self, &(self->cexample_vars).desc);
    (self->cexample_vars).timerId    = opentimers_create(self, TIMER_GENERAL_PURPOSE, TASKPRIO_COAP);
 opentimers_scheduleIn(self, 
        (self->cexample_vars).timerId,
        CEXAMPLEPERIOD,
        TIME_MS,
        TIMER_PERIODIC,
        cexample_timer_cb
    );
}

//=========================== private =========================================

owerror_t cexample_receive(OpenMote* self,  OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen) {

   return E_FAIL;
}

void cexample_timer_cb(OpenMote* self, opentimers_id_t id){
    // calling the task directly as the timer_cb function is executed in
    // task mode by opentimer already
 cexample_task_cb(self);
}

void cexample_task_cb(OpenMote* self) {
    OpenQueueEntry_t*    pkt;
    owerror_t            outcome;
    uint8_t              i;
    coap_option_iht      options[2];

    open_addr_t          parentNeighbor;
    bool                 foundNeighbor;

    uint16_t             x_int       = 0;
    uint16_t             sum         = 0;
    uint16_t             avg         = 0;
    uint8_t              N_avg       = 10;
    uint8_t              medtype;

    // don't run if not synch
    if ( ieee154e_isSynch(self) == FALSE) {
        return;
    }

    // don't run on dagroot
    if ( idmanager_getIsDAGroot(self)) {
 opentimers_destroy(self, (self->cexample_vars).timerId);
        return;
    }

    foundNeighbor = icmpv6rpl_getPreferredParentEui64(self, &parentNeighbor);
    if (foundNeighbor==FALSE) {
        return;
    }

    if ( schedule_hasManagedTxCellToNeighbor(self, &parentNeighbor) == FALSE) {
        return;
    }

    if ((self->cexample_vars).busySendingCexample==TRUE) {
        // don't continue if I'm still sending a previous cexample packet
        return;
    }

    for (i = 0; i < N_avg; i++) {
        sum += x_int;
    }
    avg = sum/N_avg;

    // create a CoAP RD packet
    pkt = openqueue_getFreePacketBuffer(self, COMPONENT_CEXAMPLE);
    if (pkt==NULL) {
 openserial_printError(self, 
            COMPONENT_CEXAMPLE,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }
    // take ownership over that packet
    pkt->creator                   = COMPONENT_CEXAMPLE;
    pkt->owner                     = COMPONENT_CEXAMPLE;
    // CoAP payload
 packetfunctions_reserveHeaderSize(self, pkt,PAYLOADLEN);
    for (i=0;i<PAYLOADLEN;i++) {
        pkt->payload[i]             = i;
    }
    avg = openrandom_get16b(self);
    pkt->payload[0]                = (avg>>8)&0xff;
    pkt->payload[1]                = (avg>>0)&0xff;

    // set location-path option
    options[0].type = COAP_OPTION_NUM_URIPATH;
    options[0].length = sizeof(cexample_path0) - 1;
    options[0].pValue = (uint8_t *) cexample_path0;

    // set content-type option
    medtype = COAP_MEDTYPE_APPOCTETSTREAM;
    options[1].type = COAP_OPTION_NUM_CONTENTFORMAT;
    options[1].length = 1;
    options[1].pValue = &medtype;

    // metadata
    pkt->l4_destination_port       = WKP_UDP_COAP;
    pkt->l3_destinationAdd.type    = ADDR_128B;
    // does the ipAddr_motesEecs still work?
    memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motesEecs,16);

    // send
    outcome = opencoap_send(self, 
        pkt,
        COAP_TYPE_NON,
        COAP_CODE_REQ_PUT,
        1, // token len
        options,
        2, // options len
        &(self->cexample_vars).desc
    );

    // avoid overflowing the queue if fails
    if (outcome==E_FAIL) {
 openqueue_freePacketBuffer(self, pkt);
    } else {
        (self->cexample_vars).busySendingCexample = TRUE;
    }

    return;
}

void cexample_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {

    // free the packet buffer entry
 openqueue_freePacketBuffer(self, msg);

    // allow to send next cexample packet
    (self->cexample_vars).busySendingCexample = FALSE;
}



