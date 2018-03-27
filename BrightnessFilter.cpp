#include "BrightnessFilter.h"
#include <Arduino.h> // for constrain

BrightnessFilter::BrightnessFilter(const float p_brightness) : Filter(),
    m_brightness(p_brightness) {
}

void BrightnessFilter::brightness(const float p_brightness) {
    m_brightness = p_brightness;
}

float BrightnessFilter::brightness() const {
    return m_brightness;
}

HSB BrightnessFilter::handleFilter(const uint32_t p_count,
                                   const uint32_t p_time,
                                   const HSB& _hsb) {
    uint16_t white1 = (float)_hsb.white1() / 100.0 * (float)m_brightness;
    uint16_t white2 = (float)_hsb.white2() / 100.0 * (float)m_brightness;
    uint16_t brightness = (float)_hsb.brightness() / 100.0 * (float)m_brightness;
    return _hsb.toBuilder()
           .white1(constrain(white1, 0, 1020))
           .white2(constrain(white2, 0, 1020))
           .brightness(constrain(brightness, 0, 1020))
           .build();
}
