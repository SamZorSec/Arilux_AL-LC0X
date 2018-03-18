#pragma once
#include "Store.h"
#include "stdint.h"

class EEPromStore : public Store {
private:
    const int m_eepromAddress;
    const unsigned long m_debounceWaitTime;
    const unsigned long m_commitWaitTime;
    unsigned long m_startCommitTime;
    unsigned long m_startDebounceTime;

    HSB m_lastHSB;
    bool m_hsbChanged;

public:
    EEPromStore(const int p_eepromAddress, const unsigned long p_debounceWaitTime, const unsigned long p_commitWaitTime);

    /**
         * Get the current value from EEProm
         * Warning: If you just store a value in EEPROM but it wasn´t comitted yet,
         * you will may see different values
         */
    virtual const HSB getHSB() const;

    /**
         * StoreHSB valuer on EEProm.
         * It will take care that we de-bounce to actual store
         * untill it´s stable for backoffWait Time or CommitTime
         */
    virtual bool storeHSB(HSB hsb);
    virtual void initStore(HSB hsb);

private:
    uint16_t crc16(uint8_t* a, uint16_t length) const;
    uint16_t crc16Update(uint16_t crc, uint8_t a) const;
};
