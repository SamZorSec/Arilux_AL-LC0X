#pragma once
#include "Store.h"
#include "PubSubClient.h" // https://github.com/knolleary/pubsubclient/releases/tag/v2.6


class MQTTStore : public Store {
private:
    const char* m_topic;
    PubSubClient m_mqttClient;
    const unsigned long m_debounceWaitTime;
    unsigned long m_startDebounceTime;
    HSB m_lastHSB;
public:
    MQTTStore(const char* p_topic, const PubSubClient& p_mqttClient, const unsigned long p_debounceWaitTime);

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
    void storeState(const HSB& hsb);
    void publishToMQTT(const char* topic, const char* payload);
};
