#include "FlashEffect.h"

FlashEffect::FlashEffect(const HSB p_hsb,
                         const unsigned long p_currentCount,
                         const uint8_t p_period,
                         const uint8_t p_pulseWidth) :
    Effect(),
    m_hsb(p_hsb),
    m_currentCount(p_currentCount),
    m_period(p_period),
    m_pulseWidth(p_pulseWidth) {
}

HSB FlashEffect::handleEffect(const unsigned long p_count,
                              const unsigned long p_time,
                              const HSB& p_hsb) {
    const unsigned long cc = p_count % m_period;

    if (cc < m_pulseWidth) {
        return m_hsb;
    } else {
        return p_hsb;
    }
}

bool FlashEffect::hasModification(const unsigned long p_count,
                                  const unsigned long p_time,
                                  const HSB& p_hsb) const {
    const unsigned long cc = p_count % m_period;

    if (cc == 0 || (cc - m_pulseWidth) == 0) {
        return true;
    } else {
        return false;
    }
}

bool FlashEffect::isCompleted(const unsigned long p_count,
                              const unsigned long p_time,
                              const HSB& p_hsb) const {
    return false;
}

HSB FlashEffect::finalState(const unsigned long p_count,
                            const unsigned long p_time,
                            const HSB& p_hsb) const {
    return p_hsb;
}
