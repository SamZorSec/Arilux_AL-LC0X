#pragma once
#include <stdint.h>

#include "effect.h"
#include <hsb.h>

/**
 * Cycle between all colors of the rainbow
 */
class RainbowEffect final : public Effect {
private:
    const float m_startHue;
    const float m_rotationsSec;
    const uint32_t m_startTime;
public:
    RainbowEffect();
    RainbowEffect(float p_startHue, float p_rotationsSec, uint32_t m_startTime);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb);

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& hsb) const;
};
