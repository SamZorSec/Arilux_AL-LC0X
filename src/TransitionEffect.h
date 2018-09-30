#pragma once
#include <stdint.h>
#include "Effect.h"
#include <HSB.h>

/**
 * Transitions nicely between two HSB values
 */
class TransitionEffect final : public Effect {
private:
    const HSB m_hsb;
    const uint32_t m_startMillis;
    const uint32_t m_endMillis;
    const uint32_t m_duration;

public:
    /**
     * p_hsb     : Ending HSB value
     */
    TransitionEffect(const HSB& p_hsb, const uint32_t p_startMillis, const uint32_t m_duration);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb);

    virtual bool isCompleted(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& hsb) const;

private:
    HSB calcHSB(const uint32_t p_count,
                const uint32_t p_time,
                const HSB& hsb) const;
};
