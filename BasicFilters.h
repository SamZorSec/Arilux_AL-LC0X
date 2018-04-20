#pragma once
#include "Filter.h"
#include "HSB.h"
#include <stdint.h>

/**
 * ALter the brightness of a HSB value
 */
class BrightnessFilter final : public Filter {
private:
    float m_brightness;
public:
    /**
     * p_brightness : Initial brightness
     */
    BrightnessFilter(const float p_brightness);

    /**
     * Set the brightness 0..100
     */
    void brightness(const float p_brightness);

    float brightness() const;

    virtual HSB handleFilter(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb);

};


class PowerFilter final : public Filter {
private:
    bool m_power;
public:
    /**
     * p_brightness : Initial brightness
     */
    PowerFilter(const bool p_power);

    /**
     * Set the brightness 0..100
     */
    void power(const bool p_brightness);

    bool power() const;

    virtual HSB handleFilter(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb);

};