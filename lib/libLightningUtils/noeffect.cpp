#include "noeffect.h"

NoEffect::NoEffect() : Effect() {
}

HSB NoEffect::handleEffect(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) {
    return p_hsb;
}

HSB NoEffect::finalState(const uint32_t p_count,
                         const uint32_t p_time,
                         const HSB& p_hsb) const {
    return p_hsb;
}
