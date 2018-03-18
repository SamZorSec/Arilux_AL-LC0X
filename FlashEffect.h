#pragma once
#include "Effect.h"
#include "HSB.h"
#include <stdint.h>

/**
 * Flash/Strobe between two different colors
 */
class FlashEffect : public Effect {
private:
    const unsigned long m_currentCount;
    const uint8_t m_period;
    const uint8_t m_pulseWidth;
    const HSB m_hsb;

public:
    FlashEffect(const HSB p_hsb,
                const unsigned long p_currentCount,
                const uint8_t p_period,
                const uint8_t p_pulseWidth);

    virtual HSB handleEffect(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& p_hsb) ;

    virtual bool hasModification(const unsigned long p_count,
                                 const unsigned long p_time,
                                 const HSB& p_hsb)  const;

    virtual bool isCompleted(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& p_hsb)  const;

    virtual HSB finalState(const unsigned long p_count,
                           const unsigned long p_time,
                           const HSB& p_hsb)  const;
};
