#include "Arilux.h"

Arilux::Arilux() : m_redPin(ARILUX_RED_PIN),
    m_greenPin(ARILUX_GREEN_PIN),
    m_bluePin(ARILUX_BLUE_PIN)
#if defined(RGBW) || defined(RGBWW)
    ,
    m_white1Pin(ARILUX_WHITE1_PIN)
#ifdef RGBWW
    ,
    m_white2Pin(ARILUX_WHITE2_PIN)
#endif
#endif
{
}

bool Arilux::init(void) const {
    analogWriteRange(ARILUX_PWM_RANGE);
    analogWriteFreq(ARILUX_PWM_FREQUENCY);
    pinMode(m_redPin, OUTPUT);
    pinMode(m_greenPin, OUTPUT);
    pinMode(m_bluePin, OUTPUT);
#if defined(RGBW) || defined(RGBWW)
    pinMode(m_white1Pin, OUTPUT);
#ifdef RGBWW
    pinMode(m_white2Pin, OUTPUT);
#endif
#endif
    return true;
}

const char* Arilux::getColorString(void) const {
    return (char*)ARILUX_COLOR_STRING;
}

bool Arilux::setAll(const uint16_t p_red, const uint16_t p_green, const uint16_t p_blue, const uint16_t p_white1, const uint16_t p_white2) const {
    if ((p_red > ARILUX_PWM_RANGE) ||
        (p_green > ARILUX_PWM_RANGE) ||
        (p_blue > ARILUX_PWM_RANGE) ||
        (p_white1 > ARILUX_PWM_RANGE) ||
        (p_white2 > ARILUX_PWM_RANGE)) {
        return false;
    }

    analogWrite(m_redPin, p_red);
    analogWrite(m_greenPin, p_green);
    analogWrite(m_bluePin, p_blue);
#if defined(RGBW) || defined(RGBWW)
    analogWrite(m_white1Pin, p_white1);
#ifdef RGBWW
    analogWrite(m_white2Pin, p_white2);
#endif
#endif
    return true;
}
