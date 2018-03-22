#pragma once
#include "Effect.h"
#include "HSB.h"
#include <stdint.h>

/**
 * Flash/Strobe between two different colors
 */
class FlashEffect : public Effect {
private:
    const uint32_t m_currentCount;
    const uint8_t m_period;
    const uint8_t m_pulseWidth;
    const HSB m_hsb;

public:
    FlashEffect(const HSB p_hsb,
                const uint32_t p_currentCount,
                const uint8_t p_period,
                const uint8_t p_pulseWidth);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) ;

    virtual bool isCompleted(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb)  const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb)  const;
};
