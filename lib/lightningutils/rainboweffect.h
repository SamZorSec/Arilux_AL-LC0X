#pragma once
#include <stdint.h>
#include "effect.h"

class HSB;

/**
 * Cycle between all colors of the rainbow
 */
class RainbowEffect final : public Effect {
private:
    const float m_startHue;
    const float m_secPerRotation;
    const uint32_t m_startTime;
    HSB calculateHsb(const uint32_t p_time, const HSB& hsb) const;
public:
    RainbowEffect(uint32_t p_startTime);
    explicit RainbowEffect(float p_startHue, float p_secPerRotation, uint32_t m_startTime);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const;
};
