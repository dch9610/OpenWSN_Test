/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:46:37.399000.
*/
#include "opendefs_obj.h"
#include "openrandom_obj.h"
#include "idmanager_obj.h"

//=========================== variables =======================================

// declaration of global variable _random_vars_ removed during objectification.

//=========================== prototypes ======================================

//=========================== public ==========================================

void openrandom_init(OpenMote* self) {
    // seed the random number generator with the last 2 bytes of the MAC address
    (self->random_vars).shift_reg  = 0;
    (self->random_vars).shift_reg += idmanager_getMyID(self, ADDR_16B)->addr_16b[0]*256;
    (self->random_vars).shift_reg += idmanager_getMyID(self, ADDR_16B)->addr_16b[1];
}

uint16_t openrandom_get16b(OpenMote* self) {
    uint8_t  i;
    uint16_t random_value;
    random_value = 0;
    for(i=0;i<16;i++) {
        // Galois shift register
        // taps: 16 14 13 11
        // characteristic polynomial: x^16 + x^14 + x^13 + x^11 + 1
        random_value          |= ((self->random_vars).shift_reg & 0x01)<<i;
        (self->random_vars).shift_reg  = ((self->random_vars).shift_reg>>1)^(-(int16_t)((self->random_vars).shift_reg & 1)&0xb400);
    }
    return random_value;
}

uint16_t openrandom_getRandomizePeriod(OpenMote* self, uint16_t period, uint16_t range){
    uint16_t new_period;
    if (period<range){
        // randomly choose a new period from [period/2 ... period+period/2]
        new_period = period/2+ openrandom_get16b(self)%period;
    } else {
        new_period = period-range/2+ openrandom_get16b(self)%range;
    }
    return new_period;
}

//=========================== private =========================================