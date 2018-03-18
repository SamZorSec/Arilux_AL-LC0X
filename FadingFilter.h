#pragma once
#include "Filter.h"
#include "HSB.h"

/**
 * Transitions nicely between two HSB values
 */
class FadingFilter : public Filter {
private:
    const float m_alpha;
    float m_cptHue;
    float m_cptSaturation;
    float m_cptBrightness;
    float m_cptWhite1;
    float m_cptWhite2;
    bool m_hasModification;

public:
    /**
     * hsb     : Starting HSB value
     * p_alpha : How quickly we change to the final value 0.05 is a good start
     */
    FadingFilter(const HSB _hsb, const float p_alpha);

    virtual HSB handleFilter(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& hsb);

};
