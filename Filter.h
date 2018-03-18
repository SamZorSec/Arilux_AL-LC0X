#pragma once
#include "HSB.h"

/**
 * Base effect class
 */
class Filter {
public:
    /**
       * generate a new HSB color based on count or time or current HSB color
       */
    virtual HSB handleFilter(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& p_hsb);
};