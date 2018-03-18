#include "RainbowEffect.h"

RainbowEffect::RainbowEffect() : Effect() {
}

HSB RainbowEffect::handleEffect(const unsigned long p_count,
                                const unsigned long p_time,
                                const HSB& hsb) {
    return HSB(p_count % 360, hsb.getSaturation(), hsb.getBrightness(), hsb.getWhite1(), hsb.getWhite2());
}

bool RainbowEffect::hasModification(const unsigned long p_count,
                                    const unsigned long p_time,
                                    const HSB& hsb) const {
    return true;
}

bool RainbowEffect::isCompleted(const unsigned long p_count,
                                const unsigned long p_time,
                                const HSB& hsb) const {
    return false;
}

HSB RainbowEffect::finalState(const unsigned long p_count,
                              const unsigned long p_time,
                              const HSB& hsb) const {
    return HSB(p_count % 359, hsb.getSaturation(), hsb.getBrightness(), hsb.getWhite1(), hsb.getWhite2());
}

