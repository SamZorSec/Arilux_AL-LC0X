#pragma once
#include "Effect.h"
#include "HSB.h"

/**
 * Transitions nicely between two HSB values
 */
class TransitionEffect : public Effect {
private:
    const HSB m_hsb;
    const int m_startMillis;
    const int m_endMillis;
    const int m_totalMillis;

public:
    /**
     * p_hsb     : Eding HSB value
     */
    TransitionEffect(const HSB& p_hsb, const unsigned long p_startMillis, const int p_millis);

    virtual HSB handleEffect(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& hsb);

    virtual bool hasModification(const unsigned long p_count,
                                 unsigned long p_time,
                                 const HSB& hsb) const;

    virtual bool isCompleted(const unsigned long p_count,
                             const unsigned long p_time,
                             const HSB& hsb) const;

    virtual HSB finalState(const unsigned long p_count,
                           const unsigned long p_time,
                           const HSB& hsb) const;

private:
    HSB calcHSB(const unsigned long p_count,
                const unsigned long p_time,
                const HSB& hsb) const;
};
