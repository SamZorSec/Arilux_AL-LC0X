#pragma once
#include <stdint.h>

#include "Effect.h"
#include "HSB.h"

class NoEffect : public Effect {
private:
public:
    NoEffect();

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb);

    /**
     * This effect is unique such that the NoEffect filter never ends
     * even though it doesn´t change the value.
     */
    virtual bool isCompleted(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const;
};
