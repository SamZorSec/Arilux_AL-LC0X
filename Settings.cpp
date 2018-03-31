#include "Settings.h"
#include "debug.h"

SettingsDTO::SettingsDTO() :
    m_data({0, 0, 50, 0, 0, 0, 0}),
m_modified(false) {
}

SettingsDTO::SettingsDTO(const blobData_t& p_data) :
    m_data(p_data),
    m_modified(false) {
}

void SettingsDTO::hsb(const HSB& hsb) {
    m_modified = m_modified || (HSB(m_data.m_hue, m_data.m_saturation, m_data.m_brightness, m_data.m_white1, m_data.m_white2) != hsb);
    m_data.m_hue = hsb.hue();
    m_data.m_saturation = hsb.saturation();
    m_data.m_brightness = hsb.brightness();
    m_data.m_white1 = hsb.white1();
    m_data.m_white2 = hsb.white2();
}

HSB SettingsDTO::hsb() const {
    return HSB(m_data.m_hue, m_data.m_saturation, m_data.m_brightness, m_data.m_white1, m_data.m_white2);
}

void SettingsDTO::filter(const uint8_t num, const bool stat) {
    const uint8_t before = m_data.m_filters;
    m_data.m_filters = stat ? m_data.m_filters |= (uint8_t)0x01 << num : m_data.m_filters &= ~((uint8_t)0x01 << num);
    m_modified = m_modified || m_data.m_filters != before;
}

bool SettingsDTO::filter(const uint8_t num) const {
    return (1 >> num) & 0x01;
}
void SettingsDTO::remote(const uint32_t p_remoteBase) {
    m_data.m_remoteBase = p_remoteBase;
}

uint32_t SettingsDTO::remote() const {
    return m_data.m_remoteBase;
}

bool SettingsDTO::modified() const {
    return m_modified;
}

bool SettingsDTO::operator == (const SettingsDTO& rhs) const {
    return m_data.m_hue == rhs.m_data.m_hue &&
           m_data.m_saturation == rhs.m_data.m_saturation &&
           m_data.m_brightness == rhs.m_data.m_brightness &&
           m_data.m_white1 == rhs.m_data.m_white1 &&
           m_data.m_white2 == rhs.m_data.m_white2 &&
           m_data.m_remoteBase == rhs.m_data.m_remoteBase &&
           m_data.m_filters == rhs.m_data.m_filters;
}

bool SettingsDTO::operator !=(const SettingsDTO& rhs) const {
    return m_data.m_hue != rhs.m_data.m_hue ||
           m_data.m_saturation != rhs.m_data.m_saturation ||
           m_data.m_brightness != rhs.m_data.m_brightness ||
           m_data.m_white1 != rhs.m_data.m_white1 ||
           m_data.m_white2 != rhs.m_data.m_white2 ||
           m_data.m_remoteBase != rhs.m_data.m_remoteBase ||
           m_data.m_filters != rhs.m_data.m_filters;
}

void SettingsDTO::reset() {
    m_modified = false;
}

const blobData_t&   SettingsDTO::blob() const {
    return m_data;
}
uint8_t SettingsDTO::blobSize() const {
    return sizeof(blobData_t);
}


SettingsDTO& SettingsDTO::operator = (const SettingsDTO& rhs) {
    if (&rhs == this) {
        return *this;
    }

    m_data.m_hue = rhs.m_data.m_hue;
    m_data.m_brightness = rhs.m_data.m_brightness;
    m_data.m_saturation = rhs.m_data.m_saturation;
    m_data.m_white1 = rhs.m_data.m_white1;
    m_data.m_white2 = rhs.m_data.m_white2;
    m_data.m_remoteBase = rhs.m_data.m_remoteBase;
    m_data.m_filters = rhs.m_data.m_filters;
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
            forceStorage(settings);
            didStore = true;
        }
    }

    return didStore;
}

void Settings::forceStorage(SettingsDTO& settings) {
    store(settings);
    m_startCommitTime = millis();
    m_modified = false;
    DEBUG_PRINTLN(F("Settings : Store"));
}
