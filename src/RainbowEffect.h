#pragma once
#include <stdint.h>

#include "Effect.h"
#include <HSB.h>

/**
 * Cycle between all colors of the rainbow
 */
class RainbowEffect final : public Effect {
private:
public:
    RainbowEffect();

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb);

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& hsb) const;
};