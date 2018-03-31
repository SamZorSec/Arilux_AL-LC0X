#include "EEPromStore.h"

#include <EEPROM.h>
#include <Arduino.h>
#include "debug.h"

#define HSB_START_ADDRESS 0

EEPromStore::EEPromStore(const uint16_t p_eepromAddress,
                         const uint32_t p_debounceWaitTime,
                         const uint32_t p_commitWaitTime) :
    Settings(p_debounceWaitTime, p_commitWaitTime),
    m_eepromAddress(p_eepromAddress) {
}

SettingsDTO EEPromStore::get() const {
    SettingsDTO data;
    EEPROM.get(m_eepromAddress, data);
    const uint16_t calculcatedCRC = crc16(reinterpret_cast<uint8_t*>(&data), sizeof(data));
    uint16_t storedCRC;
    EEPROM.get(m_eepromAddress + sizeof(data), storedCRC);

    if (storedCRC == calculcatedCRC) {
        return data;
    } else {
        return SettingsDTO();
    }
}

void EEPromStore::store(const SettingsDTO& settingsDTO) {
    SettingsDTO data = settingsDTO;
    const uint16_t crc = crc16(settingsDTO.blob(), settingsDTO.blobSize());
    EEPROM.put(m_eepromAddress, settingsDTO);
    EEPROM.put(m_eepromAddress + sizeof(settingsDTO), crc);
    DEBUG_PRINTLN(F("EEPromStore : Store"));
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
