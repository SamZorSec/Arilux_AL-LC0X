#include "TransitionEffect.h"
#include "Arduino.h" // Can we do better then addign Arduino.h ? This is needed for map()

TransitionEffect::TransitionEffect(const HSB& p_hsb,
                                   const unsigned long p_startMillis,
                                   const int p_millis) : Effect(),
    m_hsb(p_hsb),
    m_startMillis(p_startMillis),
    m_endMillis(p_startMillis + p_millis),
    m_totalMillis(p_millis) {
}

HSB TransitionEffect::handleEffect(const unsigned long p_count,
                                   const unsigned long p_time,
                                   const HSB& _hsb) {
    return calcHSB(p_count, p_time, _hsb);
}

bool TransitionEffect::hasModification(const unsigned long p_count,
                                       const unsigned long p_time,
                                       const HSB& hsb) const {
    return p_time - m_startMillis < m_totalMillis;
}

HSB TransitionEffect::finalState(const unsigned long p_count,
                                 const unsigned long p_time,
                                 const HSB& _hsb) const {
    return calcHSB(p_count, m_endMillis, _hsb);
}

bool TransitionEffect::isCompleted(const unsigned long p_count,
                                   const unsigned long p_time,
                                   const HSB& hsb) const {
    return p_time >= m_startMillis + m_totalMillis;
}

HSB TransitionEffect::calcHSB(const unsigned long p_count,
                              const unsigned long p_time,
                              const HSB& _hsb) const {
    const unsigned int percent = ((p_time - m_startMillis) * 1000) / m_totalMillis;
    const int m_hsbsPath = HSB::hueShortestPath(_hsb.getHue(), m_hsb.getHue());
    const int newHue = map(percent, 0, 1000, _hsb.getHue(), m_hsbsPath);
    return HSB(
               HSB::fixHue(newHue),
               map(percent, 0, 1000, _hsb.getSaturation(), m_hsb.getSaturation()),
               map(percent, 0, 1000, _hsb.getBrightness(), m_hsb.getBrightness()),
               map(percent, 0, 1000, _hsb.getWhite1(), m_hsb.getWhite1()),
               map(percent, 0, 1000, _hsb.getWhite2(), m_hsb.getWhite2()));
}
