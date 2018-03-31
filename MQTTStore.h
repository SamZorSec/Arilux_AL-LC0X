#pragma once
#include <stdint.h>
#include "Settings.h"
#include "PubSubClient.h" // https://github.com/knolleary/pubsubclient/releases/tag/v2.6


class MQTTStore final : public Settings {
private:
    const char* m_topic;
    PubSubClient m_mqttClient;
public:
    MQTTStore(const char* p_topic, const PubSubClient& p_mqttClient, const uint32_t p_debounceWaitTime);

private:
    virtual void store(const SettingsDTO& settingsDTO) override;
};
