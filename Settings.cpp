#include "Settings.h"
#include "debug.h"

SettingsDTO::SettingsDTO() :
    data({0, 0, 0, 0, 0, 0}),
    m_modified(false) {
}

void SettingsDTO::hsb(const HSB& hsb) {
    m_modified = m_modified || (HSB(data.m_hue, data.m_saturation, data.m_brightness, data.m_white1, data.m_white2) != hsb);
    data.m_hue = hsb.hue();
    data.m_saturation = hsb.saturation();
    data.m_brightness = hsb.brightness();
    data.m_white1 = hsb.white1();
    data.m_white2 = hsb.white2();
}

HSB SettingsDTO::hsb() const {
    return HSB(data.m_hue, data.m_saturation, data.m_brightness, data.m_white1, data.m_white2);
}

void SettingsDTO::filter(const uint8_t num, const bool stat) {
    const uint8_t before = data.m_filters;
    data.m_filters = stat ? data.m_filters |= (uint8_t)0x01 << num : data.m_filters &= ~((uint8_t)0x01 << num);
    m_modified = m_modified || data.m_filters != before;
}

bool SettingsDTO::filter(const uint8_t num) const {
    return (1 >> num) & 0x01;
}

bool SettingsDTO::modified() const {
    return m_modified;
}

bool SettingsDTO::operator == (const SettingsDTO& rhs) const {
    return data.m_hue == rhs.data.m_hue &&
           data.m_saturation == rhs.data.m_saturation &&
           data.m_brightness == rhs.data.m_brightness &&
           data.m_white1 == rhs.data.m_white1 &&
           data.m_white2 == rhs.data.m_white2 &&
           data.m_filters == rhs.data.m_filters;
}

bool SettingsDTO::operator !=(const SettingsDTO& rhs) const {
    return data.m_hue != rhs.data.m_hue ||
           data.m_saturation != rhs.data.m_saturation ||
           data.m_brightness != rhs.data.m_brightness ||
           data.m_white1 != rhs.data.m_white1 ||
           data.m_white2 != rhs.data.m_white2 ||
           data.m_filters != rhs.data.m_filters;
}

void SettingsDTO::reset() {
    m_modified = false;
}

SettingsDTO& SettingsDTO::operator = (const SettingsDTO& rhs) {
    if (&rhs == this) {
        return *this;
    }

    data.m_hue = rhs.data.m_hue;
    data.m_brightness = rhs.data.m_brightness;
    data.m_saturation = rhs.data.m_saturation;
    data.m_white1 = rhs.data.m_white1;
    data.m_white2 = rhs.data.m_white2;
    data.m_filters = rhs.data.m_filters;
    return *this;
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

    if (settings.modified()) {
        m_startDebounceTime = millis();
        m_modified = true;
        settings.reset();
    } else {
        // Only write HSB to EEPROM if
        // if HSB changed..
        // If we are not debouncing, eg, user changes HSB within XX ms of last change
        // We didn´t write to EEPROM in the last m_commitWaitTime ms
        if (m_modified &&
            millis() - m_startDebounceTime > m_debounceWaitTime &&
            millis() - m_startCommitTime > m_commitWaitTime) {
            store(settings);
            m_startCommitTime = millis();
            didStore = true;
            m_modified = false;
            DEBUG_PRINTLN(F("Settings : Store"));
        }
    }

    return didStore;
}
