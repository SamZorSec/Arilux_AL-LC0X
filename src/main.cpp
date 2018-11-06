/*
  Alternative firmware for Arilux AL-LC0X series of ESP8266 based RGB LED controllers.
  See the README at https://github.com/mertenats/Arilux_AL-LC0X for more information.
  Licensed under the MIT license.
*/
#include <memory>
#include "debug.h"

#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include "config.h"

#if defined(IR_REMOTE)
#include <IRremoteESP8266.h> // https://github.com/markszabo/IRremoteESP8266
#endif
#if defined(RF_REMOTE)
#include <RCSwitch.h> // https://github.com/sui77/rc-switch
#endif

#include <ArduinoOTA.h>
#include <ESP_EEPROM.h>

#include "arilux.h"
#include <hsb.h>

// Included in code so we can increase packet size
// This is something that´s not possible with Arduino IDE, currently using v2.6
// #define MQTT_MAX_PACKET_SIZE 128
// #define MQTT_MAX_TRANSFER_SIZE 128
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

#include <settings.h>
#include "eepromstore.h"
#include "mqttstore.h"
#include <optparser.h>
#include <statemachine.h>

// Effects
#include <noeffect.h>
#include <flasheffect.h>
#include <rainboweffect.h>
#include <transitioneffect.h>
#include <colorseffect.h>

// Filters
#include <nofilter.h>
#include <fadingfilter.h>
#include <basicfilters.h>

// Number of ms per effect transistion, 20ms == 50 Hz
#define FRAMES_PER_SECOND        50
#define EFFECT_PERIOD_CALLBACK   (1000 / FRAMES_PER_SECOND)

// MQTT topics
const char* chipId;
const char* mqttClientID;
const char* mqttTopicPrefix;
const char* mqttLastWillTopic;
const char* mqttColorTopic;
const char* mqttSubscriberTopic;
const char* homeAssistantDiscoveryTopic;
const char* homeAssistantDiscoveryMsg;

// length of the base (#) topic we subscribe to, we use this as an offset
// when we get topics so we can easely compare this with our seperate topics
size_t mqttSubscriberTopicStrLength = 0;

// MQTT buffer
char mqttReceiveBuffer[512];

// Arilux friendly name
const char* friendlyName;

// Counter that keeps counting up, used for filters, effects or other means to keep EFFECT_PERIOD_CALLBACK
// of transitions
volatile uint32_t transitionCounter = 1;

// Keep track when the last time we ran the effect state changes
volatile uint32_t effectPeriodStartMillis = 0;

// When set to true we request the current HSB to be stored in the topic rather than in the state
// THis is needed when the devices changes state from a other source, for example RF or IR control
volatile boolean mqttStoreHSBInTopic = false;

volatile boolean receiveRFCodeFromRemote = true;

// Holds the current working HSB color
HSB workingHsb(0, 0, STARTUP_MIN_BRIGHTNESS, 0, 0);

// Hold´s the color of the last color generated and send to teh Arilux device
HSB currentHsb(0, 0, STARTUP_MIN_BRIGHTNESS, 0, 0);

// Pointers to current effect and filter
std::unique_ptr<Effect> currentEffect(new NoEffect());
std::unique_ptr<Filter> currentFilter(new FadingFilter(workingHsb, FILTER_FADING_ALPHA));

// Filter to control overall brightness
BrightnessFilter brightnessFilter(25);
PowerFilter powerFilter(true);

// Set to true during cold startup
boolean coldStartupActive = true;

// Arilux device interface
Arilux arilux(RED_PIN, GREEN_PIN, BLUE_PIN, WHITE1_PIN, WHITE2_PIN);

#if defined(RF_REMOTE)
RCSwitch rcSwitch = RCSwitch();
#endif
#if defined(TLS)
WiFiClientSecure wifiClient;
#else
WiFiClient wifiClient;
#endif
PubSubClient mqttClient(wifiClient);

// Settings
SettingsDTO settingsDTO;

// Eeprom storage
// wait 500ms after last commit, then commit no more often than every 30s
std::unique_ptr<EEPromStore> eepromStore(nullptr);
Settings eepromSaveHandler(
    500,
    30000,
    []() {eepromStore->save(settingsDTO);EEPROM.commit();},
    []() {return settingsDTO.modified();}
);

// MQTT Storage
// mqtt updates as quickly as possible with a maximum frequence of MQTT_STATE_UPDATE_DELAY
std::unique_ptr<MQTTStore> mqttStore(nullptr);
Settings mqttSaveHandler(
    1,
    MQTT_STATE_UPDATE_DELAY,
    []() {mqttStore->save(settingsDTO);},
    []() {return settingsDTO.modified();}
);

State* BOOTSEQUENCESTART;
//State* SETUPSERIAL;
State* DELAYEDMQTTCONNECTION;
State* TESTMQTTCONNECTION;
State* CONNECTMQTT;
State* PUBLISHONLINE;
State* SUBSCRIBECOMMANDTOPIC;
State* WAITFORCOMMANDCAPTURE;
// Boot sequence setup
State* bootSequenceStates[7];

std::unique_ptr<StateMachine> bootSequence(nullptr);


///////////////////////////////////////////////////////////////////////////
//  SSL/TLS
///////////////////////////////////////////////////////////////////////////
/*
  Function called to verify the fingerprint of the MQTT server certificate
*/
#if defined(TLS)
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


/**
 * Publish a message to mqtt
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

/**
 * Turns off, effectifly setting brightness to 0
 */
HSB getOffState(const HSB& hsb) {
    return hsb.toBuilder().white1(0).white2(0).brightness(0).build();
}

/**
 * Get´s the ON state of a given HSB
 * When hsn is off it will be turned on with the given brightness
 */
HSB getOnState(const HSB& hsb, float brightness) {
    // If the light is already on, we ignore EEPROM settings
    if (hsb.brightness() > 0) {
        return hsb;
    } else {
        return hsb.toBuilder()
               .brightness(brightness)
               .build();
    }
}

///////////////////////////////////////////////////////////////////////////
//  MQTT HANDLING
///////////////////////////////////////////////////////////////////////////

/**
 * Parse a potential color string from a topic´s value
 * String can be in the format of hsb=xHx,xSx,xBx w1=xWx w2=xWx xHx,xSx,xBx,xW1x,xW2x h=xxx s=xxx b=xxx
 * Missing values will be used from the input HSB
 * The below example result all in the same HSB values
 * Example 1: hsb=12,13,14 w1=15 w2=16
 * Example 2: 12,13,14,15,16
 * Example 3: hsb=12,13,14,15,16
 * Example 4: s=13 h=12 b=14 w2=16 w1=15
 * Example 5: hsb=100,101,102 12,13,14,15,16
 */
HSB hsbFromString(const HSB& hsb, const char* data) {
    float h, s, b, w1, w2;
    h = hsb.hue();
    s = hsb.saturation();
    b = hsb.brightness();
    w1 = hsb.white1();
    w2 = hsb.white2();
    OptParser::get(data, [&h, &s, &b, &w1, &w2](OptValue f) {
        if (strstr(f.key(), "hsb") != nullptr || strstr(f.key(), ",") != nullptr) {
            OptParser::get(f.asChar(), ",", [&h, &s, &b, &w1, &w2](OptValue c) {
                switch (c.pos()) {
                    case 0:
                        h = constrain(c.asFloat(), 0.f, 359.99f);
                        break;

                    case 1:
                        s = constrain(c.asFloat(), 0.f, 100.f);
                        break;

                    case 2:
                        b = constrain(c.asFloat(), 0.f, 100.f);
                        break;

                    case 3:
                        w1 = constrain(c.asFloat(), 0.f, 100.f);
                        break;

                    case 4:
                        w2 = constrain(c.asFloat(), 0.f, 100.f);
                        break;
                }
            });
        } else if (strcmp(f.key(), "h") == 0) {
            h = constrain(f.asFloat(), 0.f, 359.99f);
        } else if (strcmp(f.key(), "s") == 0) {
            s = constrain(f.asFloat(), 0.f, 100.f);
        } else if (strcmp(f.key(), "b") == 0) {
            b = constrain(f.asFloat(), 0.f, 100.f);
        } else if (strcmp(f.key(), "w1") == 0) {
            w1 = constrain(f.asFloat(), 0.f, 100.f);
        } else if (strcmp(f.key(), "w2") == 0) {
            w2 = constrain(f.asFloat(), 0.f, 100.f);
        }
    });
    return HSB(h, s, b, w1, w2);
}

/*
   Function called when a MQTT message arrived
   @param p_topic   The topic of the MQTT message
   @param p_payload The payload of the MQTT message
   @param p_length  The length of the payload
*/
void mqttCommandCallback(char* p_topic, byte* p_payload, uint16_t p_length) {
    // Mem copy into own buffer, I am not sure if p_payload is null terminated
    if (p_length >= sizeof(mqttReceiveBuffer)) {
        DEBUG_PRINT(F("MQTT Message to long."));
        return;
    }

    memcpy(mqttReceiveBuffer, p_payload, p_length);
    mqttReceiveBuffer[p_length] = 0;
    DEBUG_PRINT(F("MQTT Message received: "));
    auto topicPos = p_topic + mqttSubscriberTopicStrLength;
    DEBUG_PRINTLN(topicPos);
    DEBUG_PRINT(F("Payload: "));
    DEBUG_PRINTLN(mqttReceiveBuffer);

    // Process topics
    // We use strstr if we want to both handle state and commands
    // we use strcmp if we just want to handle command topics
    if (strstr(topicPos, MQTT_COLOR_TOPIC) != nullptr) {
        workingHsb = hsbFromString(workingHsb, mqttReceiveBuffer);

        bool power;
        bool hasPowerValue = false;
        OptParser::get(mqttReceiveBuffer, [&power, &hasPowerValue](OptValue v) {
            if (strcmp(v.asChar(), STATE_ON) == 0) {
                hasPowerValue = true;
                power = true;

                // When power was off and we turn power on
                // we ensure that we have a brightness > 0
                if (!settingsDTO.power()) {
                    workingHsb = getOnState(workingHsb, settingsDTO.brightness());
                }
            } else if (strcmp(v.asChar(), STATE_OFF) == 0) {
                hasPowerValue = true;
                power = false;
            }
        });

        if (hasPowerValue) {
            settingsDTO.power(power);
        }
    }

    // During startup and we receive color topic, we ensure device goes on
    if (coldStartupActive) {
        workingHsb = getOnState(workingHsb, settingsDTO.brightness());
        settingsDTO.power(true);
        return;
    }

    if (strstr(topicPos, MQTT_FILTER_TOPIC) != nullptr) {
        // Get variables from payload
        const char* name;
        float alpha = FILTER_FADING_ALPHA;
        OptParser::get(mqttReceiveBuffer, [&name, &alpha](OptValue v) {
            // Get variables from filter
            if (strcmp(v.key(), FNAME) == 0) {
                name = v.asChar();
            } else if (strcmp(v.key(), FALPHA) == 0) {
                alpha = constrain(v.asFloat(), 0.001, 1.0);
            }
        });

        // Setup the filter
        if (strcmp(name, FILTER_NONE) == 0) {
            currentFilter.reset(new NoFilter());
        } else if (strcmp(name, FILTER_FADING) == 0) {
            // Note, using currentHSB instead of workingHSB to get directly the correct color
            // visible
            currentFilter.reset(new FadingFilter(currentHsb, alpha));
        }
    } else if (strstr(topicPos, MQTT_REMOTE_TOPIC) != nullptr) {
        const uint32_t base = atol(mqttReceiveBuffer);

        if (base > 0) {
            settingsDTO.remoteBase(base);
        }
    }

    // We absolutly never want to process any messages below here during bootup
    // as they might have odd and unwanted side effects
    if (strstr(topicPos, MQTT_EFFECT_TOPIC) != 0) {
        // Get variables from payload
        const char* name;
        int16_t pulse = -1;
        int16_t period = -1;
        int32_t duration = -1;
        float m_speed = -1.f;
        const HSB hsb = hsbFromString(workingHsb, mqttReceiveBuffer);
        OptParser::get(mqttReceiveBuffer, [&name, &period, &pulse, &duration](OptValue v) {
            // Get variables from filter
            if (strcmp(v.key(), ENAME) == 0) {
                name = v.asChar();
            }

            if (strcmp(v.key(), FILTER_PERIOD) == 0) {
                period = v.asInt();
            }

            if (strcmp(v.key(), FILTER_PULSE) == 0) {
                pulse = v.asInt();
            }

            if (strcmp(v.key(), FILTER_DURATION) == 0) {
                duration = v.asLong();
            }
        });

        if (strcmp(name, EFFECT_NONE) == 0) {
            currentEffect.reset(new NoEffect());
        } else if (strcmp(name, EFFECT_RAINBOW) == 0) {
            if (duration>0) {
                currentEffect.reset(new RainbowEffect(currentHsb.hue(), duration, millis()));
            } else {
                currentEffect.reset(new RainbowEffect(millis()));
            }
        } else if (strcmp(name, EFFECT_FLASH) == 0) {
            period = period < 2 ? FRAMES_PER_SECOND : period;
            pulse = pulse < period && pulse > 0 ? pulse : period >> 1;

            if (hsb == workingHsb) {
                currentEffect.reset(new FlashEffect(currentHsb.toBuilder().brightness(0).build(), transitionCounter, period, pulse));
            } else {
                currentEffect.reset(new FlashEffect(hsb, transitionCounter, period, pulse));
            }
        } else if (strcmp(name, EFFECT_FADE) == 0) {
            if (duration > 0) {
                currentEffect.reset(new TransitionEffect(hsb, millis(), duration));
            }
        }
    } else if (strcmp(topicPos, MQTT_RESTART_TOPIC) == 0) {
        if (strcmp(mqttReceiveBuffer, "1") == 0) {
            ESP.restart();
        }
    } else if (strcmp(topicPos, MQTT_STORE_TOPIC) == 0) {
        if (strcmp(mqttReceiveBuffer, "1") == 0) {
            eepromStore->save(settingsDTO);
            EEPROM.commit();
        }
    }
}


///////////////////////////////////////////////////////////////////////////
//  WiFi
///////////////////////////////////////////////////////////////////////////

void setupWiFi() {
    WiFi.hostname(mqttClientID);

    delay(10);
    Serial.print(F("INFO: Connecting to: "));
    Serial.println(WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println();
    Serial.println(F("INFO: WiFi connected"));
    Serial.print(F("INFO: IP address: "));
    Serial.println(WiFi.localIP());

    randomSeed(micros());
    MDNS.begin(mqttClientID);
}

///////////////////////////////////////////////////////////////////////////
//  Function called to handle received RF codes from the remote
///////////////////////////////////////////////////////////////////////////
#if defined(RF_REMOTE)
void handleRFRemote(void) {
    if (rcSwitch.available()) {
        // Before boot is finnished and the remote
        // is pressed we will store the remote controle base code
        if (receiveRFCodeFromRemote) {
            settingsDTO.remoteBase(rcSwitch.getReceivedValue() & 0xFFFF00);
        }

        const uint32_t value = rcSwitch.getReceivedValue() - settingsDTO.remoteBase();
        DEBUG_PRINT(F("Key Received : "));
        DEBUG_PRINT(value & 0xFFFF00);
        DEBUG_PRINT(F(" / key:"));
        DEBUG_PRINTLN(value);

        switch (value) {
            case ARILUX_REMOTE_KEY_BRIGHT_PLUS:
                brightnessFilter.increase();
                break;

            case ARILUX_REMOTE_KEY_BRIGHT_MINUS:
                brightnessFilter.decrease();
                break;

            case ARILUX_REMOTE_KEY_OFF:
                settingsDTO.power(false);
                break;

            case ARILUX_REMOTE_KEY_ON:
                settingsDTO.power(true);
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
                workingHsb = workingHsb.toBuilder().hue(fmod(workingHsb.hue() + 5, 360.f)).build();
                break;

            case ARILUX_REMOTE_KEY_SPEED_MINUS:
                // TODO: Implement some incremantal speedup filter
                workingHsb = workingHsb.toBuilder().hue(fmod(workingHsb.hue() - 5, 360.f)).build();
                break;

            case ARILUX_REMOTE_KEY_MODE_PLUS:
                workingHsb = workingHsb.toBuilder().saturation(constrain(workingHsb.saturation() + 5, 0.f, 100.f)).build();
                break;

            case ARILUX_REMOTE_KEY_MODE_MINUS:
                workingHsb = workingHsb.toBuilder().saturation(constrain(workingHsb.saturation() - 5, 0.f, 100.f)).build();
                break;

            default:
                DEBUG_PRINTLN(F("ERROR: RF code not defined"));
                break;
        }

        // Store in mqtt some time after user presses the remote
        if (value & 0xFFFF00) {
            mqttStoreHSBInTopic = true;
        }

        rcSwitch.resetAvailable();
    }
}
#endif

/**
 * Create a new char* from input
 */
char* makeString(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 255, format, args);
    va_end(args);
    return strdup(buffer);
}

void generateMqttTopicStrings() {
     // chipId : 00FF1234
    chipId = makeString("%08X", ESP.getChipId());
    // ARILUX00FF1234
    mqttClientID = makeString(HOSTNAME_TEMPLATE, chipId);
    // RGBW/00FF1234
    mqttTopicPrefix = makeString(MQTT_TOPIC_PREFIX_TEMPLATE, MQTT_PREFIX, chipId);
    // RGBW/00FF1234/lastwill
    mqttLastWillTopic = makeString(MQTT_LASTWILL_TOPIC_TEMPLATE, mqttTopicPrefix);
    //  RGBW/00FF1234/+
    mqttSubscriberTopic = makeString(MQTT_SUBSCRIBER_TOPIC_TEMPLATE, mqttTopicPrefix);
    // Calculate length of the subcriber topic
    mqttSubscriberTopicStrLength = strlen(mqttSubscriberTopic) - 2;
    // friendlyName : Arilux LC11 RGBW LED Controller 00FF1234
    friendlyName = makeString("Arilux %s %s LED Controller %s", DEVICE_MODEL, MQTT_PREFIX, chipId);
    homeAssistantDiscoveryTopic = makeString(HOME_ASSISTANCE_MQTT_DISCOVERY_TOPIC_TEMPLATE,
                                             HOME_ASSISTANT_MQTT_DISCOVERY_PREFIX, DEVICE_MODEL, MQTT_PREFIX, chipId);
    // Hass Message for auto discovery
    homeAssistantDiscoveryMsg = makeString(MQTT_HASS_DISCOVERY_TEMPLATE,
                                           friendlyName,
                                           mqttTopicPrefix,
                                           MQTT_COLOR_TOPIC,
                                           mqttTopicPrefix,
                                           MQTT_COLOR_TOPIC,
                                           mqttLastWillTopic);
}


/**
 * Trun on the lights after reboot
 * requires eeprom store to be initialised
 */
void initialiseAfterStartup() {
    // set color from EEPROM and enable PWM on lights
    // to ensure we turn on light as quickly as possible
    settingsDTO = eepromStore->get();
    if (settingsDTO.brightness() < STARTUP_MIN_BRIGHTNESS) {
        settingsDTO.brightness(STARTUP_MIN_BRIGHTNESS);
    }
    // Enforce on in power filter and settings
    settingsDTO.power(true);
    settingsDTO.reset();
    powerFilter.power(true);
    workingHsb = getOnState(settingsDTO.hsb(), settingsDTO.brightness());
    currentHsb = workingHsb;
}


void updateOutput(const HSB& hsb) {
    float colors[3];
    hsb.constantRGB(colors);
    arilux.setAll(colors[0], colors[1], colors[2], hsb.cwhite1(), hsb.cwhite2());
}


/**
 * Start OTA
 */
void startOTA() {
    // Start OTA
    ArduinoOTA.setHostname(mqttClientID);

    ArduinoOTA.onStart([]() {
        // Disable outputs as this might interfere with OTA
        arilux.setAll(0,0,0,0,0);
        DEBUG_PRINTLN(F("OTA Beginning"));
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
    ArduinoOTA.handle();
}

///////////////////////////////////////////////////////////////////////////
//  SETUP() AND LOOP()
///////////////////////////////////////////////////////////////////////////

void setup() {

    BOOTSEQUENCESTART = new State([](){
        return TESTMQTTCONNECTION;
    });

    DELAYEDMQTTCONNECTION = new StateTimed(1500, []() {
        return TESTMQTTCONNECTION;
    });

    TESTMQTTCONNECTION = new State([]() {
        receiveRFCodeFromRemote = false;
        if (mqttClient.connected())  {
            if (WiFi.status() != WL_CONNECTED) {
                mqttClient.disconnect();
            }
            return DELAYEDMQTTCONNECTION;
        }
        return CONNECTMQTT;
    });

    CONNECTMQTT = new State([](){
        if (mqttClient.connect(mqttClientID, MQTT_USER, MQTT_PASS, mqttLastWillTopic, 0, 1, MQTT_LASTWILL_OFFLINE)) {
            return PUBLISHONLINE;
        }
        DEBUG_PRINTLN(F("ERROR: The connection to the MQTT broker failed"));
        DEBUG_PRINT(F("Username: "));
        DEBUG_PRINTLN(MQTT_USER);
        DEBUG_PRINT(F("Broker: "));
        DEBUG_PRINTLN(MQTT_SERVER);
        return DELAYEDMQTTCONNECTION;
    });

    PUBLISHONLINE = new State([](){
        publishToMQTT(mqttLastWillTopic, MQTT_LASTWILL_ONLINE);
        #if defined(HOME_ASSISTANT_MQTT_DISCOVERY)
            DEBUG_PRINTLN(homeAssistantDiscoveryMsg);
            publishToMQTT(homeAssistantDiscoveryTopic, homeAssistantDiscoveryMsg);
        #endif
        return SUBSCRIBECOMMANDTOPIC;
    });

    SUBSCRIBECOMMANDTOPIC = new State([](){
        if (mqttClient.subscribe(mqttSubscriberTopic, 0)) {
            DEBUG_PRINT(F("INFO: Connected to topic : "));
            return WAITFORCOMMANDCAPTURE;
        }
        DEBUG_PRINT(F("ERROR: Failed to connect to topic : "));
        mqttClient.disconnect();
        return DELAYEDMQTTCONNECTION;
    });

    WAITFORCOMMANDCAPTURE = new StateTimed(3000, [](){
        coldStartupActive = false;
        return TESTMQTTCONNECTION;
    });

    bootSequenceStates[0] = BOOTSEQUENCESTART;
    bootSequenceStates[1] = DELAYEDMQTTCONNECTION;
    bootSequenceStates[2] = TESTMQTTCONNECTION;
    bootSequenceStates[3] = CONNECTMQTT;
    bootSequenceStates[4] = PUBLISHONLINE;
    bootSequenceStates[5] = SUBSCRIBECOMMANDTOPIC;
    bootSequenceStates[6] = WAITFORCOMMANDCAPTURE;
    bootSequence.reset(new StateMachine(7, bootSequenceStates));

    // setup Strings
    generateMqttTopicStrings();

    // Enable serial port
    Serial.begin(9600);
    delay(50);
    Serial.print("Starting");
    Serial.print("Hostname:");
    Serial.println(mqttClientID);

    eepromStore.reset(new EEPromStore(0));
    mqttStore.reset(new MQTTStore(
                        mqttTopicPrefix,
                        MQTT_COLOR_STATE_TOPIC,
                        MQTT_REMOTE_STATE_TOPIC,
                        MQTT_STATE_STATE_TOPIC,
                        mqttClient,
                        STATE_IN_COLOR_TOPIC
                    ));

    EEPROM.begin(EEPromStore::requestedSize());
    initialiseAfterStartup();

    startOTA();

    #if !defined(PAUSE_FOR_OTA)
        arilux.init();
        updateOutput(currentHsb);
    #endif

    // Setup Wi-Fi
    setupWiFi();

    #if defined(TLS)
        // Check the fingerprint of CloudMQTT's SSL cert
        verifyFingerprint();
    #endif

    #if defined(ARILUX_DEBUG_TELNET)
        // Start the Telnet server
        startTelnet();
        handleTelnet();
    #endif

    #if defined(PAUSE_FOR_OTA)
        uint16_t i = 0;
        do {
            yield();
            ArduinoOTA.handle();
            yield();
            delay(10);
            i++;
        } while (i < 750);
        arilux.init();
        updateOutput(currentHsb);
    #endif



    #if defined(IR_REMOTE)
        // Start the IR receiver
        irRecv.enableIRIn();
    #endif

    #if defined(RF_REMOTE)
           // Start the RF receiver
        rcSwitch.enableReceive(RF_PIN);
    #endif

    // Setup mqtt
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCommandCallback);

    // Start boot sequence
    bootSequence->start();

    // Avoid running towards millis() when loop starts
    effectPeriodStartMillis = millis();

}

void handleEffects() {
    const uint32_t currentMillies = millis();
    workingHsb = brightnessFilter.handleFilter(transitionCounter, currentMillies, workingHsb);

    // Last filters are for modification of the HSB values before we send it to the DEVICE_MODEL
    // and we don´t want to store these
    settingsDTO.hsb(workingHsb);

    // Store brightness only if it´s >= STARTUP_MIN_BRIGHTNESS
    if (workingHsb.brightness() >= STARTUP_MIN_BRIGHTNESS) {
        settingsDTO.brightness(workingHsb.brightness());
    }

    currentHsb = currentEffect->handleEffect(transitionCounter, currentMillies, workingHsb);
    currentHsb = powerFilter.handleFilter(transitionCounter, currentMillies, currentHsb);
    currentHsb = currentFilter->handleFilter(transitionCounter, currentMillies, currentHsb);
    updateOutput(currentHsb);
}


#define NUMBER_OF_SLOTS 15
void loop() {

    const uint32_t currentMillis = millis();

    if (currentMillis - effectPeriodStartMillis >= EFFECT_PERIOD_CALLBACK) {
        effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
        transitionCounter++;
        handleEffects();

        uint8_t slot = 0;

        if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            ArduinoOTA.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // This might 'overshoot' the effect by a littie due to the number of slots, do we accept that?
            if (currentEffect->isCompleted(transitionCounter, currentMillis, workingHsb)) {
                workingHsb = currentEffect->finalState(transitionCounter, currentMillis, workingHsb);
                currentEffect.reset(new NoEffect());
            }
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            mqttClient.loop();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            eepromSaveHandler.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            mqttSaveHandler.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            settingsDTO.reset();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            powerFilter.power(settingsDTO.power());
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            bootSequence->handle();
        }
#if defined(ARILUX_DEBUG_TELNET)
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle Telnet connection for debugging
            handleTelnet();
        }
#endif
#if defined(IR_REMOTE)
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle received IR codes from the remote
            handleIRRemote();
        }

#endif
#if defined(RF_REMOTE)
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle received RF codes from the remote
            handleRFRemote();
        }
#endif

        const uint32_t thisDuration = (millis() - currentMillis);

        if (thisDuration > EFFECT_PERIOD_CALLBACK) {
            DEBUG_PRINT(F("Spiked : "));
            DEBUG_PRINT(slot);
            DEBUG_PRINT(F(" "));
            DEBUG_PRINTLN(thisDuration);
        }

    }
}
