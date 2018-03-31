#pragma once

#include <stdint.h>
#include "HSB.h"

struct blobData_t {
    uint16_t m_hue;
    uint16_t m_saturation;
    uint16_t m_brightness;
    uint16_t m_white1;
    uint16_t m_white2;
    uint16_t m_filters;
    uint32_t m_remoteBase;
};

class SettingsDTO final {
private:
    blobData_t m_data;
    bool m_modified;
public:
    explicit SettingsDTO();
    SettingsDTO(const blobData_t& p_data);
    void hsb(const HSB& hsb);
    HSB hsb() const;
    void filter(const uint8_t num, const bool stat);
    bool filter(const uint8_t num) const;
    void remote(const uint32_t p_remoteBase);
    uint32_t remote() const;
    bool modified() const;
    void reset();
    const blobData_t& blob() const;
    uint8_t blobSize() const;
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
    void forceStorage(SettingsDTO& settingsDTO);
};
