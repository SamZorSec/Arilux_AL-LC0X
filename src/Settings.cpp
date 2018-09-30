#include "Settings.h"
#include "debug.h"

Settings::Settings(const uint32_t p_debounceWaitTime, const uint32_t p_commitWaitTime):
    m_debounceWaitTime(p_debounceWaitTime),
    m_commitWaitTime(p_commitWaitTime),
    m_startCommitTime(millis() - p_commitWaitTime),
    m_startDebounceTime(millis()) {
}

bool Settings::handle(SettingsDTO& settings) {
    bool didStore = false;

    if (m_debounceWaitTime == 0) {
        if (settings.modified() &&
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
            if (settings.modified() &&
                millis() - m_startDebounceTime > m_debounceWaitTime &&
                millis() - m_startCommitTime > m_commitWaitTime) {
                save(settings);
                didStore = true;
            }
        }
    }

    return didStore;
}

void Settings::save(SettingsDTO& settings) {
    save(settings, false);
}

void Settings::save(SettingsDTO& settings, bool force) {
    if (settings.modified() || force) {
        DEBUG_PRINTLN(F("Settings : Store"));
        store(settings);
    }

    m_startCommitTime = millis();
}
