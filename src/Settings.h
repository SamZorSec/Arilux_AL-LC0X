#pragma once

#include <stdint.h>
#include <HSB.h>

#define SETTINGS_PROPERTY(typeIn,name) \
    private: \
    typeIn m_##name; \
    typeIn l_##name; \
    public: \
    typeIn name() const {return m_##name;} \
    void name(const typeIn& value){m_##name = value;}


class SettingsDTO final {
    
    SETTINGS_PROPERTY(HSB, hsb)
    SETTINGS_PROPERTY(uint32_t, remoteBase)
    SETTINGS_PROPERTY(uint8_t, filter)
    SETTINGS_PROPERTY(bool, power)
    SETTINGS_PROPERTY(float, brightness)

public:
    SettingsDTO(const HSB& p_hsb,
                         const uint32_t p_remoteBase,
                         const uint8_t p_filter,
                         const bool p_power,
                         const float p_brightness) :
    m_hsb(p_hsb),l_hsb(p_hsb),
    m_remoteBase(p_remoteBase),l_remoteBase(p_remoteBase),
    m_filter(p_filter),l_filter(p_filter),
    m_power(p_power),l_power(p_power),
    m_brightness(l_brightness),l_brightness(p_brightness) {
    }

    SettingsDTO() : SettingsDTO(HSB(0, 0, 50, 0, 0), 0, 0, true, 50) {
    }


    bool modified() const {
        return l_hsb != m_hsb || 
        l_remoteBase != m_remoteBase || 
        l_filter != m_filter ||
        l_power != m_power ||
        l_brightness != m_brightness;
    }

    void reset() {
        l_hsb = m_hsb;
        l_remoteBase = m_remoteBase;
        l_filter = m_filter;
        l_power = m_power;
        l_brightness = m_brightness;
    }

};

class Settings {
private:
    const uint32_t m_debounceWaitTime;
    const uint32_t m_commitWaitTime;
    uint32_t m_startCommitTime;
    uint32_t m_startDebounceTime;
private:
    virtual void store(const SettingsDTO& settings) = 0;
public:
    Settings(const uint32_t p_debounceWaitTime, const uint32_t p_startDebounceTime);
    bool handle(SettingsDTO& settingsDTO);
    void save(SettingsDTO& settingsDTO);
    void save(SettingsDTO& settingsDTO, bool force);
    void reset();
};


