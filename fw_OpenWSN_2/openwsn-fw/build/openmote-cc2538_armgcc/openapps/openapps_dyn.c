#include "opendefs.h"

#include "opencoap.h"
#include "hwan.h"

void openapps_init(void) {
   opencoap_init();
   hwan_init();
}