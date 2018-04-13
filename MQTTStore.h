#pragma once
#include <stdint.h>
#include "Settings.h"
#include "PubSubClient.h" // https://github.com/knolleary/pubsubclient/releases/tag/v2.6


class MQTTStore final : public Settings {
private:
    PubSubClient m_mqttClient;
    const char* m_baseTopic;
    const char* m_hsbTopic;
    const char* m_remoteBaseTopic;
public:
    MQTTStore(
        const char* p_baseTopic,
        const char* p_hsbTopic,
        const char* p_remoteBaseTopic,
        const PubSubClient& p_mqttClient,
        const uint32_t p_debounceWaitTime);

private:
    virtual void storeHsb(const HSB& p_hsb) override;
    virtual void storeRemoteBase(const uint32_t p_remoteBase) override;
    void publish(const char* baseTopic, const char* topic, const char* payload);
};
