#include "HSB.h"

HSB::HSB(const uint16_t p_hue,
         const uint16_t p_saturation,
         const uint16_t p_brightness,
         const uint16_t p_white1,
         const uint16_t p_white2)
    : m_hue(p_hue % 360),
      m_saturation(p_saturation),
      m_brightness(p_brightness),
      m_white1(p_white1),
      m_white2(p_white2) {
}

HSB::HSB(const HSB& p_hsb)
    : m_hue(p_hsb.m_hue),
      m_saturation(p_hsb.m_saturation),
      m_brightness(p_hsb.m_brightness),
      m_white1(p_hsb.m_white1),
      m_white2(p_hsb.m_white2) {
}

HSBBuilder HSB::toBuilder() const {
    return HSBBuilder(m_hue, m_saturation, m_brightness, m_white1, m_white2);
}

void HSB::getHSB(uint16_t colors[]) const {
    colors[0] = m_hue;
    colors[1] = m_saturation;
    colors[2] = m_brightness;
}

uint16_t HSB::hue() const {
    return m_hue;
}

uint16_t HSB::saturation() const {
    return m_saturation;
}

uint16_t HSB::brightness() const {
    return m_brightness;
}

uint16_t HSB::white1() const {
    return m_white1;
}

uint16_t HSB::white2() const {
    return m_white2;
}

uint16_t HSB::cwhite1() const {
    return m_white1;
}

uint16_t HSB::cwhite2() const {
    return m_white2;
}

void HSB::constantRGB(uint16_t colors[]) const {
    uint32_t r_temp, g_temp, b_temp;
    uint16_t inverse_sat = SBW_RANGE - m_saturation;
    uint16_t index_mod = m_hue % 120;

    if (m_hue < 120) {
        r_temp = 120 - index_mod;
        g_temp = index_mod;
        b_temp = 0;
    } else if (m_hue < 240) {
        r_temp = 0;
        g_temp = 120 - index_mod;
        b_temp = index_mod;
    } else if (m_hue < 360) {
        r_temp = index_mod;
        g_temp = 0;
        b_temp = 120 - index_mod;
    } else {
        r_temp = 0;
        g_temp = 0;
        b_temp = 0;
    }

    r_temp = ((r_temp * m_saturation) / 120) + inverse_sat;
    g_temp = ((g_temp * m_saturation) / 120) + inverse_sat;
    b_temp = ((b_temp * m_saturation) / 120) + inverse_sat;
    r_temp = (r_temp * m_brightness) / SBW_RANGE;
    g_temp = (g_temp * m_brightness) / SBW_RANGE;
    b_temp = (b_temp * m_brightness) / SBW_RANGE;
    colors[0]   = r_temp;
    colors[1]   = g_temp;
    colors[2]   = b_temp;
}

bool HSB::operator ==(const HSB& rhs) const {
    return m_hue == rhs.m_hue &&
           m_brightness == rhs.m_brightness &&
           m_saturation == rhs.m_saturation &&
           m_white1 == rhs.m_white1 &&
           m_white2 == rhs.m_white2;
}

bool HSB::operator !=(const HSB& rhs) const {
    return !(*this == rhs);
}

HSB& HSB::operator = (const HSB& rhs) {
    if (&rhs == this) {
        return *this;
    }

    m_hue = rhs.m_hue;
    m_brightness = rhs.m_brightness;
    m_saturation = rhs.m_saturation;
    m_white1 = rhs.m_white1;
    m_white2 = rhs.m_white2;
    return *this;
}
