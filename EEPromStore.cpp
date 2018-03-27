#include "EEPromStore.h"

#include <EEPROM.h>
#include <Arduino.h>
#include "debug.h"

#define HSB_START_ADDRESS 0

struct HSBEEPromData {
    uint16_t hue;
    uint16_t saturation;
    uint16_t brightness;
    uint16_t white1;
    uint16_t white2;
    uint16_t crc;
};

EEPromStore::EEPromStore(const uint16_t p_eepromAddress,
                         const uint32_t p_debounceWaitTime,
                         const uint32_t p_commitWaitTime) :

    m_eepromAddress(p_eepromAddress),
    m_debounceWaitTime(p_debounceWaitTime),
    m_commitWaitTime(p_commitWaitTime),
    m_startCommitTime(0),
    m_startDebounceTime(0),
    m_lastHSB(HSB(0, 0, 0, 0, 0)),
    m_hsbChanged(false)  {
}


const HSB EEPromStore::getHSB() const {
    if (m_hsbChanged) {
        return m_lastHSB;
    } else {
        HSBEEPromData data;
        EEPROM.get(m_eepromAddress, data);
        // const uint16_t crc = crc16(reinterpret_cast<uint8_t*>(&data), sizeof(data) - 2);
        const uint16_t crc = 0;

        if (crc == data.crc || true) {
            return HSB(data.hue, data.saturation, data.brightness, data.white1, data.white2);
        } else {
            return HSB(0, 0, 50, 0, 0);
        }
    }
}

bool EEPromStore::storeHSB(const HSB hsb) {
    // if we get a new HSB track it´s time when we received it
    // We are trying to prevent the first value to be stored
    // after the value hasn´t been set for a while. So when a user changes the
    // light we wait untill the user stops changing it.
    bool didStore = false;

    if (m_lastHSB != hsb) {
        m_startDebounceTime = millis();
        m_hsbChanged = true;
        m_lastHSB = hsb;
        DEBUG_PRINTLN(F("EEPromStore : Debounce"));
    } else {
        // Only write HSB to EEPROM if
        // if HSB changed..
        // If we are not debouncing, eg, user changes HSB within XX ms of last change
        // We didn´t write to EEPROM in the last m_commitWaitTime ms
        if (m_hsbChanged &&
            millis() - m_startDebounceTime > m_debounceWaitTime &&
            millis() - m_startCommitTime > m_commitWaitTime) {
            HSBEEPromData data = {
                hsb.hue(),
                hsb.getSaturation(),
                hsb.brightness(),
                hsb.white1(),
                hsb.white2()
            };
            data.crc = crc16(reinterpret_cast<uint8_t*>(&data), sizeof(data) - 2);
            EEPROM.put(m_eepromAddress, data);
            m_hsbChanged = false;
            m_startCommitTime = millis();
            didStore = true;
            DEBUG_PRINTLN(F("EEPromStore : Store"));
        }
    }

    return didStore;
}

void EEPromStore::initStore(HSB hsb) {
    m_lastHSB = hsb;
    m_hsbChanged = false;
    m_startDebounceTime = millis();
    m_startCommitTime = millis() - m_commitWaitTime;
}

void EEPromStore::commit() {
    EEPROM.commit();
}

uint16_t EEPromStore::crc16(uint8_t* a, uint16_t length) const {
    uint16_t crc = 0;

    for (uint16_t i = 0; i < length; i++) {
        crc = crc16Update(crc, a[i]);
    }

    return crc;
}

uint16_t EEPromStore::crc16Update(uint16_t crc, uint8_t a) const {
    int i;
    crc ^= a;

    for (i = 0; i < 8; ++i)  {
        if (crc & 1) {
            crc = (crc >> 1) ^ 0xA001;
        } else {
            crc = (crc >> 1);
        }
    }

    return crc;
}
