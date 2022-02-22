#include "opendefs.h"

#include "opencoap.h"
#include "chang.h"

void openapps_init(void) {
   opencoap_init();
   chang_init();
}