#include "Arilux.h"

Arilux::Arilux(const uint8_t red_pin,
        const uint8_t green_pin,
        const uint8_t blue_pin,
        const uint8_t white1_pin,
        const uint8_t white2_pin) :
    m_redPin(red_pin),
    m_greenPin(green_pin),
    m_bluePin(blue_pin),
    m_white1Pin(white1_pin),
    m_white2Pin(white2_pin)
{
}

bool Arilux::init(void) const {
    analogWriteRange(ARILUX_PWM_RANGE);
    analogWriteFreq(ARILUX_PWM_FREQUENCY);
    pinMode(m_redPin, OUTPUT);
    pinMode(m_greenPin, OUTPUT);
    pinMode(m_bluePin, OUTPUT);
    if (m_white1Pin!=0) {
        pinMode(m_white1Pin, OUTPUT);
    }
    if (m_white2Pin!=0) {
        pinMode(m_white2Pin, OUTPUT);
    }
    return true;
}

bool Arilux::setAll(const float p_red, const float p_green, const float p_blue, const float p_white1, const float p_white2) const {
    auto clamp = [](uint16_t in) {
        return in > ARILUX_PWM_MAX_RANGE_VALUE ? ARILUX_PWM_RANGE : in < ARILUX_PWM_MIN_RANGE_VALUE ? 0 : in;
    };
    analogWrite(m_redPin, clamp(map(p_red, 0.f, 100.f, 0, ARILUX_RED_PWM_RANGE)));
    analogWrite(m_greenPin, clamp(map(p_green, 0.f, 100.f, 0, ARILUX_GREEN_PWM_RANGE)));
    analogWrite(m_bluePin, clamp(map(p_blue, 0.f, 100.f, 0, ARILUX_BLUE_PWM_RANGE)));
    if (m_white1Pin!=0) {
        analogWrite(m_white1Pin, clamp(map(p_white1, 0.f, 100.f, 0, ARILUX_WHITE1_PWM_RANGE)));
    }
    if (m_white2Pin!=0) {
        analogWrite(m_white2Pin, clamp(map(p_white2, 0.f, 100.f, 0, ARILUX_WHITE2_PWM_RANGE)));
    }
    return true;
}
