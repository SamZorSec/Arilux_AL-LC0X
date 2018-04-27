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
    m_modifications() {
}

bool SettingsDTO::modified() const {
    return m_modifications.modified();
}

void SettingsDTO::reset() {
    m_modifications.reset();
}

Settings::Settings(const uint32_t p_debounceWaitTime, const uint32_t p_commitWaitTime):
    m_debounceWaitTime(p_debounceWaitTime),
    m_commitWaitTime(p_commitWaitTime),
    m_startCommitTime(millis() - p_commitWaitTime),
    m_startDebounceTime(millis()),
    m_modifications() {
}

bool Settings::handle(SettingsDTO& settings) {
    bool didStore = false;

    m_modifications = m_modifications | settings.modifications();
    if (m_debounceWaitTime == 0) {
        if (m_modifications.modified() &&
            millis() - m_startCommitTime > m_commitWaitTime) {
            store(settings);
            didStore = true;
        }
    } else {
        if (settings.modified()) {
            m_startDebounceTime = millis();
        } else {
            // Only write HSB to EEPROM if
            // if HSB changed..
            // If we are not debouncing, eg, user changes HSB within XX ms of last change
            // We didn´t write to EEPROM in the last m_commitWaitTime ms
            if (m_modifications.modified() &&
                millis() - m_startDebounceTime > m_debounceWaitTime &&
                millis() - m_startCommitTime > m_commitWaitTime) {
                store(settings);
                didStore = true;
            }
        }
    }

    return didStore;
}

void Settings::store(SettingsDTO& settings) {
    store(settings, false);
}

void Settings::store(SettingsDTO& settings, bool force) {

    if (m_modifications.hsb || force) {
        DEBUG_PRINTLN(F("Settings : Store HSB"));
        storeHsb(settings);
    }

    if (m_modifications.remoteBase || force) {
        DEBUG_PRINTLN(F("Settings : Store Remote"));
        storeRemoteBase(settings);
    }

    if (m_modifications.power || force) {
        DEBUG_PRINTLN(F("Settings : Store Power"));
        storePower(settings);
    }

    // Need to implement filter
    m_startCommitTime = millis();
    m_modifications.reset();
}
