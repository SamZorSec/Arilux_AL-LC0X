#include "fadingfilter.h"

FadingFilter::FadingFilter(const HSB _hsb, const float p_alpha) : Filter(),
    m_alpha(p_alpha),
    m_cptHue(_hsb.hue()),
    m_cptSaturation(_hsb.saturation()),
    m_cptBrightness(_hsb.brightness()),
    m_cptWhite1(_hsb.white1()),
    m_cptWhite2(_hsb.white2()) {
}

float newCValue(const float m_alpha, const float toV, const float fromV) {
    return  fromV + (toV - fromV) * m_alpha;
}

HSB FadingFilter::handleFilter(const uint32_t p_count,
                               const uint32_t p_time,
                               const HSB& _hsb) {
    const auto sPathHue = HSB::hueShortestPath((float)_hsb.hue(), m_cptHue);
    const auto dwHue = _hsb.hue() - sPathHue;
    const auto dwSat = _hsb.saturation() - m_cptSaturation;
    const auto dwBright = _hsb.brightness() - m_cptBrightness;
    const auto dw1 = _hsb.white1() - m_cptWhite1;
    const auto dw2 = _hsb.white2() - m_cptWhite2;
    m_cptHue = sPathHue + dwHue * m_alpha;
    m_cptSaturation = m_cptSaturation + dwSat * m_alpha;
    m_cptBrightness = m_cptBrightness + dwBright * m_alpha;
    m_cptWhite1 = m_cptWhite1 + dw1 * m_alpha;
    m_cptWhite2 = m_cptWhite2 + dw2 * m_alpha;
    return HSB(HSB::fixHue(m_cptHue), m_cptSaturation, m_cptBrightness, m_cptWhite1, m_cptWhite2);
}

//    auto sPathHue = HSB::hueShortestPath((float)_hsb.hue(), m_cptHue);
//    m_cptHue = newCValue(m_alpha, sPathHue, m_cptHue);
//    m_cptSaturation = newCValue(m_alpha, _hsb.saturation(), m_cptSaturation);
//    m_cptBrightness = newCValue(m_alpha, _hsb.brightness(), m_cptBrightness);
//    m_cptWhite1 = newCValue(m_alpha, _hsb.white1(), m_cptWhite1);
//    m_cptWhite2 = newCValue(m_alpha, _hsb.white2(), m_cptWhite2);
//    return HSB(HSB::fixHue(m_cptHue), m_cptSaturation, m_cptBrightness, m_cptWhite1, m_cptWhite2);