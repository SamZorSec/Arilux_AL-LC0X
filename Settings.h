#pragma once

#include <stdint.h>
#include "HSB.h"

#define SETTINGS_PROPERTY(typeIn,name) \
    typeIn name() const {return m_##name;} \
    void name(const typeIn& value){m_modifications.name = m_modifications.name || value != m_##name; m_##name = value;}

struct Modifications {
    Modifications() : hsb(false), remoteBase(false), filter(false), power(false) {}
    Modifications(bool p_Hsb, bool p_RemoteBase, bool p_Filter, bool p_Power) :
        hsb(p_Hsb), remoteBase(p_RemoteBase), filter(p_Filter), power(p_Power) {}
    bool hsb;
    bool remoteBase;
    bool filter;
    bool power;
    void reset() {
        hsb = false;
        remoteBase = false;
        filter = false;
        power = false;
    }
    bool modified() const {
        return hsb || power || remoteBase || filter;
    }
    Modifications& operator|(const Modifications& m) {
        hsb = hsb || m.hsb;
        remoteBase = remoteBase || m.remoteBase;
        filter = filter || m.filter;
        power = power || m.power;
        return *this;
    }

    Modifications& operator = (const Modifications& rhs) {
        if (&rhs == this) {
            return *this;
        }

        hsb = rhs.hsb;
        remoteBase = rhs.remoteBase;
        filter = rhs.filter;
        power = rhs.power;
        return *this;
    }
};

class SettingsDTO final {
private:
    HSB m_hsb;
    uint32_t m_remoteBase;
    uint8_t m_filter;
    bool m_power;

    Modifications m_modifications;
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

    const Modifications& modifications() const {
        return m_modifications;
    }

    bool modified() const;

    void reset();
};

class Settings {
private:
    const uint32_t m_debounceWaitTime;
    const uint32_t m_commitWaitTime;
    uint32_t m_startCommitTime;
    uint32_t m_startDebounceTime;
    Modifications m_modifications;
private:
    virtual void storeHsb(const SettingsDTO& settings) = 0;
    virtual void storeRemoteBase(const SettingsDTO& settings) = 0;
    virtual void storePower(const SettingsDTO& settings) = 0;
public:
    Settings(const uint32_t p_debounceWaitTime, const uint32_t p_startDebounceTime);
    bool handle(SettingsDTO& settingsDTO);
    void store(SettingsDTO& settingsDTO);
    void store(SettingsDTO& settingsDTO, bool force);
    void reset();
};


