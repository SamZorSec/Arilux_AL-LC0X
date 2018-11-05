#include "transitioneffect.h"
#include <hsb.h>

#ifndef UNIT_TEST
    #include <Arduino.h>
#else
    extern "C" uint32_t millis();
#endif

TransitionEffect::TransitionEffect(const HSB& p_hsb,
                                   const uint32_t p_startMillis,
                                   const uint32_t m_duration) : Effect(),
    m_hsb(p_hsb),
    m_startMillis(p_startMillis),
    m_endMillis(p_startMillis + m_duration),
    m_duration(m_duration) {
}

HSB TransitionEffect::handleEffect(const uint32_t p_count,
                                   const uint32_t p_time,
                                   const HSB& p_hsb) const {
    return calcHSB(p_count, p_time, p_hsb);
}

HSB TransitionEffect::finalState(const uint32_t p_count,
                                 const uint32_t p_time,
                                 const HSB& p_hsb) const {
    return calcHSB(p_count, m_endMillis, p_hsb);
}

bool TransitionEffect::isCompleted(const uint32_t p_count,
                                   const uint32_t p_time,
                                   const HSB& p_hsb) const {
    return p_time > m_endMillis;
}

HSB TransitionEffect::calcHSB(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& p_hsb) const {
    const float percent = ((p_time - m_startMillis) * 100) / m_duration;
    const float m_hsbsPath = HSB::hueShortestPath(p_hsb.hue(), m_hsb.hue());
    const float newHue = map(percent, 0.f, 100.f, p_hsb.hue(), m_hsbsPath);

    auto fmap = [](float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    };


    return HSB(
               HSB::fixHue(newHue),
               fmap(percent, 0.f, 100.f, p_hsb.saturation(), m_hsb.saturation()),
               fmap(percent, 0.f, 100.f, p_hsb.brightness(), m_hsb.brightness()),
               fmap(percent, 0.f, 100.f, p_hsb.white1(), m_hsb.white1()),
               fmap(percent, 0.f, 100.f, p_hsb.white2(), m_hsb.white2()));
}
