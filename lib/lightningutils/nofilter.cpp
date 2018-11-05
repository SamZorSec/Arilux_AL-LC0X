#include "nofilter.h"
#include <hsb.h>

NoFilter::NoFilter() : Filter() {
}

HSB NoFilter::handleFilter(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) {
    return p_hsb;
}
