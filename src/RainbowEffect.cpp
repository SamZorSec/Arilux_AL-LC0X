#include "RainbowEffect.h"

RainbowEffect::RainbowEffect() : Effect() {
}

HSB RainbowEffect::handleEffect(const uint32_t p_count,
                                const uint32_t p_time,
                                const HSB& hsb) {
    return HSB(p_count % 360, hsb.saturation(), hsb.brightness(), hsb.white1(), hsb.white2());
}

HSB RainbowEffect::finalState(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& hsb) const {
    return hsb;
}

