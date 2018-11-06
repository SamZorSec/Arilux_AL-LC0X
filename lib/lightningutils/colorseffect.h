#pragma once
#include <stdint.h>
#include <vector>
#include "effect.h"

class HSB;



/**
 * Cycle between all colors of the rainbow
 */
class ColorsEffect final : public Effect {
private:
    const std::vector<float> m_colors;
    const uint32_t m_durationPerColor;
    const uint32_t m_startTime;

public:
    explicit ColorsEffect(const std::vector<float> p_colors, uint32_t p_durationPerColor, uint32_t p_startTime);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const;
};
