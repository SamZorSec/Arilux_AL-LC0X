#include "rainboweffect.h"
#include <hsb.h>

RainbowEffect::RainbowEffect(uint32_t p_startTime) :
    Effect(),
    m_startHue(0.0),
    m_secPerRotation(10.f),
    m_startTime(p_startTime) {
}

RainbowEffect::RainbowEffect(float p_startHue, float p_secPerRotation, uint32_t p_startTime) :
    Effect(),
    m_startHue(p_startHue),
    m_secPerRotation(p_secPerRotation),
    m_startTime(p_startTime) {
}

HSB RainbowEffect::handleEffect(const uint32_t p_count,
                                const uint32_t p_time,
                                const HSB& p_hsb) const {
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
    float hue = fmod( rotationSec / m_secPerRotation + m_startHue, 360.f);
    return HSB(hue, hsb.saturation(), hsb.brightness(), hsb.white1(), hsb.white2());              
}
