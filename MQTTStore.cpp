#include "MQTTStore.h"

#include <Arduino.h>
#include "debug.h"

extern char jsonBuffer[512];

MQTTStore::MQTTStore(
    const char* p_baseTopic,
    const char* p_hsbTopic,
    const char* p_remoteBaseTopic,
    const char* p_stateTopic,
    const PubSubClient& p_mqttClient,
    const uint32_t p_debounceWaitTime,
    const bool p_stateInColorTopic) :
    Settings(0, p_debounceWaitTime),
    m_baseTopic(p_baseTopic),
    m_hsbTopic(p_hsbTopic),
    m_remoteBaseTopic(p_remoteBaseTopic),
    m_stateTopic(p_stateTopic),
    m_mqttClient(p_mqttClient),
    m_stateInColorTopic(p_stateInColorTopic) {
}


void MQTTStore::storeHsb(const SettingsDTO& settings) {
    char payloadBuffer[64];
    const HSB hsb = settings.hsb();
    sprintf(payloadBuffer, "hsb=%d,%.2f,%.2f,%.2f,%.2f",
            hsb.hue(),
            (hsb.saturation() / 10.2),
            (hsb.brightness() / 10.2),
            (hsb.white1() / 10.2),
            (hsb.white2() / 10.2)
           );

    if (m_stateInColorTopic) {
        sprintf(payloadBuffer + strlen(payloadBuffer), " state=%s", settings.power() ? STATE_ON : STATE_OFF);
    }

    publish(m_baseTopic, m_hsbTopic, payloadBuffer);
}

void MQTTStore::storeRemoteBase(const SettingsDTO& settings) {
    char payloadBuffer[16];
    sprintf(payloadBuffer, "%d", settings.remoteBase());
    publish(m_baseTopic, m_remoteBaseTopic, payloadBuffer);
}

void MQTTStore::storePower(const SettingsDTO& settings) {
    publish(m_baseTopic, m_stateTopic, settings.power() ? STATE_ON : STATE_OFF);

    if (m_stateInColorTopic) {
        storeHsb(settings);
    }
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
