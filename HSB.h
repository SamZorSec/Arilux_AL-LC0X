#pragma once

#include <math.h>

class HSBBuilder;

class HSB {
private:
    int m_hue;
    int m_saturation;
    int m_brightness;

    int m_white1;
    int m_white2;
public:
    explicit HSB(int p_hue, int p_saturation, int p_brightness, int p_white1, int p_white2);
    HSB(const HSB& hsb);
    HSBBuilder toBuilder();

    void getHSB(int colors[]) const;

    int getHue() const;
    int getSaturation() const;
    int getBrightness() const;

    int getWhite1() const;
    int getWhite2() const;

    int getCWhite1() const;
    int getCWhite2() const;
    void getConstantRGB(int colors[]) const;

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
    static int fixHue(int hue) {
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
    int m_hue;
    int m_saturation;
    int m_brightness;

    int m_white1;
    int m_white2;
public:
    HSBBuilder() :
        m_hue(0),    m_saturation(0), m_brightness(0), m_white1(0), m_white2(0) {
    }

    HSBBuilder(const HSB& hsb) :
        m_hue(hsb.getBrightness()),
        m_saturation(hsb.getSaturation()),
        m_brightness(hsb.getBrightness()),
        m_white1(hsb.getWhite1()),
        m_white2(hsb.getWhite2()) {
    }

    explicit HSBBuilder(int p_hue, int p_saturation, int p_brightness, int p_white1, int p_white2) :
        m_hue(p_hue), m_saturation(p_saturation), m_brightness(p_brightness), m_white1(p_white1), m_white2(p_white2) {
    }

    HSBBuilder& hue(int p_hue) {
        m_hue = p_hue;
        return *this;
    }
    HSBBuilder& saturation(int p_saturation) {
        m_saturation = p_saturation;
        return *this;
    }
    HSBBuilder& brightness(int p_brightness) {
        m_brightness = p_brightness;
        return *this;
    }
    HSBBuilder& white1(int p_white1) {
        m_white1 = p_white1;
        return *this;
    }
    HSBBuilder& white2(int p_white2) {
        m_white2 = p_white2;
        return *this;
    }
    HSB build() {
        return HSB(m_hue, m_saturation, m_brightness, m_white1, m_white2);
    }
};
