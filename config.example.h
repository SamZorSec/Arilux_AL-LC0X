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
#define WIFI_SSID                             ""
#define WIFI_PASSWORD                         ""

// MQTT. Leave username/password blank if none
#define MQTT_SERVER                           "m21.cloudmqtt.com"
#define MQTT_PORT                             12345
#define MQTT_USER                             "user"
#define MQTT_PASS                             "pass"

// MQTT payloads
#define ARILUX_MQTT_STATE_ON_PAYLOAD          "ON"
#define ARILUX_MQTT_STATE_OFF_PAYLOAD         "OFF"
#define ARILUX_MQTT_STATE_WHITE_ON_PAYLOAD    "WHITE_ON"

// Base hostname, used for the MQTT Client ID and OTA hostname.
// You must leave %s in place. It will be replaced with the Chip ID.
#define HOST                                  "ARILUX%s"

#ifdef TLS
// SHA1 fingerprint of the certificate
// openssl x509 -fingerprint -in <certificate>.crt
// Default value is the CloudMQTT certificate fingerprint
const char* fingerprint = "A5 02 FF 13 99 9F 8B 39 8E F1 83 4F 11 23 65 0B 32 36 FC 07";
#endif
