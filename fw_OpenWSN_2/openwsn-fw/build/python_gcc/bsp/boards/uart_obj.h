/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:47:16.770000.
*/
#ifndef __UART_H
#define __UART_H

/**
\addtogroup BSP
\{
\addtogroup uart
\{

\brief Cross-platform declaration "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "Python.h"

#include "stdint.h"
#include "board_obj.h"
 
//=========================== define ==========================================

#define XOFF            0x13
#define XON             0x11
#define XONXOFF_ESCAPE  0x12
#define XONXOFF_MASK    0x10
// XOFF            is transmitted as [XONXOFF_ESCAPE,           XOFF^XONXOFF_MASK]==[0x12,0x13^0x10]==[0x12,0x03]
// XON             is transmitted as [XONXOFF_ESCAPE,            XON^XONXOFF_MASK]==[0x12,0x11^0x10]==[0x12,0x01]
// XONXOFF_ESCAPE  is transmitted as [XONXOFF_ESCAPE, XONXOFF_ESCAPE^XONXOFF_MASK]==[0x12,0x12^0x10]==[0x12,0x02]

//=========================== typedef =========================================

typedef enum {
   UART_EVENT_THRES,
   UART_EVENT_OVERFLOW,
} uart_event_t;

typedef void    (*uart_tx_cbt)(OpenMote* self);
typedef uint8_t (*uart_rx_cbt)(OpenMote* self);

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void uart_init(OpenMote* self);
void uart_setCallbacks(OpenMote* self, uart_tx_cbt txCb, uart_rx_cbt rxCb);
void uart_enableInterrupts(OpenMote* self);
void uart_disableInterrupts(OpenMote* self);
void uart_clearRxInterrupts(OpenMote* self);
void uart_clearTxInterrupts(OpenMote* self);
void uart_setCTS(OpenMote* self, bool state);
void uart_writeByte(OpenMote* self, uint8_t byteToWrite);
#ifdef FASTSIM
void uart_writeCircularBuffer_FASTSIM(OpenMote* self, uint8_t* buffer, uint16_t* outputBufIdxR, uint16_t* outputBufIdxW);
#endif
uint8_t uart_readByte(OpenMote* self);

// interrupt handlers
kick_scheduler_t uart_tx_isr(OpenMote* self);
kick_scheduler_t uart_rx_isr(OpenMote* self);

/**
\}
\}
*/

#endif
