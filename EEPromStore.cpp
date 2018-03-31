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
    blobData_t data;
    uint16_t storedCRC=0;
    EEPROM.get(m_eepromAddress, data);
    EEPROM.get(m_eepromAddress + sizeof(blobData_t), storedCRC);
    const uint16_t calculcatedCRC = crc16(reinterpret_cast<uint8_t*>(&data), sizeof(blobData_t));

    if (storedCRC == calculcatedCRC) {
        DEBUG_PRINT(F("EEPromStore : CRC match "));
        return SettingsDTO(data);
    } else {
        DEBUG_PRINT(F("EEPromStore : CRC mismatch "));
        return SettingsDTO();
    }
}

void EEPromStore::store(const SettingsDTO& settingsDTO) {
    blobData_t data = settingsDTO.blob();
    uint16_t crc = crc16(reinterpret_cast<uint8_t*>(&data), sizeof(blobData_t));
    EEPROM.put(m_eepromAddress, data);
    EEPROM.put(m_eepromAddress + sizeof(blobData_t), crc);
    DEBUG_PRINTLN(F("EEPromStore : Store "));
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
