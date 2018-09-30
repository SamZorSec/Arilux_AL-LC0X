#include <HSB.h>

HSB::HSB(const float p_hue,
         const float p_saturation,
         const float p_brightness,
         const float p_white1,
         const float p_white2)
    : m_hue(fmod(p_hue, 360.f)),
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

void HSB::getHSB(float colors[]) const {
    colors[0] = m_hue;
    colors[1] = m_saturation;
    colors[2] = m_brightness;
}

float HSB::hue() const {
    return m_hue;
}

float HSB::saturation() const {
    return m_saturation;
}

float HSB::brightness() const {
    return m_brightness;
}

float HSB::white1() const {
    return m_white1;
}

float HSB::white2() const {
    return m_white2;
}

float HSB::cwhite1() const {
    return m_white1;
}

float HSB::cwhite2() const {
    return m_white2;
}

void HSB::constantRGB(float colors[]) const {
    float r_temp, g_temp, b_temp;
    float inverse_sat = 100.0 - m_saturation;
    float m_brightness_mod = m_brightness / 100.f;
    float m_saturation_mod = m_saturation / 120.f;
    float index_mod = fmod(m_hue, 120.f);

    if (m_hue < 120.f) {
        r_temp = 120.f - index_mod;
        g_temp = index_mod;
        b_temp = 0.f;
    } else if (m_hue < 240.f) {
        r_temp = 0.f;
        g_temp = 120.f - index_mod;
        b_temp = index_mod;
    } else if (m_hue < 360.f) {
        r_temp = index_mod;
        g_temp = 0.f;
        b_temp = 120.f - index_mod;
    } else {
        r_temp = 0.f;
        g_temp = 0.f;
        b_temp = 0.f;
    }

    r_temp = (r_temp * m_saturation_mod) + inverse_sat;
    g_temp = (g_temp * m_saturation_mod) + inverse_sat;
    b_temp = (b_temp * m_saturation_mod) + inverse_sat;
    r_temp = (r_temp * m_brightness_mod);
    g_temp = (g_temp * m_brightness_mod);
    b_temp = (b_temp * m_brightness_mod);
    colors[0] = r_temp;
    colors[1] = g_temp;
    colors[2] = b_temp;
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
