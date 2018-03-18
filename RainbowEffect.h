#pragma once
#include "Effect.h"
#include "HSB.h"

/**
 * Cycle between all colors of the rainbow
 */
class RainbowEffect : public Effect {
private:
public:
    RainbowEffect();

    virtual HSB handleEffect(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& hsb);

    virtual bool hasModification(const unsigned long p_count,
                                 const unsigned long p_time,
                                 const HSB& hsb) const;

    virtual bool isCompleted(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& hsb) const;

    virtual HSB finalState(const unsigned long p_count,
                           const unsigned long p_time,
                           const HSB& hsb) const;
};
