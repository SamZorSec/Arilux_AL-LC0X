#include "Settings.h"
#include "debug.h"

SettingsDTO::SettingsDTO() :
    SettingsDTO(HSB(0, 0, 50, 0, 0), 0, 0) {
}

SettingsDTO::SettingsDTO(const HSB& p_hsb, const uint32_t p_remoteBase, const uint8_t p_filters) :
    m_hsb(p_hsb),
    m_remoteBase(p_remoteBase),
    m_filters(p_filters),
    m_mHsb(false),
    m_mRemoteBase(false) {
}

void SettingsDTO::hsb(const HSB& p_hsb) {
    m_mHsb = m_mHsb || p_hsb != m_hsb;
    m_hsb = p_hsb;
}

HSB SettingsDTO::hsb() const {
    return m_hsb;
}

void SettingsDTO::filter(const uint8_t num, const bool stat) {
    //    const uint8_t before = m_data.m_filters;
    //    m_data.m_filters = stat ? m_data.m_filters |= (uint8_t)0x01 << num : m_data.m_filters &= ~((uint8_t)0x01 << num);
    //    m_modified = m_modified || m_data.m_filters != before;
}

bool SettingsDTO::filter(const uint8_t num) const {
    return 0;
    //    return (1 >> num) & 0x01;
}
void SettingsDTO::remote(const uint32_t p_remoteBase) {
    m_remoteBase = m_remoteBase || p_remoteBase != m_remoteBase;
    m_remoteBase = p_remoteBase;
}

uint32_t SettingsDTO::remote() const {
    return m_remoteBase;
}

bool SettingsDTO::modified() const {
    return m_mHsb || m_mRemoteBase;
}

void SettingsDTO::reset() {
    m_mRemoteBase = false;
    m_mHsb = false;
}

Settings::Settings(const uint32_t p_debounceWaitTime, const uint32_t p_commitWaitTime):
    m_debounceWaitTime(p_debounceWaitTime),
    m_commitWaitTime(p_commitWaitTime),
    m_startCommitTime(millis() - p_commitWaitTime),
    m_startDebounceTime(millis()),
    m_modified(false) {
}

bool Settings::handle(SettingsDTO& settings) {
    bool didStore = false;

    if (m_debounceWaitTime == 0) {
        if (settings.modified() &&
            millis() - m_startCommitTime > m_commitWaitTime) {
            forceStorage(settings);
            didStore = true;
        }
    } else {
        if (settings.modified()) {
            m_startDebounceTime = millis();
            m_modified = true;
        } else {
            // Only write HSB to EEPROM if
            // if HSB changed..
            // If we are not debouncing, eg, user changes HSB within XX ms of last change
            // We didn´t write to EEPROM in the last m_commitWaitTime ms
            if (m_modified &&
                millis() - m_startDebounceTime > m_debounceWaitTime &&
                millis() - m_startCommitTime > m_commitWaitTime) {
                forceStorage(settings);
                didStore = true;
            }
        }
    }

    return didStore;
}

void Settings::forceStorage(SettingsDTO& settings) {
    // TODO optimise for each changed variable
    // We have to keep this local for multiple settings
    storeHsb(settings.hsb());
    DEBUG_PRINTLN(F("Settings : Store HSB"));
    storeRemoteBase(settings.remote());
    DEBUG_PRINTLN(F("Settings : Store Remote"));
    m_startCommitTime = millis();
    m_modified = false;
}
