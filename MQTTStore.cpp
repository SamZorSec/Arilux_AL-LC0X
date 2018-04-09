#include "MQTTStore.h"

#include <Arduino.h>
#include "debug.h"

extern char jsonBuffer[512];

MQTTStore::MQTTStore(
    const char* p_baseTopic,
    const char* p_hsbTopic,
    const char* p_remoteBaseTopic,
    const PubSubClient& p_mqttClient,
    const uint32_t p_debounceWaitTime) :
    Settings(0, p_debounceWaitTime),
    m_baseTopic(p_baseTopic),
    m_hsbTopic(p_hsbTopic),
    m_remoteBaseTopic(p_remoteBaseTopic),
    m_mqttClient(p_mqttClient) {
}

void MQTTStore::storeHsb(const HSB& p_hsb) {
    char topicBuffer[64];
    char payloadBuffer[64];
    sprintf(payloadBuffer, "hsb=%d,%.2f,%.2f,%.2f,%.2f",
            p_hsb.hue(),
            (p_hsb.saturation() / 10.2),
            (p_hsb.brightness() / 10.2),
            (p_hsb.white1() / 10.2),
            (p_hsb.white2() / 10.2)
           );
    publish(m_baseTopic, m_hsbTopic, payloadBuffer);
}

void MQTTStore::storeRemoteBase(const uint32_t p_remoteBase) {
    char topicBuffer[64];
    char payloadBuffer[16];
    sprintf(payloadBuffer, "%d", p_remoteBase);
    publish(m_baseTopic, m_remoteBaseTopic, payloadBuffer);
}

void MQTTStore::publish(const char* baseTopic, const char* topic, const char* payload) {
    char topicBuffer[64];
    strcpy(topicBuffer, baseTopic);
    strcat(topicBuffer, topic);

    if (m_mqttClient.publish(topicBuffer, payload, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
        DEBUG_PRINT(topicBuffer);
        DEBUG_PRINT(F(":"));
        DEBUG_PRINTLN(payload);
    } else {
        DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
    }
}