#pragma once
#include <stdint.h>
#include "effect.h"

class HSB;

class NoEffect final : public Effect {
private:
public:
    NoEffect();

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const;
};
