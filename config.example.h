// Uncomment the block for your model
// Begin LC01 block, uncomment starting below this line
// #define RGB
// #define DEVICE_MODEL                           "LC01"
// End LC01 block, stop uncommenting above this line

// Begin LC02 block, uncomment starting below this line
// #define RGBW
// #define DEVICE_MODEL                           "LC02"
// End LC02 block, stop uncommenting above this line

// Begin LC03 block, uncomment starting below this line
// #define RGB
// #define IR_REMOTE
// #define DEVICE_MODEL                           "LC03"
// End LC03 block, stop uncommenting above this line

// Begin LC04 block, uncomment starting below this line
// #define RGBW
// #define IR_REMOTE
// #define DEVICE_MODEL                           "LC04"
// End LC04 block, stop uncommenting above this line

// Begin LC08 block, uncomment starting below this line
// #define RGBWW
// #define DEVICE_MODEL                           "LC08"
// End LC08 block, stop uncommenting above this line

// Begin LC09 block, uncomment starting below this line
// #define RGB
// #define RF_REMOTE
// #define DEVICE_MODEL                           "LC09"
// End LC09 block, stop uncommenting above this line

// Begin LC10 block, uncomment starting below this line
// #define RGBW
// #define RF_REMOTE
// #define DEVICE_MODEL                           "LC10"
// End LC10 block, stop uncommenting above this line

// Begin LC11 block, uncomment starting below this line
// #define RGBWW
// #define RF_REMOTE
// #define DEVICE_MODEL                           "LC11"
// End LC11 block, stop uncommenting above this line

// If you can't find your model above, you can use the defines below instead.
// #define RGB
// #define RGBW
// #define RGBWW
// #define IR_REMOTE
// #define RF_REMOTE
// #define DEVICE_MODEL                           "LC0X"

// Wi-Fi
#define WIFI_SSID                              ""
#define WIFI_PASSWORD                          ""

// MQTT server settings, leave username/password blank if no authentication required
#define MQTT_SERVER                            "m21.cloudmqtt.com"
#define MQTT_PORT                              12345
#define MQTT_USER                              "user"
#define MQTT_PASS                              "pass"

// MQTT payloads
#define MQTT_STATE_ON_PAYLOAD                  "ON"
#define MQTT_STATE_OFF_PAYLOAD                 "OFF"
#define MQTT_STATE_ON_WHITE_PAYLOAD            "ON_WHITE"

// MQTT topics
// Leaving this as default will prefix all MQTT topics with RGB(W/WW)/<chipid>
#define MQTT_TOPIC_PREFIX_TEMPLATE             "%s/%s"

// Leave %s at the front if you wish to use the MQTT Topic Prefix described above
#define MQTT_STATE_STATE_TOPIC_TEMPLATE        "%s/state/state"
#define MQTT_STATE_COMMAND_TOPIC_TEMPLATE      "%s/state/set"
#define MQTT_BRIGHTNESS_STATE_TOPIC_TEMPLATE   "%s/brightness/state"
#define MQTT_BRIGHTNESS_COMMAND_TOPIC_TEMPLATE "%s/brightness/set"
#define MQTT_COLOR_STATE_TOPIC_TEMPLATE        "%s/color/state"
#define MQTT_COLOR_COMMAND_TOPIC_TEMPLATE      "%s/color/set"
#define MQTT_STATUS_TOPIC_TEMPLATE             "%s/status"

#if defined(RGBW) || defined (RGBWW)
#define MQTT_WHITE_STATE_TOPIC_TEMPLATE        "%s/white/state"
#define MQTT_WHITE_COMMAND_TOPIC_TEMPLATE      "%s/white/set"
#endif

// Enable Home Assistant MQTT discovery support
#define HOME_ASSISTANT_MQTT_DISCOVERY
#define HOME_ASSISTANT_MQTT_DISCOVERY_PREFIX   "homeassistant"

// Base hostname, used for the MQTT Client ID and OTA hostname.
// If you leave %s in place it will be replaced with the Chip ID.
#define HOST                                   "ARILUX%s"

// Enable console output via telnet
// #define DEBUG_TELNET

// TLS support, make sure to edit the fingerprint and the MQTT broker IP address if
// you are not using CloudMQTT
// #define TLS
// #define TLS_FINGERPRINT                     "A5 02 FF 13 99 9F 8B 39 8E F1 83 4F 11 23 65 0B 32 36 FC 07"
