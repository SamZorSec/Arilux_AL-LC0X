#include "Settings.h"
#include "debug.h"

SettingsDTO::SettingsDTO() :
    SettingsDTO(HSB(0, 0, 50, 0, 0), 0, 0, false) {
}

SettingsDTO::SettingsDTO(const HSB& p_hsb,
                         const uint32_t p_remoteBase,
                         const uint8_t p_filters,
                         const bool p_power) :
    m_hsb(p_hsb),
    m_remoteBase(p_remoteBase),
    m_filter(p_filters),
    m_power(p_power),

    m_anyModified(false),
    mod_hsb(false),
    mod_remoteBase(false),
    mod_filter(false),
    mod_power(false) {
}

bool SettingsDTO::modified() const {
    return m_anyModified;
}

void SettingsDTO::reset() {
    mod_hsb = false;
    mod_remoteBase = false;
    mod_filter = false;
    mod_power = false;
    m_anyModified = false;
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
    DEBUG_PRINTLN(F("Settings : Store HSB"));
    storeHsb(settings);
    DEBUG_PRINTLN(F("Settings : Store Remote"));
    storeRemoteBase(settings);
    DEBUG_PRINTLN(F("Settings : Store Power"));
    storePower(settings);
    m_startCommitTime = millis();
    m_modified = false;
}
