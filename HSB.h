#pragma once

#include <math.h>
#include <stdint.h>

class HSBBuilder;

class HSB {
private:
    uint16_t m_hue;
    uint16_t m_saturation;
    uint16_t m_brightness;

    uint16_t m_white1;
    uint16_t m_white2;
public:
    explicit HSB(const uint16_t p_hue, const uint16_t p_saturation, const uint16_t p_brightness, const uint16_t p_white1, const uint16_t p_white2);
    HSB(const HSB& hsb);
    HSBBuilder toBuilder() const;

    void getHSB(uint16_t colors[]) const;

    uint16_t hue() const;
    uint16_t getSaturation() const;
    uint16_t brightness() const;

    uint16_t white1() const;
    uint16_t white2() const;

    uint16_t cwhite1() const;
    uint16_t cwhite2() const;
    void constantRGB(uint16_t colors[]) const;

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
    static uint16_t fixHue(uint16_t hue) {
        return (hue < 0 ? hue + 360 : hue > 360 ? hue - 360 : hue) % 360;
    }

    /**
     *  Fix a hue if it´s outside of the range <0 will get
     *  360 added and >360 will get 360 substracted
     */
    static float fixHue(float hue) {
        return fmod((hue < 0.0 ? hue + 360.0 : hue > 360.0 ? hue - 360.0 : hue), 360.0);
    }

    bool operator ==(const HSB& rhs) const;
    bool operator !=(const HSB& rhs) const;
    HSB& operator  =(const HSB& rhs);

};

class HSBBuilder {
private:
    uint16_t m_hue;
    uint16_t m_saturation;
    uint16_t m_brightness;

    uint16_t m_white1;
    uint16_t m_white2;
public:
    HSBBuilder() :
        m_hue(0),    m_saturation(0), m_brightness(0), m_white1(0), m_white2(0) {
    }

    HSBBuilder(const HSB& hsb) :
        m_hue(hsb.brightness()),
        m_saturation(hsb.getSaturation()),
        m_brightness(hsb.brightness()),
        m_white1(hsb.white1()),
        m_white2(hsb.white2()) {
    }

    explicit HSBBuilder(uint16_t p_hue, uint16_t p_saturation, uint16_t p_brightness, uint16_t p_white1, uint16_t p_white2) :
        m_hue(p_hue), m_saturation(p_saturation), m_brightness(p_brightness), m_white1(p_white1), m_white2(p_white2) {
    }

    HSBBuilder& hue(uint16_t p_hue) {
        m_hue = p_hue;
        return *this;
    }
    HSBBuilder& saturation(uint16_t p_saturation) {
        m_saturation = p_saturation;
        return *this;
    }
    HSBBuilder& brightness(uint16_t p_brightness) {
        m_brightness = p_brightness;
        return *this;
    }
    HSBBuilder& white1(uint16_t p_white1) {
        m_white1 = p_white1;
        return *this;
    }
    HSBBuilder& white2(uint16_t p_white2) {
        m_white2 = p_white2;
        return *this;
    }
    HSB build() {
        return HSB(m_hue, m_saturation, m_brightness, m_white1, m_white2);
    }
};
