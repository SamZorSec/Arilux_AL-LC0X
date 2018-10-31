#include "flasheffect.h"

FlashEffect::FlashEffect(const HSB p_hsb,
                         const uint32_t p_currentCount,
                         const uint8_t p_period,
                         const uint8_t p_pulseWidth) :
    Effect(),
    m_hsb(p_hsb),
    m_currentCount(p_currentCount),
    m_period(p_period),
    m_pulseWidth(p_pulseWidth) {
}

HSB FlashEffect::handleEffect(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& p_hsb) {
    const uint32_t cc = p_count % m_period;

    if (cc < m_pulseWidth) {
        return m_hsb;
    } else {
        return p_hsb;
    }
}

HSB FlashEffect::finalState(const uint32_t p_count,
                            const uint32_t p_time,
                            const HSB& p_hsb) const {
    return p_hsb;
}
