/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:46:55.864000.
*/
#include "opendefs_obj.h"
#include "uexpiration_monitor_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"
#ifdef DEADLINE_OPTION_ENABLED 
#include "iphc_obj.h"
#endif

//=========================== variables =======================================
umonitor_vars_t umonitor_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void umonitor_init(OpenMote* self) {
   // clear local variables
   memset(&umonitor_vars,0,sizeof(umonitor_vars_t));

   // register at UDP stack
   umonitor_vars.desc.port              = WKP_UDP_MONITOR;
   umonitor_vars.desc.callbackReceive   = &umonitor_receive;
   umonitor_vars.desc.callbackSendDone  = &umonitor_sendDone;
 openudp_register(self, &umonitor_vars.desc);
}

void umonitor_receive(OpenMote* self, OpenQueueEntry_t* request) {
   uint16_t          temp_l4_destination_port;
   OpenQueueEntry_t* reply;
#ifdef DEADLINE_OPTION_ENABLED  
   monitor_expiration_vars_t	deadline;
#endif
   
   reply = openqueue_getFreePacketBuffer(self, COMPONENT_UMONITOR);
   if (reply==NULL) {
 openserial_printError(self, 
         COMPONENT_UMONITOR,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
 openqueue_freePacketBuffer(self, request); //clear the request packet as well
      return;
   }
   
   reply->owner                         = COMPONENT_UMONITOR;
   reply->creator                       = COMPONENT_UMONITOR;
   reply->l4_protocol                   = IANA_UDP;
   temp_l4_destination_port             = request->l4_destination_port;
   reply->l4_destination_port           = request->l4_sourcePortORicmpv6Type;
   reply->l4_sourcePortORicmpv6Type     = temp_l4_destination_port;
   reply->l3_destinationAdd.type        = ADDR_128B;
   memcpy(&reply->l3_destinationAdd.addr_128b[0],&request->l3_sourceAdd.addr_128b[0],16);
   
   /*************** Packet Payload  ********************/
   // [Expiration time, Delay]   
 packetfunctions_reserveHeaderSize(self, reply,(2*sizeof(uint16_t)));
#ifdef DEADLINE_OPTION_ENABLED  
   memset(&deadline, 0, sizeof(monitor_expiration_vars_t));
 iphc_getDeadlineInfo(self, &deadline);
   memcpy(&reply->payload[0],&deadline.time_elapsed,sizeof(uint16_t));
   memcpy(&reply->payload[2],&deadline.time_left,sizeof(uint16_t));
#endif
 openqueue_freePacketBuffer(self, request);
   
   if (( openudp_send(self, reply))==E_FAIL) {
 openqueue_freePacketBuffer(self, reply);
   }
}

void umonitor_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
 openqueue_freePacketBuffer(self, msg);
}

bool umonitor_debugPrint(OpenMote* self) {
   return FALSE;
}

//=========================== private =========================================
