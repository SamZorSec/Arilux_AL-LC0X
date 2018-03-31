#include "MQTTStore.h"

#include <Arduino.h>
#include "debug.h"
#include <ArduinoJson.h>

extern char jsonBuffer[512];


MQTTStore::MQTTStore(const char* p_topic, const PubSubClient& p_mqttClient, const uint32_t p_debounceWaitTime) :
    Settings(0, p_debounceWaitTime),
    m_topic(p_topic),
    m_mqttClient(p_mqttClient) {
}

void MQTTStore::store(const SettingsDTO& settingsDTO) {
    static char jsonBuffer[64];
    const HSB& hsb = settingsDTO.hsb();
    sprintf(jsonBuffer, "{\"hsb\":{\"h\":%d,\"s\":%d,\"b\":%d},\"w1\":%d,\"w2\":%d}",
            hsb.hue(),
            (hsb.saturation() + 2) >> 2,
            (hsb.brightness() + 2) >> 2,
            (hsb.white1() + 2) >> 2,
            (hsb.white2() + 2) >> 2
           );

    if (m_mqttClient.publish(m_topic, jsonBuffer, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
    } else {
        DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
    }
}

