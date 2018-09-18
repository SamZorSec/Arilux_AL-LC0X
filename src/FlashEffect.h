#pragma once
#include <stdint.h>

#include "Effect.h"
#include "HSB.h"

/**
 * Flash/Strobe between two different colors
 */
class FlashEffect final : public Effect {
private:
    const HSB m_hsb;
    const uint32_t m_currentCount;
    const uint8_t m_period;
    const uint8_t m_pulseWidth;

public:
    FlashEffect(const HSB p_hsb,
                const uint32_t p_currentCount,
                const uint8_t p_period,
                const uint8_t p_pulseWidth);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) ;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb)  const;
};
