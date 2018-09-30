#pragma once
#include <stdint.h>
#include "Settings.h"
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

class MQTTStore final : public Settings {
private:
    const char* m_baseTopic;
    const char* m_hsbTopic;
    const char* m_remoteBaseTopic;
    const char* m_stateTopic;
    PubSubClient m_mqttClient;
    bool m_stateInColorTopic;
public:
    MQTTStore(
        const char* p_baseTopic,
        const char* p_hsbTopic,
        const char* p_remoteBaseTopic,
        const char* p_stateTopic,
        const PubSubClient& p_mqttClient,
        const uint32_t p_debounceWaitTime,
        const bool p_stateInColorTopic);

private:
    virtual void store(const SettingsDTO& settings) override;

    virtual void storeHsb(const SettingsDTO& settings);
    virtual void storeRemoteBase(const SettingsDTO& settings);
    virtual void storePower(const SettingsDTO& settings);
    void publish(const char* baseTopic, const char* topic, const char* payload);
};
