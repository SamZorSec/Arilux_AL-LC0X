#pragma once
#include <HSB.h>
#include <stdint.h>

/**
 * Base effect class
 */
class Effect {
public:
    /**
       * generate a new HSB color based on count or time or current HSB color
       */
    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb);

    /*
       * Should return true of the effect has been complated
       */
    virtual bool isCompleted(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const {
        return false;
    };

    /*
       * Caluculate the final state, usually this is the same value as from handleEffect
       * This method was added such that we always end up in a resonable state, for example during
       * flash we can ensure we don´t end up in a black state
       */
    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const = 0;
};
