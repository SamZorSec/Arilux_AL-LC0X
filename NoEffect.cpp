#include "NoEffect.h"

NoEffect::NoEffect() : Effect() {
}

HSB NoEffect::handleEffect(const unsigned long p_count,
                           const unsigned long p_time,
                           const HSB& p_hsb) {
    return p_hsb;
}

bool NoEffect::hasModification(const unsigned long p_count,
                               const unsigned long p_time,
                               const HSB& p_hsb) const {
    return false;
}

bool NoEffect::isCompleted(const unsigned long p_count,
                           const unsigned long p_time,
                           const HSB& p_hsb) const {
    return false;
}

HSB NoEffect::finalState(const unsigned long p_count,
                         const unsigned long p_time,
                         const HSB& p_hsb) const {
    return p_hsb;
}
