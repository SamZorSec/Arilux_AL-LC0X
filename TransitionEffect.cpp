#include "TransitionEffect.h"
#include "Arduino.h" // Can we do better then addign Arduino.h ? This is needed for map()

TransitionEffect::TransitionEffect(const HSB& p_hsb,
                                   const uint32_t p_startMillis,
                                   const uint16_t p_millis) : Effect(),
    m_hsb(p_hsb),
    m_startMillis(p_startMillis),
    m_endMillis(p_startMillis + p_millis),
    m_totalMillis(p_millis) {
}

HSB TransitionEffect::handleEffect(const uint32_t p_count,
                                   const uint32_t p_time,
                                   const HSB& _hsb) {
    return calcHSB(p_count, p_time, _hsb);
}

HSB TransitionEffect::finalState(const uint32_t p_count,
                                 const uint32_t p_time,
                                 const HSB& _hsb) const {
    return calcHSB(p_count, m_endMillis, _hsb);
}

bool TransitionEffect::isCompleted(const uint32_t p_count,
                                   const uint32_t p_time,
                                   const HSB& hsb) const {
    return p_time >= m_startMillis + m_totalMillis;
}

HSB TransitionEffect::calcHSB(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& _hsb) const {
    const uint16_t percent = ((p_time - m_startMillis) * 1000) / m_totalMillis;
    const uint16_t m_hsbsPath = HSB::hueShortestPath(_hsb.hue(), m_hsb.hue());
    const uint16_t newHue = map(percent, 0, 1000, _hsb.hue(), m_hsbsPath);
    return HSB(
               HSB::fixHue(newHue),
               map(percent, 0, 1000, _hsb.getSaturation(), m_hsb.getSaturation()),
               map(percent, 0, 1000, _hsb.brightness(), m_hsb.brightness()),
               map(percent, 0, 1000, _hsb.white1(), m_hsb.white1()),
               map(percent, 0, 1000, _hsb.white2(), m_hsb.white2()));
}
