#include "basicfilters.h"
#include "constrain.h"
#include <hsb.h>

BrightnessFilter::BrightnessFilter(const float p_increaseBy) :
    Filter(),
    m_increase(0),
    m_increaseBy(p_increaseBy) {
}

HSB BrightnessFilter::handleFilter(const uint32_t p_count,
                                   const uint32_t p_time,
                                   const HSB& _hsb) {
    if (m_increase == 0) {
        return _hsb;
    }

    if (m_increase == 1 && _hsb.brightness() >= 100.f && _hsb.white1() >= 100.f && _hsb.white2() >= 100.f) {
        m_increase = 0;
        return _hsb;
    } else if (m_increase == -1 && _hsb.brightness() == 0 && _hsb.white1() == 0 && _hsb.white2() == 0) {
        m_increase = 0;
        return _hsb;
    }

    float m_brightness;

    if (m_increase == 1) {
        m_brightness = m_increaseBy;
    } else if (m_increase == -1) {
        m_brightness = - m_increaseBy;
    } else {
      m_brightness = 0;
    }

    m_increase = 0;
    float brightness = _hsb.brightness() + _hsb.brightness() / 100.f * m_brightness;
    float white1 = _hsb.white1() + _hsb.white1() / 100.f * m_brightness;
    float white2 = _hsb.white2() + _hsb.white2() / 100.f * m_brightness;

    // Don´t allow to turn off with brightness controls
    // This has the side effect that we cannot turn on or if we do we don´t
    // know anymore what leds where on.
    if (white1 < 10.f && white2 < 10.f && brightness < 10.f) {
        return _hsb;
    }

    return _hsb.toBuilder()
           .white1(Helpers::constrain(white1, 0.f, 100.f))
           .white2(Helpers::constrain(white2, 0.f, 100.f))
           .brightness(Helpers::constrain(brightness, 0.f, 100.f))
           .build();
}

void BrightnessFilter::increase() {
    m_increase = 1;
}

void BrightnessFilter::decrease() {
    m_increase = -1;
}


PowerFilter::PowerFilter(const bool p_power) : Filter(),
    m_power(p_power) {
}

void PowerFilter::power(const bool p_power) {
    m_power = p_power;
}

bool PowerFilter::power() const {
    return m_power;
}

HSB PowerFilter::handleFilter(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& _hsb) {
    return _hsb.toBuilder()
           .white1(m_power ? _hsb.white1() : 0.f)
           .white2(m_power ? _hsb.white2() : 0.f)
           .brightness(m_power ? _hsb.brightness() : 0.f)
           .build();
}

