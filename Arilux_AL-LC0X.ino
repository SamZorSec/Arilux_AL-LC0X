/*
    Arilux AL-LC03
    Alternative firmware

    Work in progress...
*/

#define IR_REMOTE
#define DEBUG_TELNET

#include <ESP8266WiFi.h>      // https://github.com/esp8266/Arduino
#include <PubSubClient.h>     // https://github.com/knolleary/pubsubclient/releases/tag/v2.6
#ifdef IR_REMOTE
#include <IRremoteESP8266.h>  // https://github.com/markszabo/IRremoteESP8266
#endif

#include <ArduinoOTA.h>
#include "Arilux.h"

// in a terminal: telnet arilux.local
#ifdef DEBUG_TELNET
  WiFiServer  telnetServer(23);
  WiFiClient  telnetClient;
#endif

// Macros for debugging
#ifdef DEBUG_TELNET
  #define     DEBUG_PRINT(x)    telnetClient.print(x)
  #define     DEBUG_PRINTLN(x)  telnetClient.println(x)
#else
  #define     DEBUG_PRINT(x)
  #define     DEBUG_PRINTLN(x)
#endif

// Wi-Fi
const char*   WIFI_SSID     = "[REDACTED]";
const char*   WIFI_PASSWORD = "[REDACTED]";

// MQTT topics
const char*   ARILUX_MQTT_STATE_STATE_TOPIC         = "arilux/state/state";
const char*   ARILUX_MQTT_STATE_COMMAND_TOPIC       = "arilux/state/set";
const char*   ARILUX_MQTT_BRIGHTNESS_STATE_TOPIC    = "arilux/brightness/state";
const char*   ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC  = "arilux/brightness/set";
const char*   ARILUX_MQTT_COLOR_STATE_TOPIC         = "arilux/color/state";
const char*   ARILUX_MQTT_COLOR_COMMAND_TOPIC       = "arilux/color/set";

// MQTT payloads
const char*   ARILUX_MQTT_STATE_ON_PAYLOAD          = "ON";
const char*   ARILUX_MQTT_STATE_OFF_PAYLOAD         = "OFF";

// MQTT buffer
char msgBuffer[12];

// MQTT
const char*   MQTT_CLIENT_ID                        = "arilux";
const char*   MQTT_SERVER_IP                        = "[REDACTED]";
const int     MQTT_SERVER_PORT                      = 1883;
const char*   MQTT_USER                             = "[REDACTED]";
const char*   MQTT_PASSWORD                         = "[REDACTED]";

volatile uint8_t cmd = ARILUX_CMD_NOT_DEFINED;

Arilux        arilux;
#ifdef IR_REMOTE
  IRrecv      irRecv(ARILUX_IR_PIN);
#endif
WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);

/*
   Function called when a MQTT message arrived
   @param p_topic   The topic of the MQTT message
   @param p_payload The payload of the MQTT message
   @param p_length  The length of the payload
*/
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }

  // handle the MQTT topic of the received message
  if (String(ARILUX_MQTT_STATE_COMMAND_TOPIC).equals(p_topic)) {
    if (payload.equals(String(ARILUX_MQTT_STATE_ON_PAYLOAD))) {
      if (arilux.turnOn())
        cmd = ARILUX_CMD_STATE_CHANGED;
    } else if (payload.equals(String(ARILUX_MQTT_STATE_OFF_PAYLOAD))) {
      if (arilux.turnOff())
        cmd = ARILUX_CMD_STATE_CHANGED;
    }
  } else if (String(ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC).equals(p_topic)) {
    if (arilux.setBrightness(payload.toInt()))
      cmd = ARILUX_CMD_BRIGHTNESS_CHANGED;
  } else if (String(ARILUX_MQTT_COLOR_COMMAND_TOPIC).equals(p_topic)) {
    // get the position of the first and second commas
    uint8_t firstIndex = payload.indexOf(',');
    uint8_t lastIndex = payload.lastIndexOf(',');

    if (arilux.setColor(payload.substring(0, firstIndex).toInt(), payload.substring(firstIndex + 1, lastIndex).toInt(), payload.substring(lastIndex + 1).toInt()))
      cmd = ARILUX_CMD_COLOR_CHANGED;
  }
}

/*
  Function called to connect/reconnect to the MQTT broker
*/
void connectMQTT(void) {
  while (!mqttClient.connected()) {
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      DEBUG_PRINTLN(F("INFO: The client is successfully connected to the MQTT broker"));
    } else {
      DEBUG_PRINTLN(F("ERROR: The connection to the MQTT broker failed"));
      DEBUG_PRINT(F("Username: "));
      DEBUG_PRINTLN(MQTT_USER);
      DEBUG_PRINT(F("Password: "));
      DEBUG_PRINTLN(MQTT_PASSWORD);
      DEBUG_PRINT(F("Broker: "));
      DEBUG_PRINTLN(MQTT_SERVER_IP);

      delay(1000);
      ESP.reset();
    }
  }

  if (mqttClient.subscribe(ARILUX_MQTT_STATE_COMMAND_TOPIC)) {
    DEBUG_PRINT(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
    DEBUG_PRINTLN(ARILUX_MQTT_STATE_COMMAND_TOPIC);
  } else {
    DEBUG_PRINT(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
    DEBUG_PRINTLN(ARILUX_MQTT_STATE_COMMAND_TOPIC);
  }
  if (mqttClient.subscribe(ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC)) {
    DEBUG_PRINT(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
    DEBUG_PRINTLN(ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC);
  } else {
    DEBUG_PRINT(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
    DEBUG_PRINTLN(ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC);
  }
  if (mqttClient.subscribe(ARILUX_MQTT_COLOR_COMMAND_TOPIC)) {
    DEBUG_PRINT(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
    DEBUG_PRINTLN(ARILUX_MQTT_COLOR_COMMAND_TOPIC);
  } else {
    DEBUG_PRINT(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
    DEBUG_PRINTLN(ARILUX_MQTT_COLOR_COMMAND_TOPIC);
  }
}

/*
   Function called to handle Telnet clients
   https://www.youtube.com/watch?v=j9yW10OcahI
*/
#ifdef DEBUG_TELNET
void handleTelnet(void) {
  if (telnetServer.hasClient()) {
    if (!telnetClient || !telnetClient.connected()) {
      if (telnetClient) {
        telnetClient.stop();
      }
      telnetClient = telnetServer.available();
    } else {
      telnetServer.available().stop();
    }
  }
}
#endif

/*
   Function called to handle received IR codes from the remote
*/
#ifdef IR_REMOTE
void handleIRRemote(void) {
  decode_results  results;

  if (irRecv.decode(&results)) {
    switch (results.value) {
      case ARILUX_IR_CODE_KEY_UP:
        if (arilux.increaseBrightness())
          cmd = ARILUX_CMD_BRIGHTNESS_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_DOWN:
        if (arilux.decreaseBrightness())
          cmd = ARILUX_CMD_BRIGHTNESS_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_OFF:
        if (arilux.turnOff())
          cmd = ARILUX_CMD_STATE_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_ON:
        if (arilux.turnOn())
          cmd = ARILUX_CMD_STATE_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_R:
        if (arilux.setColor(255, 0, 0))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_G:
        if (arilux.setColor(0, 255, 0))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_B:
        if (arilux.setColor(0, 0, 255))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_W:
        if (arilux.setColor(255, 255, 255))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_1:
        if (arilux.setColor(255, 51, 51))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_2:
        if (arilux.setColor(102, 204, 0))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_3:
        if (arilux.setColor(0, 102, 204))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_FLASH:
        // TODO
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_FLASH"));
        break;
      case ARILUX_IR_CODE_KEY_4:
        if (arilux.setColor(255, 102, 102))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_5:
        if (arilux.setColor(0, 255, 255))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_6:
        if (arilux.setColor(153, 0, 153))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_STROBE:
        // TODO
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_STROBE"));
        break;
      case ARILUX_IR_CODE_KEY_7:
        if (arilux.setColor(255, 255, 102))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_8:
        if (arilux.setColor(51, 153, 255))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_9:
        if (arilux.setColor(255, 0, 255))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_FADE:
        // TODO
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_FADE"));
        break;
      case ARILUX_IR_CODE_KEY_10:
        if (arilux.setColor(255, 255, 0))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_11:
        if (arilux.setColor(0, 128, 255))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_12:
        if (arilux.setColor(255, 102, 178))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_IR_CODE_KEY_SMOOTH:
        // TODO
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_SMOOTH"));
        break;
      default:
        DEBUG_PRINTLN(F("ERROR: IR code not defined"));
        break;
    }
    irRecv.resume();
  }
}
#endif

/*
   Function called to handle commands due to changes
*/
void handleCMD(void) {
  switch (cmd) {
    case ARILUX_CMD_NOT_DEFINED:
      break;
    case ARILUX_CMD_STATE_CHANGED:
      if (arilux.getState()) {
        if (mqttClient.publish(ARILUX_MQTT_STATE_STATE_TOPIC, ARILUX_MQTT_STATE_ON_PAYLOAD, true)) {
          DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
          DEBUG_PRINT(ARILUX_MQTT_STATE_STATE_TOPIC);
          DEBUG_PRINT(F(". Payload: "));
          DEBUG_PRINTLN(ARILUX_MQTT_STATE_ON_PAYLOAD);
        } else {
          DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
        }
      } else {
        if (mqttClient.publish(ARILUX_MQTT_STATE_STATE_TOPIC, ARILUX_MQTT_STATE_OFF_PAYLOAD, true)) {
          DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
          DEBUG_PRINT(ARILUX_MQTT_STATE_STATE_TOPIC);
          DEBUG_PRINT(F(". Payload: "));
          DEBUG_PRINTLN(ARILUX_MQTT_STATE_OFF_PAYLOAD);
        } else {
          DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
        }
      }
      cmd = ARILUX_CMD_NOT_DEFINED;
      break;
    case ARILUX_CMD_BRIGHTNESS_CHANGED:
      snprintf(msgBuffer, sizeof(msgBuffer), "%d", arilux.getBrightness());
      if (mqttClient.publish(ARILUX_MQTT_BRIGHTNESS_STATE_TOPIC, msgBuffer, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
        DEBUG_PRINT(ARILUX_MQTT_BRIGHTNESS_STATE_TOPIC);
        DEBUG_PRINT(F(". Payload: "));
        DEBUG_PRINTLN(msgBuffer);
      } else {
        DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
      }
      cmd = ARILUX_CMD_NOT_DEFINED;
      break;
    case ARILUX_CMD_COLOR_CHANGED:
      snprintf(msgBuffer, sizeof(msgBuffer), "%d,%d,%d", arilux.getRedValue(), arilux.getGreenValue(), arilux.getBlueValue());
      if (mqttClient.publish(ARILUX_MQTT_COLOR_STATE_TOPIC, msgBuffer, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
        DEBUG_PRINT(ARILUX_MQTT_COLOR_STATE_TOPIC);
        DEBUG_PRINT(F(". Payload: "));
        DEBUG_PRINTLN(msgBuffer);
      } else {
        DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
      }
      cmd = ARILUX_CMD_NOT_DEFINED;
      break;
    default:
      break;
  }
}

/*
   Function called to setup the connection to the WiFi AP
*/
void setupWiFi() {
  delay(10);

  Serial.print(F("INFO: Connecting to: "));
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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

void setup() {
  Serial.begin(115200);

#ifdef DEBUG_TELNET
  // start the Telnet server
  telnetServer.begin();
  telnetServer.setNoDelay(true);
#endif

  // setup the Wi-Fi
  setupWiFi();

  // init the Arilux LED controller
  if (arilux.init())
    cmd = ARILUX_CMD_STATE_CHANGED;

#ifdef IR_REMOTE
  // start the IR receiver
  irRecv.enableIRIn();
#endif

  // init MQTT
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  mqttClient.setCallback(callback);
  connectMQTT();

  // set hostname and start OTA
  ArduinoOTA.setHostname("arilux");
  ArduinoOTA.begin();
}

void loop() {
#ifdef DEBUG_TELNET
  // handle Telnet connection for debugging
  handleTelnet();
#endif

  yield();

#ifdef IR_REMOTE
  // handle received IR codes from the remote
  handleIRRemote();
#endif

  yield();

  // handle commands
  handleCMD();

  yield();

  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  yield();

  ArduinoOTA.handle();

  yield();
}
