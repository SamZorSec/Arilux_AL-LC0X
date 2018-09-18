#pragma once

#include <stdint.h>
#include <math.h>

class HSBBuilder;


class HSB {
private:
    float m_hue;
    float m_saturation;
    float m_brightness;

    float m_white1;
    float m_white2;
public:
    explicit HSB(const float p_hue, const float p_saturation, const float p_brightness, const float p_white1, const float p_white2);
    HSB(const HSB& hsb);
    HSBBuilder toBuilder() const;

    void getHSB(float colors[]) const;

    float hue() const;
    float saturation() const;
    float brightness() const;

    float white1() const;
    float white2() const;

    float cwhite1() const;
    float cwhite2() const;

    /**
     * Create rgb colors 0..255
     */
    void constantRGB(float colors[]) const;

    /**
     * Calculate shortest path from one hue to the next hue
     * this might return hue values > 360 or < 0
     */
    template<typename T>
    static T hueShortestPath(T fromHue, T toHue) {
        const T distance = toHue - fromHue;

        if (distance > -180 && distance <= 180) {
            return toHue;
        } else {
            if (distance < 0) {
                return toHue + 360;
            } else {
                return toHue - 360;
            }
        }
    }

    /**
     *  Fix a hue if it´s outside of the range <0 will get
     *  360 added and >360 will get 360 substracted
     */
    static float fixHue(uint16_t hue) {
        return (hue < 0 ? hue + 360 : hue > 360 ? hue - 360 : hue) % 360;
    }

    /**
     *  Fix a hue if it´s outside of the range <0 will get
     *  360 added and >360 will get 360 substracted
     */
    static float fixHue(float hue) {
        return fmod((hue < 0.f ? hue + 360.f : hue > 360.f ? hue - 360.f : hue), 360.f);
    }

    bool operator ==(const HSB& rhs) const;
    bool operator !=(const HSB& rhs) const;
    HSB& operator  =(const HSB& rhs);

};

class HSBBuilder {
private:
    float m_hue;
    float m_saturation;
    float m_brightness;

    float m_white1;
    float m_white2;
public:
    HSBBuilder() :
        m_hue(0),    m_saturation(0), m_brightness(0), m_white1(0), m_white2(0) {
    }

    HSBBuilder(const HSB& hsb) :
        m_hue(hsb.brightness()),
        m_saturation(hsb.saturation()),
        m_brightness(hsb.brightness()),
        m_white1(hsb.white1()),
        m_white2(hsb.white2()) {
    }

    explicit HSBBuilder(float p_hue, float p_saturation, float p_brightness, float p_white1, float p_white2) :
        m_hue(p_hue), m_saturation(p_saturation), m_brightness(p_brightness), m_white1(p_white1), m_white2(p_white2) {
    }

    HSBBuilder& hue(float p_hue) {
        m_hue = p_hue;
        return *this;
    }
    HSBBuilder& saturation(float p_saturation) {
        m_saturation = p_saturation;
        return *this;
    }
    HSBBuilder& brightness(float p_brightness) {
        m_brightness = p_brightness;
        return *this;
    }
    HSBBuilder& white1(float p_white1) {
        m_white1 = p_white1;
        return *this;
    }
    HSBBuilder& white2(float p_white2) {
        m_white2 = p_white2;
        return *this;
    }
    HSB build() {
        return HSB(m_hue, m_saturation, m_brightness, m_white1, m_white2);
    }
};
