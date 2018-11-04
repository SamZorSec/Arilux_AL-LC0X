#include "rainboweffect.h"

RainbowEffect::RainbowEffect() : Effect(), m_startHue(0.0), m_rotationsSec(10.f), m_startTime(0) {
}

RainbowEffect::RainbowEffect(float p_startHue, float p_rotationsSec, uint32_t p_startTime) : 
    Effect(),
    m_startHue(p_startHue), 
    m_rotationsSec(p_rotationsSec), 
    m_startTime(p_startTime) {
}

HSB RainbowEffect::handleEffect(const uint32_t p_count,
                                const uint32_t p_time,
                                const HSB& p_hsb) {
    return calculateHsb(p_time, p_hsb);
}

HSB RainbowEffect::finalState(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& p_hsb) const {
    return calculateHsb(p_time, p_hsb);
}

HSB RainbowEffect::calculateHsb(const uint32_t p_time, const HSB& hsb) const {
    // rotationSec will count up to 360 in one second 
    float rotationSec = ((float)(p_time-m_startTime)) * (360.f / 1000.f);
    float hue = fmod( rotationSec / m_rotationsSec + m_startHue, 360.f);
    return HSB(hue, hsb.saturation(), hsb.brightness(), hsb.white1(), hsb.white2());              
}
