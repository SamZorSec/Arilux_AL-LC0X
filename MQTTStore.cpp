#include "MQTTStore.h"

#include <Arduino.h>
#include "debug.h"
#include <ArduinoJson.h>

extern char jsonBuffer[512];


MQTTStore::MQTTStore(const char* p_topic, const PubSubClient& p_mqttClient, const unsigned long p_debounceWaitTime) : Store(),
    m_topic(p_topic),
    m_mqttClient(p_mqttClient),
    m_debounceWaitTime(p_debounceWaitTime),
    m_lastHSB(HSB(0, 0, 0, 0, 0)) {
}

// THis store does not do retreival
const HSB MQTTStore::getHSB() const {
    return m_lastHSB;
}

bool MQTTStore::storeHSB(const HSB hsb) {
    bool didStore = false;

    // Only write HSB to EEPROM if
    // if HSB changed..
    // And if didn´t write more than XX milli seconds ago
    if (m_lastHSB != hsb &&
        millis() - m_startDebounceTime > m_debounceWaitTime) {
        m_lastHSB = hsb;
        m_startDebounceTime = millis();
        didStore = true;
        storeState(m_lastHSB);
        DEBUG_PRINTLN(F("MQTTStore : Store"));
    }

    return didStore;
}

void MQTTStore::storeState(const HSB& hsb) {
    char jsonBuffer[64];
    sprintf(jsonBuffer, "{\"hsb\":{\"h\":%d,\"s\":%d,\"b\":%d},\"w1\":%d,\"w2\":%d}", 
        hsb.getHue(),
        (hsb.getSaturation() + 2) >> 2,
        (hsb.getBrightness() + 2) >> 2,
        (hsb.getWhite1() + 2) >> 2,
        (hsb.getWhite2() + 2) >> 2
    );
    publishToMQTT(m_topic, jsonBuffer);
}

void MQTTStore::publishToMQTT(const char* topic, const char* payload) {
    if (m_mqttClient.publish(topic, payload, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
        DEBUG_PRINT(topic);
        DEBUG_PRINT(F(". Payload: "));
        DEBUG_PRINTLN(payload);
    } else {
        DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
    }
}

void MQTTStore::initStore(HSB hsb) {
    m_lastHSB = hsb;
    m_startDebounceTime = millis();
}
