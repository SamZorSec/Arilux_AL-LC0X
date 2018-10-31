#pragma once
#include <stdint.h>

#include "filter.h"
#include <hsb.h>

/**
 * Filter for color value
 * Main difference between a filter and effect is that that
 * a filter simply filters a color value for the purpose of color
 * corrections, smooth fadings etc.
 * A filter never ends.
 */
class NoFilter final : public Filter {
public:
    NoFilter();

    virtual HSB handleFilter(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb);

};
