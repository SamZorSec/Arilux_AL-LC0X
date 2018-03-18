#pragma once
#include "Effect.h"
#include "HSB.h"

class NoEffect : public Effect {
private:
public:
    NoEffect();

    virtual HSB handleEffect(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& p_hsb);

    virtual bool hasModification(const unsigned long p_count,
                                 unsigned long p_time,
                                 const HSB& p_hsb) const;

    /**
     * This effect is unique such that the NoEffect filter never ends
     * even though it doesn´t change the value.
     */
    virtual bool isCompleted(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& p_hsb) const;

    virtual HSB finalState(const unsigned long p_count,
                           const unsigned long p_time,
                           const HSB& p_hsb) const;
};
