#pragma once
#include "Settings.h"
#include "stdint.h"

class EEPromStore final : public Settings {
private:
    const uint16_t m_eepromAddress;

public:
    EEPromStore(const uint16_t p_eepromAddress, const uint32_t p_debounceWaitTime, const uint32_t p_commitWaitTime);
    SettingsDTO get() const;

private:
    virtual void store(const SettingsDTO& settingsDTO) override;
    uint16_t crc16(uint8_t* a, uint16_t length) const;
    uint16_t crc16Update(uint16_t crc, uint8_t a) const;
};
