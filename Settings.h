#pragma once

#include <stdint.h>
#include "HSB.h"

class SettingsDTO {
private:
    struct blobData_t {
        uint16_t m_hue;
        uint16_t m_saturation;
        uint16_t m_brightness;
        uint16_t m_white1;
        uint16_t m_white2;
        uint16_t m_filters;
    } data;
    bool m_modified;
public:
    SettingsDTO();
    void hsb(const HSB& hsb);
    HSB hsb() const;
    void filter(const uint8_t num, const bool stat);
    bool filter(const uint8_t num) const;
    bool modified() const;
    void reset();
    uint8_t* blob() const {
        return (uint8_t*)&data;
    }
    uint8_t blobSize() const {
        return sizeof(blobData_t);
    }

    bool operator ==(const SettingsDTO& rhs) const;
    bool operator !=(const SettingsDTO& rhs) const;
    SettingsDTO& operator = (const SettingsDTO& rhs);
};

class Settings {
private:
    const uint32_t m_debounceWaitTime;
    const uint32_t m_commitWaitTime;
    uint32_t m_startCommitTime;
    uint32_t m_startDebounceTime;
    bool m_modified;
private:
    virtual void store(const SettingsDTO& settingsDTO) = 0;
public:
    Settings(const uint32_t p_debounceWaitTime, const uint32_t p_startDebounceTime);
    bool handle(SettingsDTO& settingsDTO);
};
