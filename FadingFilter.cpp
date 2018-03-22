#include "FadingFilter.h"

FadingFilter::FadingFilter(const HSB _hsb, const float p_alpha) : Filter(),
    m_alpha(p_alpha),
    m_cptHue(_hsb.getHue()),
    m_cptSaturation(_hsb.getSaturation()),
    m_cptBrightness(_hsb.getBrightness()),
    m_cptWhite1(_hsb.getWhite1()),
    m_cptWhite2(_hsb.getWhite2()) {
}

HSB FadingFilter::handleFilter(const uint32_t p_count,
                               const uint32_t p_time,
                               const HSB& _hsb) {
    const auto sPathHue = HSB::hueShortestPath((float)_hsb.getHue(), m_cptHue);
    const auto dwHue = _hsb.getHue() - sPathHue;
    const auto dwSat = _hsb.getSaturation() - m_cptSaturation;
    const auto dwBright = _hsb.getBrightness() - m_cptBrightness;
    const auto dw1 = _hsb.getWhite1() - m_cptWhite1;
    const auto dw2 = _hsb.getWhite2() - m_cptWhite1;
    m_cptHue = sPathHue + dwHue * m_alpha;
    m_cptSaturation = m_cptSaturation + dwSat * m_alpha;
    m_cptBrightness = m_cptBrightness + dwBright * m_alpha;
    m_cptWhite1 = m_cptWhite1 + dw1 * m_alpha;
    m_cptWhite2 = m_cptWhite2 + dw2 * m_alpha;
    return HSB(HSB::fixHue(m_cptHue), m_cptSaturation, m_cptBrightness, m_cptWhite1, m_cptWhite2);
}
