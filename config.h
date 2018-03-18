#include "setup.h"

// #define DEVICE_MODEL                           "LC01"
// #define DEVICE_MODEL                           "LC02"
// #define DEVICE_MODEL                           "LC03"
// #define DEVICE_MODEL                           "LC04"
// #define DEVICE_MODEL                           "LC08"
// #define DEVICE_MODEL                           "LC09"
// #define DEVICE_MODEL                           "LC10"
// #define DEVICE_MODEL                           "LC11"

#ifndef DEVICE_MODEL
#define DEVICE_MODEL                           "LC01"
#endif

#ifndef RGB || RGBW || RGBWW
#define RGB
#endif

// #define IR_REMOTE
// #define RF_REMOTE

// Wi-Fi
#ifndef WIFI_SSID
#define WIFI_SSID                              ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD                          ""
#endif

// TLS support, make sure to edit the fingerprint and the MQTT broker IP address if
// you are not using CloudMQTT
// #define TLS
// #define TLS_FINGERPRINT                     "A5 02 FF 13 99 9F 8B 39 8E F1 83 4F 11 23 65 0B 32 36 FC 07"

// MQTT server settings
#ifndef MQTT_SERVER
#define MQTT_SERVER                            "127.0.0.1"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT                              1883
#endif

#ifndef MQTT_USER
#define MQTT_USER                              ""
#endif

#ifndef MQTT_PASS
#define MQTT_PASS                              ""
#endif

// How often we are updating the mqtt state in ms
#ifndef MQTT_UPDATE_DELAY
#define MQTT_UPDATE_DELAY                       125
#endif

// MQTT topics : RGBW/00FF1234
#define MQTT_TOPIC_PREFIX_TEMPLATE             "%s/%s"

// Last Will and Testament topic : RGBW/00FF1234/status
#define MQTT_LASTWILL_TOPIC_TEMPLATE           "%s/status"

// State template : RGBW/00FF1234/json/state
#define MQTT_STATE_TOPIC_TEMPLATE              "%s/json/state"

// Topic Template : RGBW/00FF1234/json/set
#define MQTT_COMMAND_TOPIC_TEMPLATE             "%s/json/set"

// Enable Home Assistant MQTT discovery support. Requires ArduinoJSON library to be installed.
#define HOME_ASSISTANT_MQTT_DISCOVERY
#define HOME_ASSISTANT_MQTT_DISCOVERY_PREFIX   "homeassistant"

// Base hostname, used for the MQTT Client ID and OTA hostname
#ifndef HOSTNAME_TEMPLATE                      
#define HOSTNAME_TEMPLATE                       "ARILUX%s"
#endif

// Enable console output via telnet OR SERIAL
// #define DEBUG_TELNET
// #define DEBUG_SERIAL

#define FILTER                      "filter"
#define FNAME                       "name"
#define FILTER_NONE                 "none"
#define FILTER_FADING               "fading"
#define FALPHA                      "alpha"
#define FILTER_FADING_ALPHA         0.04

#define EFFECT                  "effect"
#define EFFECT_NONE             "none"
#define EFFECT_FLASH            "flash"
#define EFFECT_RAINBOW          "rainbow"
#define EFFECT_FADE             "fade"
#define TWIDTH                      "width"
#define TNAME                       "name"
#define TDURATION                   "duration"

#define STATE                       "state"
#define SON                         "ON"
#define SOFF                        "OFF"
