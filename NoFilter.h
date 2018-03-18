#pragma once
#include "Filter.h"
#include "HSB.h"

/**
 * Filter for color value
 * Main difference between a filter and effect is that that
 * a filter simply filters a color value for the purpose of color
 * corrections, smooth fadings etc.
 * A filter never ends.
 */
class NoFilter : public Filter {
public:
    NoFilter();

    virtual HSB handleFilter(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& p_hsb);

};
