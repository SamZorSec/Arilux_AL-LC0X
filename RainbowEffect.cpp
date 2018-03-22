#include "RainbowEffect.h"

RainbowEffect::RainbowEffect() : Effect() {
}

HSB RainbowEffect::handleEffect(const uint32_t p_count,
                                const uint32_t p_time,
                                const HSB& hsb) {
    return HSB(p_count % 360, hsb.getSaturation(), hsb.getBrightness(), hsb.getWhite1(), hsb.getWhite2());
}

bool RainbowEffect::isCompleted(const uint32_t p_count,
                                const uint32_t p_time,
                                const HSB& hsb) const {
    return false;
}

HSB RainbowEffect::finalState(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& hsb) const {
    return HSB(p_count % 359, hsb.getSaturation(), hsb.getBrightness(), hsb.getWhite1(), hsb.getWhite2());
}

