#include "colorseffect.h"
#include <hsb.h>



ColorsEffect::ColorsEffect(const std::vector<float> p_colors, uint32_t p_durationPerColor, uint32_t p_startTime) :
    Effect(),
    m_colors(p_colors),
    m_durationPerColor(p_durationPerColor),
    m_startTime(p_startTime) {
}

HSB ColorsEffect::handleEffect(const uint32_t p_count,
                                const uint32_t p_time,
                                const HSB& p_hsb) const {
    if (m_colors.size() == 0) {
        return p_hsb;
    }
    const uint8_t item = ((p_time-m_startTime) / m_durationPerColor) % m_colors.size();                                    
    return p_hsb.toBuilder().hue(m_colors.at(item)).build();
}

HSB ColorsEffect::finalState(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& p_hsb) const {
    return p_hsb;
}
