#pragma once
#include <stdint.h>

#include "Filter.h"
#include <hsb.h>

/**
 * Transitions nicely between two HSB values
 */
class FadingFilter final : public Filter {
private:
    const float m_alpha;
    float m_cptHue;
    float m_cptSaturation;
    float m_cptBrightness;
    float m_cptWhite1;
    float m_cptWhite2;

public:
    /**
     * hsb     : Starting HSB value
     * p_alpha : How quickly we change to the final value 0.f5 is a good start
     */
    FadingFilter(const HSB _hsb, const float p_alpha);

    virtual HSB handleFilter(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb);

};
