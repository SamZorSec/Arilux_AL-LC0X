// Uncomment the #defines that make sense for your model.
// Arilux LC08 is RGBWW
// #define RGBWW
// Arilux LC02 and LC04 are RGBW
#define RGBW
// Arilux LC03 and LC09 is RGB only
// #define RGB

// All devices except LC09 support IR
#define IR_REMOTE

// Arilux LC09 is only confirmed device that has RF remote support
// #define RF_REMOTE

// TLS support, make sure to edit the fingerprint and the MQTT broker IP address if
// you are not using CloudMQTT
//#define TLS

// Enable console output via telnet
#define DEBUG_TELNET

// Wifi
#define WIFI_SSID                ""
#define WIFI_PASSWORD            ""

// MQTT. Leave username/password blank if none
#define MQTT_SERVER              "m21.cloudmqtt.com"
#define MQTT_PORT                12345
#define MQTT_USER                "user"
#define MQTT_PASS                "pass"

// Enable Home Assistant MQTT discovery support
#define HOME_ASSISTANT_MQTT_DISCOVERY
#define HOME_ASSISTANT_MQTT_DISCOVERY_PREFIX    "homeassistant"

#ifdef TLS
  // SHA1 fingerprint of the certificate
  // openssl x509 -fingerprint -in <certificate>.crt
  const char* fingerprint = "A5 02 FF 13 99 9F 8B 39 8E F1 83 4F 11 23 65 0B 32 36 FC 07";
#endif
