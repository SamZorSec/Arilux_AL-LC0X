#pragma once

#include <stdint.h>
#include "HSB.h"

class SettingsDTO final {
private:
    HSB m_hsb;
    uint32_t m_remoteBase;
    uint8_t m_filters;
    bool m_mHsb;
    bool m_mRemoteBase;
public:
    explicit SettingsDTO();
    SettingsDTO(const HSB& p_hsb, const uint32_t p_remoteBase, const uint8_t p_filters);

    void hsb(const HSB& p_hsb);
    HSB hsb() const;
    bool hsbModified() const {
        return m_mHsb;
    };

    void filter(const uint8_t num, const bool stat);
    bool filter(const uint8_t num) const;
    bool filterModified() const {
        return false;
    }

    void remote(const uint32_t p_remoteBase);
    uint32_t remote() const;
    bool remoteModified() const {
        return m_mRemoteBase;
    };

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
    virtual void storeHsb(const HSB& hsb) = 0;
    virtual void storeRemoteBase(const uint32_t p_remoteBase) = 0;
public:
    Settings(const uint32_t p_debounceWaitTime, const uint32_t p_startDebounceTime);
    bool handle(SettingsDTO& settingsDTO);
    void forceStorage(SettingsDTO& settingsDTO);
};
