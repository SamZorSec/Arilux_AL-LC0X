#pragma once

#include <stdint.h>
#include "HSB.h"

#define SETTINGS_PROPERTY(typeIn,name) \
    typeIn name() const {return m_##name;} \
    void name(const typeIn& value){mod_##name = mod_##name || value != m_##name; m_anyModified=m_anyModified || mod_##name;m_##name=value;}

class SettingsDTO final {
private:
    HSB m_hsb;
    uint32_t m_remoteBase;
    uint8_t m_filter;
    bool m_power;

    bool m_anyModified;
    bool mod_hsb;
    bool mod_remoteBase;
    bool mod_filter;
    bool mod_power;
public:
    explicit SettingsDTO();
    SettingsDTO(const HSB& p_hsb,
                const uint32_t p_remoteBase,
                const uint8_t p_filters,
                const bool p_power);

    SETTINGS_PROPERTY(HSB, hsb)
    SETTINGS_PROPERTY(bool, power)
    SETTINGS_PROPERTY(uint8_t, filter)
    SETTINGS_PROPERTY(uint32_t, remoteBase)

    bool modified() const;

    void reset();
};

class Settings {
private:
    const uint32_t m_debounceWaitTime;
    const uint32_t m_commitWaitTime;
    uint32_t m_startCommitTime;
    uint32_t m_startDebounceTime;
    bool m_modified;
private:
    virtual void storeHsb(const SettingsDTO& settings) = 0;
    virtual void storeRemoteBase(const SettingsDTO& settings) = 0;
    virtual void storePower(const SettingsDTO& settings) = 0;
public:
    Settings(const uint32_t p_debounceWaitTime, const uint32_t p_startDebounceTime);
    bool handle(SettingsDTO& settingsDTO);
    void forceStorage(SettingsDTO& settingsDTO);
};


