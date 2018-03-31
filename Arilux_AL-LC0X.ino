/*
  Alternative firmware for Arilux AL-LC0X series of ESP8266 based RGB LED controllers.
  See the README at https://github.com/mertenats/Arilux_AL-LC0X for more information.
  Licensed under the MIT license.
*/
#include <memory>

#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino

#include "config.h"

#ifdef IR_REMOTE
#include <IRremoteESP8266.h> // https://github.com/markszabo/IRremoteESP8266
#endif
#ifdef RF_REMOTE
#include <RCSwitch.h> // https://github.com/sui77/rc-switch
#endif

#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <EEPROM.h>


#include "debug.h"

#include "Arilux.h"
#include "HSB.h"

// Included in code so we can increase packet size
// This is something that´s not possible with Arduino IDE, currently using v2.6
#define MQTT_MAX_PACKET_SIZE 256
#include "PubSubClient.h" // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

#include "Settings.h"
#include "EEPromStore.h"
#include "MQTTStore.h"

// Effects
#include "NoEffect.h"
#include "FlashEffect.h"
#include "RainbowEffect.h"
#include "TransitionEffect.h"

// Filters
#include "NoFilter.h"
#include "FadingFilter.h"
#include "BrightnessFilter.h"

// Number of ms per effect transistion, 20ms == 50 Hz
#define FRAMES_PER_SECOND        50
#define EFFECT_PERIOD_CALLBACK   (1000 / FRAMES_PER_SECOND)

char chipId[12];
char mqttClientID[32];
char mqttTopicPrefix[32];

// MQTT topics
char homeAssistantDiscoveryTopic[56];
char mqttLastWillTopic[44];
char mqttStateTopic[44];
char mqttCommandTopic[44];

// MQTT buffer
char friendlyName[48];
char jsonBuffer[512];

// Counter that keeps counting up, used for filters, effects or other means to keep EFFECT_PERIOD_CALLBACK
// of transitions
volatile uint32_t transitionCounter = 0;

// Keep track when the last time we ran the effect state changes
volatile uint32_t effectPeriodStartMillis = 0;

// Keep track when the last time we tried to reconnect to mqtt
volatile uint32_t currentMqttReconnect = 0;

// True when arduinoOTAInProgress is in progress, during start
// we use this to not move to the main event loop
volatile bool arduinoOTAInProgress = false;

// Holds the current working HSB color
HSB workingHsb(0, 0, 255, 0, 0);

// Hold´s the color of the last color generated and send to teh Arilux device
HSB currentHsb(0, 0, 255, 0, 0);

// Pointers to current effect and filter
std::unique_ptr<Effect> currentEffect(new NoEffect());
std::unique_ptr<Filter> currentFilter(new FadingFilter(workingHsb, FILTER_FADING_ALPHA));

// Filter to control overall brightness
BrightnessFilter brightnessFilter(100);

// Arilux device interface
Arilux arilux;

#ifdef RF_REMOTE
RCSwitch rcSwitch = RCSwitch();
#endif
#ifdef TLS
WiFiClientSecure wifiClient;
#else
WiFiClient wifiClient;
#endif
PubSubClient mqttClient(wifiClient);

// Settings
SettingsDTO eepromSettingsDTO;
SettingsDTO mqttSettingsDTO;

// Eeprom storage
EEPromStore eepromStore(0, 5000, EEPROM_COMMIT_WAIT_DELAY);

// MQTT Storage (state handler)
MQTTStore mqttStore(mqttStateTopic, mqttClient, MQTT_UPDATE_DELAY);

// emotyJSON root, to make a few code flows easer
const JsonObject& emptyJsonRoot = StaticJsonBuffer<2>().createObject();

///////////////////////////////////////////////////////////////////////////
//  SSL/TLS
///////////////////////////////////////////////////////////////////////////
/*
  Function called to verify the fingerprint of the MQTT server certificate
*/
#ifdef TLS
void verifyFingerprint() {
    DEBUG_PRINT(F("INFO: Connecting to "));
    DEBUG_PRINTLN(MQTT_SERVER);

    if (!wifiClient.connect(MQTT_SERVER, MQTT_PORT)) {
        DEBUG_PRINTLN(F("ERROR: Connection failed. Halting execution"));
        delay(1000);
        ESP.reset();
    }

    if (wifiClient.verify(TLS_FINGERPRINT, MQTT_SERVER)) {
        DEBUG_PRINTLN(F("INFO: Connection secure"));
    } else {
        DEBUG_PRINTLN(F("ERROR: Connection insecure! Halting execution"));
        delay(1000);
        ESP.reset();
    }
}
#endif

///////////////////////////////////////////////////////////////////////////
//  Utilities
///////////////////////////////////////////////////////////////////////////

/*
  Helper function to publish to a MQTT topic with the given payload
*/

void publishToMQTT(const char* topic, const char* payload) {
    if (mqttClient.publish(topic, payload, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
        DEBUG_PRINT(topic);
        DEBUG_PRINT(F(". Payload: "));
        DEBUG_PRINTLN(payload);
    } else {
        DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
    }
}

///////////////////////////////////////////////////////////////////////////
//  MQTT
///////////////////////////////////////////////////////////////////////////
/*
   Function called when a MQTT message arrived
   @param p_topic   The topic of the MQTT message
   @param p_payload The payload of the MQTT message
   @param p_length  The length of the payload
*/

HSB getNewColorState(const HSB& hsb, const JsonObject& root) {
    uint16_t white1, white2;
    uint16_t colors[3];
    hsb.getHSB(colors);

    if (root.containsKey("hsb")) {
        const JsonObject& hsbRoot = root["hsb"];

        if (hsbRoot.containsKey("h")) {
            colors[0] = constrain(hsbRoot["h"], 0, 359);
        }

        if (hsbRoot.containsKey("s")) {
            colors[1] = constrain(hsbRoot["s"], 0, 255) << 2;
        }

        if (hsbRoot.containsKey("b")) {
            colors[2] = constrain(hsbRoot["b"], 0, 255) << 2;
        }
    }

    if (root.containsKey("w1")) {
        white1 = constrain(root["w1"], 0, 255)  << 2;
    } else {
        white1 = hsb.white1();
    }

    if (root.containsKey("w2")) {
        white1 = constrain(root["w2"], 0, 255)  << 2;
    } else {
        white2 = hsb.white2();
    }

    return HSB(colors[0], colors[1], colors[2], white1, white2);
}

void callback(char* p_topic, byte* p_payload, uint16_t p_length) {
    if (strcmp(mqttCommandTopic, p_topic) == 0) {
        // Handle the MQTT topic of the received message
        if (p_length > sizeof(jsonBuffer) - 1) {
            DEBUG_PRINTLN(F("MQTT Payload to large."));
            return;
        }

        // Mem copy into own buffer, I am not sure if p_payload is null terminated
        memcpy(jsonBuffer, p_payload, p_length);
        jsonBuffer[p_length] = 0;
        DEBUG_PRINT(F("MQTT Message received: "));
        DynamicJsonBuffer incomingJsonPayload;
        const JsonObject& root = incomingJsonPayload.parseObject(jsonBuffer);

        if (!root.success()) {
            DEBUG_PRINTLN(F("parseObject() failed"));
            return;
        }

        DEBUG_PRINTLN(jsonBuffer);

        // Load filters
        if (root.containsKey(FILTER)) {
            DEBUG_PRINT(F("Filter :"));
            // Get the name straight from the filter or use the object
            const bool isFilterNameOnly = root[FILTER].is<char*>();
            const char* filterName = isFilterNameOnly ? root[FILTER] : root[FILTER][FNAME];
            const JsonObject& filterRoot = isFilterNameOnly ? emptyJsonRoot : root[FILTER];

            // Set Filters
            if (strcmp(filterName, FILTER_NONE) == 0) {
                DEBUG_PRINT(F(" " FILTER_NONE));
                currentFilter.reset(new NoFilter());
            } else if (strcmp(filterName, FILTER_FADING) == 0) {
                DEBUG_PRINT(F(" " FILTER_FADING " "));

                if (filterRoot.containsKey(FALPHA)) {
                    const float alpha = filterRoot[FALPHA];
                    DEBUG_PRINT(alpha);

                    if (alpha > 0.001 && alpha < 1.0) {
                        currentFilter.reset(new FadingFilter(workingHsb, alpha));
                    } else {
                        DEBUG_PRINT(F(FALPHA " must be > 0.001 && < 1.0"));
                    }
                } else {
                    DEBUG_PRINT(F(" "));
                    DEBUG_PRINT(FILTER_FADING_ALPHA);
                    currentFilter.reset(new FadingFilter(workingHsb, FILTER_FADING_ALPHA));
                }
            } else {
                DEBUG_PRINT(F(" "));
                DEBUG_PRINT(filterName);
                DEBUG_PRINT(F(" not found."));
            }

            DEBUG_PRINTLN(F(" done"));
        }

        // Load transitions
        if (root.containsKey(EFFECT)) {
            DEBUG_PRINT(F("Transition :"));
            const JsonObject& transitionRoot = root[EFFECT];

            if (!transitionRoot.containsKey(TNAME)) {
                DEBUG_PRINTLN(F(" no name found."));
                return;
            }

            const char* transitionName = transitionRoot[TNAME];
            workingHsb = currentEffect->finalState(transitionCounter, millis(), workingHsb);
            workingHsb = getNewColorState(workingHsb, root);
            const HSB transitionHSB = getNewColorState(workingHsb, transitionRoot);

            if (strcmp(transitionName, EFFECT_NONE) == 0) {
                DEBUG_PRINT(F(" " EFFECT_NONE));
                currentEffect.reset(new NoEffect());
            } else if (strcmp(transitionName, EFFECT_FLASH) == 0) {
                DEBUG_PRINT(F(" " EFFECT_FLASH " "));
                const uint8_t pulseWidth = transitionRoot.containsKey(TWIDTH) ? transitionRoot[TWIDTH] : FRAMES_PER_SECOND >> 1;
                DEBUG_PRINT(pulseWidth);

                if (transitionHSB == workingHsb) {
                    currentEffect.reset(new FlashEffect(workingHsb.toBuilder().brightness(0).build(),
                                                        transitionCounter, FRAMES_PER_SECOND, pulseWidth));
                } else {
                    currentEffect.reset(new FlashEffect(transitionHSB,
                                                        transitionCounter, FRAMES_PER_SECOND, pulseWidth));
                }
            } else if (strcmp(transitionName, EFFECT_RAINBOW) == 0) {
                DEBUG_PRINT(F(" " EFFECT_RAINBOW));
                currentEffect.reset(new RainbowEffect());
            } else if (strcmp(transitionName, EFFECT_FADE) == 0) {
                DEBUG_PRINT(F(" " EFFECT_FADE " "));
                const uint16_t timeMillis = transitionRoot.containsKey(TDURATION) ? transitionRoot[TDURATION] : 1000;
                currentEffect.reset(new TransitionEffect(transitionHSB, millis(), timeMillis));
                DEBUG_PRINT(timeMillis);
            } else {
                DEBUG_PRINT(F(" "));
                DEBUG_PRINT(transitionName);
                DEBUG_PRINT(F(" Unknown"));
            }

            DEBUG_PRINT(F(" done"));
        } else {
            workingHsb = getNewColorState(workingHsb, root);
        }

        // ON/OFF are light turning the device ON
        // So we load the values from eeProm but we ensure we have a brightness > 0
        if (root.containsKey(STATE)) {
            const char* state = root[STATE];
            HSB colorState = getNewColorState(workingHsb, root);

            if (strcmp(state, SON) == 0) {
                workingHsb = getOnState(colorState);
            } else if (strcmp(state, SOFF) == 0) {
                currentEffect.reset(new NoEffect());
                workingHsb = getOffState(colorState);
            }
        }

        // Base address of the remote control
        if (root.containsKey(REMOTECMD)) {
            eepromSettingsDTO.remote(root[REMOTECMD]);
        }

        // Load transitions
        if (root.containsKey(RESTARTCMD)) {
            ESP.restart();
        }

        // Force storing settings in eeprom
        if (root.containsKey(STORECMD)) {
            eepromStore.forceStorage(eepromSettingsDTO);
        }
    }
}

HSB getOffState(const HSB& hsb) {
    return hsb.toBuilder().white1(0).white2(0).brightness(0).build();
}

HSB getOnState(const HSB& hsb) {
    const HSB settings = eepromStore.get().hsb();
    return hsb.toBuilder()
           .white1(settings.white1())
           .white2(settings.white2())
           //           .brightness(constrain(settings.brightness(),5,1020))
           .brightness(settings.brightness())
           .build();
}

void connectMQTT(void) {
    if (!mqttClient.connected()) {
        if (millis() - currentMqttReconnect > 1000) {
            currentMqttReconnect = millis();

            if (mqttClient.connect(mqttClientID, MQTT_USER, MQTT_PASS, mqttLastWillTopic, 0, 1, "dead")) {
                if (mqttClient.subscribe(mqttCommandTopic)) {
                    DEBUG_PRINT(F("INFO: Sending the MQTT subscribe succeeded for topic: "));
                } else {
                    DEBUG_PRINT(F("ERROR: Sending the MQTT subscribe failed for topic: "));
                }

                DEBUG_PRINTLN(mqttCommandTopic);
                DEBUG_PRINTLN(F("INFO: The client is successfully connected to the MQTT broker"));
                publishToMQTT(mqttLastWillTopic, "alive");
#ifdef HOME_ASSISTANT_MQTT_DISCOVERY
                DynamicJsonBuffer outgoingJsonPayload;
                JsonObject& root = outgoingJsonPayload.createObject();
                root["name"] = friendlyName;
                root["platform"] = "mqtt_json";
                root["state_topic"] = mqttStateTopic;
                root["command_topic"] = mqttCommandTopic;
                root["brightness"] = true;
                root["rgb"] = true;
                root["white_value"] = true;
                root.printTo(jsonBuffer, sizeof(jsonBuffer));
                publishToMQTT(homeAssistantDiscoveryTopic, jsonBuffer);
#endif
            } else {
                DEBUG_PRINTLN(F("ERROR: The connection to the MQTT broker failed"));
                DEBUG_PRINT(F("Username: "));
                DEBUG_PRINTLN(MQTT_USER);
                //DEBUG_PRINT(F("Password: "));
                //DEBUG_PRINTLN(MQTT_PASS);
                DEBUG_PRINT(F("Broker: "));
                DEBUG_PRINTLN(MQTT_SERVER);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//  WiFi
///////////////////////////////////////////////////////////////////////////
/*
   Function called to setup the connection to the WiFi AP
*/

void setupWiFi() {
    delay(10);
    Serial.print(F("INFO: Connecting to: "));
    Serial.println(WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    wifiClient.setNoDelay(true);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());
    Serial.println();
    Serial.println(F("INFO: WiFi connected"));
    Serial.print(F("INFO: IP address: "));
    Serial.println(WiFi.localIP());
}
///////////////////////////////////////////////////////////////////////////
//  REMOTE
///////////////////////////////////////////////////////////////////////////
/*
   Function called to handle received RF codes from the remote
*/
#ifdef RF_REMOTE
void handleRFRemote(void) {
    if (rcSwitch.available()) {
        const uint32_t value = rcSwitch.getReceivedValue() - eepromSettingsDTO.remote();
        DEBUG_PRINT(F("Key Received : "));
        DEBUG_PRINT(value & 0xFFFF00);
        DEBUG_PRINT(F(" / key:"));
        DEBUG_PRINTLN(value);

        switch (value) {
            case ARILUX_REMOTE_KEY_BRIGHT_PLUS:
                brightnessFilter
                .brightness(constrain(brightnessFilter.brightness() + BRIGHTNESS_INCREASE, 0, 200));
                break;

            case ARILUX_REMOTE_KEY_BRIGHT_MINUS:
                brightnessFilter
                .brightness(constrain(brightnessFilter.brightness() + BRIGHTNESS_DECREASE, 0, 200));
                break;

            case ARILUX_REMOTE_KEY_OFF:
                workingHsb = currentEffect->finalState(transitionCounter, millis(), workingHsb);
                currentEffect.reset(new NoEffect());
                workingHsb = getOffState(workingHsb);
                break;

            case ARILUX_REMOTE_KEY_ON:
                workingHsb = currentEffect->finalState(transitionCounter, millis(), workingHsb);
                currentEffect.reset(new NoEffect());
                workingHsb = getOnState(workingHsb);
                break;

            case ARILUX_REMOTE_KEY_RED:
                workingHsb = HSB(0, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_GREEN:
                workingHsb = HSB(120, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_BLUE:
                workingHsb = HSB(240, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_WHITE:
                workingHsb = HSB(0, 0, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_ORANGE:
                workingHsb = HSB(25, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_LTGRN:
                workingHsb = HSB(120, 100 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_LTBLUE:
                workingHsb = HSB(240, 100 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_AMBER:
                workingHsb = HSB(49, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_CYAN:
                workingHsb = HSB(180, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_PURPLE:
                workingHsb = HSB(300, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_YELLOW:
                workingHsb = HSB(60, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_PINK:
                workingHsb = HSB(350, 64 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_TOGGLE:
                workingHsb = currentEffect->finalState(transitionCounter, millis(), workingHsb);
                currentEffect.reset(new NoEffect());
                break;

            case ARILUX_REMOTE_KEY_SPEED_PLUS:
                // TODO: Implement some incremantal speedup filter
                workingHsb = workingHsb.toBuilder().hue((workingHsb.hue() + 5 % 360)).build();
                break;

            case ARILUX_REMOTE_KEY_SPEED_MINUS:
                // TODO: Implement some incremantal speedup filter
                workingHsb = workingHsb.toBuilder().hue((workingHsb.hue() - 5 % 360)).build();
                break;

            case ARILUX_REMOTE_KEY_MODE_PLUS:
                workingHsb = workingHsb.toBuilder().saturation(constrain(workingHsb.saturation() + 5, 0, 1020)).build();
                break;

            case ARILUX_REMOTE_KEY_MODE_MINUS:
                workingHsb = workingHsb.toBuilder().saturation(constrain(workingHsb.saturation() - 5, 0, 1020)).build();
                break;

            default:
                DEBUG_PRINTLN(F("ERROR: RF code not defined"));
                break;
        }

        rcSwitch.resetAvailable();
    }
}
#endif

///////////////////////////////////////////////////////////////////////////
//  SETUP() AND LOOP()
///////////////////////////////////////////////////////////////////////////
void setup() {
    Serial.begin(115200);
    delay(500);
    // chipId : 00FF1234
    sprintf(chipId, "%08X", ESP.getChipId());
    // mqttTopicPrefix : RGBW/00FF1234
    sprintf(mqttTopicPrefix, MQTT_TOPIC_PREFIX_TEMPLATE, arilux.getColorString(), chipId);
    // mqttLastWillTopic :  RGBW/00FF1234/status
    sprintf(mqttLastWillTopic, MQTT_LASTWILL_TOPIC_TEMPLATE, mqttTopicPrefix);
    // mqttStateTopic : RGBW/00FF1234/json/state
    sprintf(mqttStateTopic, MQTT_STATE_TOPIC_TEMPLATE, mqttTopicPrefix);
    // mqttCommandTopic : RGBW/00FF1234/json/set
    sprintf(mqttCommandTopic, MQTT_COMMAND_TOPIC_TEMPLATE, mqttTopicPrefix);
    // mqttClientID : ARILUX00FF1234
    sprintf(mqttClientID, HOSTNAME_TEMPLATE, chipId);
    // friendlyName : Arilux LC11 RGBW LED Controller 00FF1234
    sprintf(friendlyName, "Arilux %s %s LED Controller %s", DEVICE_MODEL, arilux.getColorString(), chipId);
    // homeAssistantDiscoveryTopic : homeassistant/light/ARILUX_LC11_RGBW_00FF1234/config
    sprintf(homeAssistantDiscoveryTopic, "%s/light/ARILUX_%s_%s_%s/config", HOME_ASSISTANT_MQTT_DISCOVERY_PREFIX, DEVICE_MODEL, arilux.getColorString(), chipId);
    Serial.print("Hostname:");
    Serial.println(mqttClientID);
    WiFi.hostname(mqttClientID);
    // Setup Wi-Fi
    setupWiFi();
#ifdef TLS
    // Check the fingerprint of CloudMQTT's SSL cert
    ve | gerprint();
#endif
    // Init the Arilux LED controller
    arilux.init();
    // Set hostname and start OTA
    ArduinoOTA.setHostname(mqttClientID);
    ArduinoOTA.onStart([]() {
        arduinoOTAInProgress = true;
        DEBUG_PRINTLN(F("OTA Beginning!"));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        DEBUG_PRINT("ArduinoOTA Error[");
        DEBUG_PRINT(error);
        DEBUG_PRINT("]: ");

        if (error == OTA_AUTH_ERROR) {
            DEBUG_PRINTLN(F("Auth Failed"));
        } else if (error == OTA_BEGIN_ERROR) {
            DEBUG_PRINTLN(F("Begin Failed"));
        } else if (error == OTA_CONNECT_ERROR) {
            DEBUG_PRINTLN(F("Connect Failed"));
        } else if (error == OTA_RECEIVE_ERROR) {
            DEBUG_PRINTLN(F("Receive Failed"));
        } else if (error == OTA_END_ERROR) {
            DEBUG_PRINTLN(F("End Failed"));
        }
    });
    ArduinoOTA.begin();
#ifdef DEBUG_TELNET
    // Start the Telnet server
    startTelnet();
#endif
#ifdef PAUSE_FOR_OTA
    uint16_t i = 0;

    do {
        yield();
        uint16_t colors[3];
        HSB hsb(i, 255 * 4, 255, 0, 0);
        hsb.constantRGB(colors);
        arilux.setAll(colors[0], colors[1], colors[2], 0, 0);
        ArduinoOTA.handle();
        yield();
        delay(10);
        i++;
    } while (i < 360 * 2 || arduinoOTAInProgress);

#endif
#ifdef IR_REMOTE
    // Start the IR receiver
    irRecv.enableIRIn();
#endif
#ifdef RF_REMOTE
    // Start the RF receiver
    rcSwitch.enableReceive(ARILUX_RF_PIN);
#endif
    // Initialise the settings and prever settings from EEPROM during startup
    EEPROM.begin(32); // TODO make some way to get datasize from objects using eeprom
    eepromSettingsDTO = eepromStore.get();
    mqttSettingsDTO = eepromSettingsDTO;
    workingHsb = eepromSettingsDTO.hsb();
#ifdef DEBUG_TELNET
    handleTelnet();
#endif
    eepromStore.get();
    // Setup mqtt
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(callback);
    connectMQTT();
    effectPeriodStartMillis = millis();
}


void handleEffects() {
    const uint32_t currentMillies = millis();
    currentHsb = currentEffect->handleEffect(transitionCounter, currentMillies, workingHsb);
    currentHsb = brightnessFilter.handleFilter(transitionCounter, currentMillies, currentHsb);
    currentHsb = currentFilter->handleFilter(transitionCounter, currentMillies, currentHsb);
    mqttSettingsDTO.hsb(currentHsb);
    uint16_t colors[3];
    currentHsb.constantRGB(colors);
    arilux.setAll(colors[0], colors[1], colors[2], currentHsb.cwhite1(), currentHsb.cwhite2());
}


void onceASecond() {
#ifdef DEBUG_SERIAL || DEBUG_TELNET
    uint16_t colors[3];
    currentHsb.constantRGB(colors);
    lastRefreshSecondTime += REFRESH_INTERVAL;
    char str[128];
    sprintf(str, "rgb %d,%d,%d", colors[0], colors[1], colors[2]);
    DEBUG_PRINTLN(str);
    currentHsb.getHSB(colors);
    sprintf(str, "hsb %d,%d,%d w %d,%d", colors[0], colors[1], colors[2], currentHsb.white1(), currentHsb.white2());
    DEBUG_PRINTLN(str);
#endif
}

#define NUMBER_OF_SLOTS 15
float avarageTime = 0.0;
void loop() {
    const uint32_t currentMillis = millis();

    if (currentMillis - effectPeriodStartMillis >= EFFECT_PERIOD_CALLBACK) {
        effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
        transitionCounter++;
        handleEffects();

        if (transitionCounter % FRAMES_PER_SECOND == 10) {
            DEBUG_PRINTLN(avarageTime / transitionCounter);
            onceASecond();
        }

        uint8_t slot = 0;

        if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            ArduinoOTA.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            if (currentEffect->isCompleted(transitionCounter, currentMillis, workingHsb)) {
                DEBUG_PRINTLN(F("Transition Completed, setting NoEffect"));
                workingHsb = currentEffect->finalState(transitionCounter, millis(), workingHsb);
                currentEffect.reset(new NoEffect());
            }
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            yield();
        }

#ifdef DEBUG_TELNET
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle Telnet connection for debugging
            handleTelnet();
        }

#endif
#ifdef IR_REMOTE
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle received IR codes from the remote
            handleIRRemote();
        }

#endif
#ifdef RF_REMOTE
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle received RF codes from the remote
            handleRFRemote();
        }

#endif
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            connectMQTT();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            mqttClient.loop();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            mqttStore.handle(mqttSettingsDTO);
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            eepromSettingsDTO.hsb(workingHsb);

            // If the brightness was set to 0
            // We get a stored brightness and use the new color
            if (workingHsb.brightness() == 0 && eepromSettingsDTO.modified()) {
                SettingsDTO storedSettings = eepromStore.get();
                storedSettings.reset();
                const HSB storedHsb = storedSettings.hsb();
                storedSettings.hsb(workingHsb.toBuilder().brightness(storedHsb.brightness()).build());
                eepromStore.handle(storedSettings);
                eepromSettingsDTO.reset();
            } else {
                eepromStore.handle(eepromSettingsDTO);
            }
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
        }

        const uint32_t thisDuration = (millis() - currentMillis);

        if (thisDuration > EFFECT_PERIOD_CALLBACK) {
            DEBUG_PRINT(F("Spiked : "));
            DEBUG_PRINT(slot);
            DEBUG_PRINT(F(" "));
            DEBUG_PRINTLN(thisDuration);
        }

        avarageTime = avarageTime + thisDuration;
    }
}
