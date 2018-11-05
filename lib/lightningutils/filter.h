#pragma once
#include <stdint.h>

class HSB;

/**
 * Base effect class
 */
class Filter {
public:
    /**
       * generate a new HSB color based on count or time or current HSB color
       */
    virtual HSB handleFilter(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb);
};