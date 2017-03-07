// Uncomment the #defines that make sense for your model
// LC01, LC03 and LC09 are RGB devices
#define RGB
// LC02, LC04 and LC10 are RGBW devices
// #define RGBW
// LC08 and LC11 are RGBWW devices
// #define RGBWW

// LC03 and LC04 support IR control
#define IR_REMOTE

// LC09, LC10 and LC11 support RF control
// #define RF_REMOTE

// TLS support, make sure to edit the fingerprint and the MQTT broker IP address if
// you are not using CloudMQTT
// #define TLS

// Enable console output via telnet
// #define DEBUG_TELNET

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

// Base hostname, used for the MQTT Client ID and OTA hostname.
// If you leave %s in place it will be replaced with the Chip ID.
#define HOST                                   "ARILUX%s"

#ifdef TLS
// SHA1 fingerprint of the certificate
// openssl x509 -fingerprint -in <certificate>.crt
const char* fingerprint = "A5 02 FF 13 99 9F 8B 39 8E F1 83 4F 11 23 65 0B 32 36 FC 07";
#endif
