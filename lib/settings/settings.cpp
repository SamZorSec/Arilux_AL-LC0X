#include "settings.h"

#ifndef UNIT_TEST
    #include <Arduino.h>
#else
    extern "C" uint32_t millis();
#endif

Settings::Settings(
        const uint32_t p_debounceWaitTime,
        const uint32_t p_minWaitCommitTime,
        SaveFunction p_save,
        ModifiedFunction p_modified
):
    m_debounceWaitTime(p_debounceWaitTime),
    m_minWaitCommitTime(p_minWaitCommitTime),
    m_startCommitTime(millis() - p_minWaitCommitTime),
    m_startDebounceTime(millis()),
    m_modifiedLatch(false),
    m_callSave(p_save),
    m_callModified(p_modified) {
}

void Settings::handle() {
    const uint32_t currentMillis = millis();
    bool thisModified = m_callModified();
    m_modifiedLatch = m_modifiedLatch || thisModified;
    if (m_debounceWaitTime == 0) {
        save();
    } else {
        // keep resetting debounce time as long as we are within min commit time
        if (thisModified && (currentMillis - m_startCommitTime <= m_minWaitCommitTime)) {
            m_startDebounceTime = currentMillis;
        } else {
            if (currentMillis - m_startDebounceTime >= m_debounceWaitTime) {
                save(false);
            }
        }
    }
}

void Settings::save() {
    save(false);
}

void Settings::save(bool force) {
    const uint32_t currentMillis = millis();
    if (force || (m_modifiedLatch &&
        currentMillis - m_startCommitTime >= m_minWaitCommitTime)) {
        m_startCommitTime = currentMillis;
        m_callSave();
        m_modifiedLatch = false;
    }

}
