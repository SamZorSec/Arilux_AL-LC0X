#pragma once
#include "HSB.h"

/**
 * Base effect class
 */
class Effect {
public:
    /**
       * generate a new HSB color based on count or time or current HSB color
       */
    virtual HSB handleEffect(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& p_hsb);

    /*
       * SHould return true if any modifications are needed to be handled
       */
    virtual bool hasModification(const unsigned long p_count,
                                 const unsigned long p_time,
                                 const HSB& p_hsb) const = 0;

    /*
       * Should return true of the effect has been complated
       */
    virtual bool isCompleted(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& p_hsb) const = 0;

    /*
       * Caluculate the final state, usually this is the same value as from handleEffect
       * This method was added such that we always end up in a resonable state, for example during
       * flash we can ensure we don´t end up in a black state
       */
    virtual HSB finalState(const unsigned long p_count,
                           const unsigned long p_time,
                           const HSB& p_hsb) const = 0;
};
