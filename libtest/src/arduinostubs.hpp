
#include <stdint.h>

#ifndef millis
uint32_t millisStubbed = 0;
extern "C" uint32_t millis() {
    return millisStubbed;
};
#endif
