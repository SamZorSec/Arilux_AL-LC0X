#include "NoFilter.h"

NoFilter::NoFilter() : Filter() {
}

HSB NoFilter::handleFilter(const unsigned long p_count,
                           const unsigned long p_time,
                           const HSB& p_hsb) {
    return p_hsb;
}
