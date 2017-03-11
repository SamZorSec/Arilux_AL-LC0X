/*
  Alternative firmware for Arilux AL-LC0X series of ESP8266 based RGB LED controllers.
  See the README at https://github.com/mertenats/Arilux_AL-LC0X for more information.
  Licensed under the MIT license.
*/

#include "config.h"
#include <ESP8266WiFi.h>        // https://github.com/esp8266/Arduino
#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient/releases/tag/v2.6
#ifdef IR_REMOTE
#include <IRremoteESP8266.h>  // https://github.com/markszabo/IRremoteESP8266
#endif
#ifdef RF_REMOTE
#include <RCSwitch.h>         // https://github.com/sui77/rc-switch
#endif
#include <ArduinoOTA.h>
#if defined(HOME_ASSISTANT_MQTT_DISCOVERY) || defined (JSON)
  #include <ArduinoJson.h>
#endif
#include "Arilux.h"

// in a terminal: telnet arilux.local
#ifdef DEBUG_TELNET
WiFiServer  telnetServer(23);
WiFiClient  telnetClient;
#endif

// Macros for debugging
#ifdef DEBUG_TELNET
#define     DEBUG_PRINT(x)    telnetClient.print(x)
#define     DEBUG_PRINT_WITH_FMT(x, fmt)    telnetClient.print(x, fmt)
#define     DEBUG_PRINTLN(x)  telnetClient.println(x)
#define     DEBUG_PRINTLN_WITH_FMT(x, fmt)  telnetClient.println(x, fmt)
#else
#define     DEBUG_PRINT(x)    Serial.print(x)
#define     DEBUG_PRINT_WITH_FMT(x, fmt)    Serial.print(x, fmt)
#define     DEBUG_PRINTLN(x)  Serial.println(x)
#define     DEBUG_PRINTLN_WITH_FMT(x, fmt)  Serial.println(x, fmt)
#endif

char   chipid[12];
char   MQTT_CLIENT_ID[32];
char   MQTT_TOPIC_PREFIX[32];

// MQTT topics
char   ARILUX_MQTT_STATUS_TOPIC[44];
#ifdef HOME_ASSISTANT_MQTT_DISCOVERY
  char   HOME_ASSISTANT_MQTT_DISCOVERY_TOPIC[56];
#endif
#ifdef JSON
  char   ARILUX_MQTT_JSON_STATE_TOPIC[44];
  char   ARILUX_MQTT_JSON_COMMAND_TOPIC[44];
#else
  char   ARILUX_MQTT_STATE_STATE_TOPIC[44];
  char   ARILUX_MQTT_STATE_COMMAND_TOPIC[44];
  char   ARILUX_MQTT_BRIGHTNESS_STATE_TOPIC[44];
  char   ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC[44];
  char   ARILUX_MQTT_COLOR_STATE_TOPIC[44];
  char   ARILUX_MQTT_COLOR_COMMAND_TOPIC[44];
  #if defined(RGBW) || defined (RGBWW)
    char   ARILUX_MQTT_WHITE_STATE_TOPIC[44];
    char   ARILUX_MQTT_WHITE_COMMAND_TOPIC[44];
  #endif
#endif

// MQTT buffer
char msgBuffer[32];
char outgoingJsonBuffer[120];

char friendlyName[32];
char configBuf[512];
#ifdef HOME_ASSISTANT_MQTT_DISCOVERY
  StaticJsonBuffer<512> HOME_ASSISTANT_MQTT_DISCOVERY_CONFIG;
#endif

volatile uint8_t cmd = ARILUX_CMD_NOT_DEFINED;

Arilux              arilux;
#ifdef IR_REMOTE
IRrecv            irRecv(ARILUX_IR_PIN);
#endif
#ifdef RF_REMOTE
RCSwitch          rcSwitch = RCSwitch();
#endif
#ifdef TLS
WiFiClientSecure  wifiClient;
#else
WiFiClient        wifiClient;
#endif
PubSubClient        mqttClient(wifiClient);

// Real values to write to the LEDs (ex. including brightness and state)
byte realRed = 0;
byte realGreen = 0;
byte realBlue = 0;

// Globals for fade/transitions
bool startFade = false;
unsigned long lastLoop = 0;
int transitionTime = 0;
bool inFade = false;
int loopCount = 0;
int stepR, stepG, stepB;
int redVal, grnVal, bluVal;

// Globals for flash
bool flash = false;
bool startFlash = false;
int flashLength = 0;
unsigned long flashStartTime = 0;
byte flashRed = 0;
byte flashGreen = 0;
byte flashBlue = 0;
byte flashBrightness = 0;

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
//  MQTT
///////////////////////////////////////////////////////////////////////////
/*
   Function called when a MQTT message arrived
   @param p_topic   The topic of the MQTT message
   @param p_payload The payload of the MQTT message
   @param p_length  The length of the payload
*/
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // Concatenate the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }

  // Handle the MQTT topic of the received message
  #ifdef JSON
    if (String(ARILUX_MQTT_JSON_COMMAND_TOPIC).equals(p_topic)) {
      DynamicJsonBuffer incomingJsonPayload;
      JsonObject& root = incomingJsonPayload.parseObject(payload);
      if (!root.success()) {
        DEBUG_PRINTLN("parseObject() failed");
        return;
      }

      if (root.containsKey("color")) {
        int red_color = root["color"]["r"];
        int green_color = root["color"]["g"];
        int blue_color = root["color"]["b"];

        realRed = red_color;
        realGreen = green_color;
        realBlue = blue_color;
      } else {
        realRed = arilux.getRedValue();
        realGreen = arilux.getGreenValue();
        realBlue = arilux.getBlueValue();
      }

      startFade = true;
      inFade = false; // Kill the current fade

      if (root.containsKey("flash")) {
        flashLength = (int)root["flash"] * 1000;

        if (root.containsKey("brightness")) {
          flashBrightness = root["brightness"];
        } else {
          flashBrightness = arilux.getBrightness();
        }

        if (root.containsKey("color")) {
          flashRed = root["color"]["r"];
          flashGreen = root["color"]["g"];
          flashBlue = root["color"]["b"];
        } else {
          flashRed = arilux.getRedValue();
          flashGreen = arilux.getGreenValue();
          flashBlue = arilux.getBlueValue();
        }

        flashRed = map(flashRed, 0, 255, 0, flashBrightness);
        flashGreen = map(flashGreen, 0, 255, 0, flashBrightness);
        flashBlue = map(flashBlue, 0, 255, 0, flashBrightness);

        flash = true;
        startFlash = true;
      } else { // Not flashing
        flash = false;
        if (root.containsKey("state")) {
          if (strcmp(root["state"], "ON") == 0) {
            arilux.turnOn();
          } else if (strcmp(root["state"], "OFF") == 0) {
            arilux.turnOff();
          }
        }

        if (root.containsKey("transition")) {
          transitionTime = root["transition"];
        } else {
          transitionTime = 0;
        }

        if (root.containsKey("brightness")) {
          int brightness = root["brightness"];
          arilux.setBrightness(brightness);
        }

        if (root.containsKey("white_value")) {
          int white_value = root["white_value"];
          arilux.setWhite(white_value, white_value);
        }
      }
      cmd = ARILUX_CMD_JSON;
    }
  #else
    if (String(ARILUX_MQTT_STATE_COMMAND_TOPIC).equals(p_topic)) {
      if (payload.equals(String(MQTT_STATE_ON_PAYLOAD))) {
        if (arilux.turnOn())
          cmd = ARILUX_CMD_STATE_CHANGED;
      } else if (payload.equals(String(MQTT_STATE_OFF_PAYLOAD))) {
        if (arilux.turnOff())
          cmd = ARILUX_CMD_STATE_CHANGED;
      }
    } else if (String(ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC).equals(p_topic)) {
      if (arilux.setBrightness(payload.toInt()))
        cmd = ARILUX_CMD_BRIGHTNESS_CHANGED;
    } else if (String(ARILUX_MQTT_COLOR_COMMAND_TOPIC).equals(p_topic)) {
      // Get the position of the first and second commas
      int commaIndex = payload.indexOf(',');
      //  Search for the next comma just after the first
      int secondCommaIndex = payload.indexOf(',', commaIndex + 1);
      String firstValue = payload.substring(0, commaIndex);
      String secondValue = payload.substring(commaIndex + 1, secondCommaIndex);
      String thirdValue = payload.substring(secondCommaIndex + 1); // To the end of the string
      int r = firstValue.toInt();
      int g = secondValue.toInt();
      int b = thirdValue.toInt();

      if (arilux.setColor(r, g, b))
        cmd = ARILUX_CMD_COLOR_CHANGED;
    }
    #if defined(RGBW) || defined (RGBWW)
      if (String(ARILUX_MQTT_WHITE_COMMAND_TOPIC).equals(p_topic)) {
        uint8_t firstIndex = payload.indexOf(',');
        if (arilux.setWhite(payload.substring(0, firstIndex).toInt(), payload.substring(firstIndex + 1).toInt()))
          cmd = ARILUX_CMD_WHITE_CHANGED;
      }
    #endif
  #endif
}

/*
  Function called to connect/reconnect to the MQTT broker
*/

volatile unsigned long lastmqttreconnect = 0;
void connectMQTT(void) {
  if (!mqttClient.connected()) {
    if (lastmqttreconnect + 1000 < millis()) {
      if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, ARILUX_MQTT_STATUS_TOPIC, 0, 1, "dead")) {
        DEBUG_PRINTLN(F("INFO: The client is successfully connected to the MQTT broker"));
        publishToMQTT(ARILUX_MQTT_STATUS_TOPIC, "alive");
        #ifdef HOME_ASSISTANT_MQTT_DISCOVERY
          JsonObject& root = HOME_ASSISTANT_MQTT_DISCOVERY_CONFIG.createObject();
          root["name"] = friendlyName;
          #ifdef JSON
            root["platform"] = "mqtt_json";
            root["state_topic"] = ARILUX_MQTT_JSON_STATE_TOPIC;
            root["command_topic"] = ARILUX_MQTT_JSON_COMMAND_TOPIC;
            root["brightness"] = true;
            root["rgb"] = true;
            #if defined(RGBW) || defined (RGBWW)
            root["white_value"] = true;
            #endif
          #else
            root["state_topic"] = ARILUX_MQTT_STATE_STATE_TOPIC;
            root["command_topic"] = ARILUX_MQTT_STATE_COMMAND_TOPIC;
            root["brightness_state_topic"] = ARILUX_MQTT_BRIGHTNESS_STATE_TOPIC;
            root["brightness_command_topic"] = ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC;
            root["rgb_state_topic"] = ARILUX_MQTT_COLOR_STATE_TOPIC;
            root["rgb_command_topic"] = ARILUX_MQTT_COLOR_COMMAND_TOPIC;
            root["payload_on"] = MQTT_STATE_ON_PAYLOAD;
            root["payload_off"] = MQTT_STATE_OFF_PAYLOAD;
          #endif
          root.printTo(configBuf, sizeof(configBuf));
          publishToMQTT(HOME_ASSISTANT_MQTT_DISCOVERY_TOPIC, configBuf);
        #endif
        flashSuccess(true);
      } else {
        DEBUG_PRINTLN(F("ERROR: The connection to the MQTT broker failed"));
        DEBUG_PRINT(F("Username: "));
        DEBUG_PRINTLN(MQTT_USER);
        DEBUG_PRINT(F("Password: "));
        DEBUG_PRINTLN(MQTT_PASS);
        DEBUG_PRINT(F("Broker: "));
        DEBUG_PRINTLN(MQTT_SERVER);
        flashSuccess(false);
      }

#ifdef JSON
      subscribeToMQTTTopic(ARILUX_MQTT_JSON_COMMAND_TOPIC);
#else
      subscribeToMQTTTopic(ARILUX_MQTT_STATE_COMMAND_TOPIC);
      subscribeToMQTTTopic(ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC);
      subscribeToMQTTTopic(ARILUX_MQTT_COLOR_COMMAND_TOPIC);

      #if defined(RGBW) || defined (RGBWW)
            subscribeToMQTTTopic(ARILUX_MQTT_WHITE_COMMAND_TOPIC);
      #endif
#endif

      lastmqttreconnect = millis();
    }
  }
}

/*
  Helper function to subscribe to a MQTT topic
*/

void subscribeToMQTTTopic(const char* topic) {
  if (mqttClient.subscribe(topic)) {
    DEBUG_PRINT(F("INFO: Sending the MQTT subscribe succeeded for topic: "));
    DEBUG_PRINTLN(topic);
  } else {
    DEBUG_PRINT(F("ERROR: Sending the MQTT subscribe failed for topic: "));
    DEBUG_PRINTLN(topic);
  }
}

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
//   TELNET
///////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////
//  IR REMOTE
///////////////////////////////////////////////////////////////////////////
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
        DEBUG_PRINT(F("ERROR: IR code not defined: "));
        DEBUG_PRINTLN_WITH_FMT(results.value, HEX);
        break;
    }
    irRecv.resume();
  }
}
#endif

///////////////////////////////////////////////////////////////////////////
//  RF REMOTE
///////////////////////////////////////////////////////////////////////////
/*
   Function called to handle received RF codes from the remote
*/
#ifdef RF_REMOTE
void handleRFRemote(void) {
  if (rcSwitch.available()) {
    int value = rcSwitch.getReceivedValue();

    switch (value) {
      case ARILUX_RF_CODE_KEY_BRIGHT_PLUS:
        if (arilux.increaseBrightness())
          cmd = ARILUX_CMD_BRIGHTNESS_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_BRIGHT_MINUS:
        if (arilux.decreaseBrightness())
          cmd = ARILUX_CMD_BRIGHTNESS_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_OFF:
        if (arilux.turnOff())
          cmd = ARILUX_CMD_STATE_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_ON:
        if (arilux.turnOn())
          cmd = ARILUX_CMD_STATE_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_RED:
        if (arilux.setColor(255, 0, 0))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_GREEN:
        if (arilux.setColor(0, 255, 0))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_BLUE:
        if (arilux.setColor(0, 0, 255))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_WHITE:
        if (arilux.setColor(255, 255, 255))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_ORANGE:
        if (arilux.setColor(255, 165, 0))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_LTGRN:
        if (arilux.setColor(144, 238, 144))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_LTBLUE:
        if (arilux.setColor(173, 216, 230))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_AMBER:
        if (arilux.setColor(255, 194, 0))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_CYAN:
        if (arilux.setColor(0, 255, 255))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_PURPLE:
        if (arilux.setColor(128, 0, 128))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_YELLOW:
        if (arilux.setColor(255, 255, 0))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_PINK:
        if (arilux.setColor(255, 192, 203))
          cmd = ARILUX_CMD_COLOR_CHANGED;
        break;
      case ARILUX_RF_CODE_KEY_TOGGLE:
        // TODO
        DEBUG_PRINTLN(F("INFO: ARILUX_RF_CODE_KEY_TOGGLE"));
        break;
      case ARILUX_RF_CODE_KEY_SPEED_PLUS:
        // TODO
        DEBUG_PRINTLN(F("INFO: ARILUX_RF_CODE_KEY_SPEED_PLUS"));
        break;
      case ARILUX_RF_CODE_KEY_MODE_PLUS:
        // TODO
        DEBUG_PRINTLN(F("INFO: ARILUX_RF_CODE_KEY_MODE_PLUS"));
        break;
      case ARILUX_RF_CODE_KEY_SPEED_MINUS:
        // TODO
        DEBUG_PRINTLN(F("INFO: ARILUX_RF_CODE_KEY_SPEED_MINUS"));
        break;
      case ARILUX_RF_CODE_KEY_MODE_MINUS:
        // TODO
        DEBUG_PRINTLN(F("INFO: ARILUX_RF_CODE_KEY_MODE_MINUS"));
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
//  CMD
///////////////////////////////////////////////////////////////////////////
/*
   Function called to handle commands due to changes
*/
void handleCMD(void) {
  #ifdef JSON
  if (cmd != ARILUX_CMD_NOT_DEFINED) {
      DynamicJsonBuffer outgoingJsonPayload;
      JsonObject& root = outgoingJsonPayload.createObject();
      String stringState = arilux.getState() ? "ON" : "OFF";
      root["state"] = stringState;
      root["brightness"] = arilux.getBrightness();
      // root["transition"] =
      root["white_value"] = arilux.getWhite1Value();
      root["color"]["r"] = arilux.getRedValue();
      root["color"]["g"] = arilux.getGreenValue();
      root["color"]["b"] = arilux.getBlueValue();
      root.printTo(outgoingJsonBuffer);
      publishToMQTT(ARILUX_MQTT_JSON_STATE_TOPIC, outgoingJsonBuffer);
  };
  #else
    switch (cmd) {
      case ARILUX_CMD_NOT_DEFINED:
        break;
      case ARILUX_CMD_STATE_CHANGED:
        publishStateChange();
        break;
      case ARILUX_CMD_BRIGHTNESS_CHANGED:
        publishBrightnessChange();
        break;
      case ARILUX_CMD_COLOR_CHANGED:
        publishColorChange();
        break;
      #if defined(RGBW) || defined (RGBWW)
        case ARILUX_CMD_WHITE_CHANGED:
          publishWhiteChange();
          break;
      #endif
      default:
        break;
    }
  #endif
  cmd = ARILUX_CMD_NOT_DEFINED;
}

#ifndef JSON
  void publishStateChange(void) {
    publishToMQTT(ARILUX_MQTT_STATE_STATE_TOPIC, (arilux.getState() ? MQTT_STATE_ON_PAYLOAD : MQTT_STATE_OFF_PAYLOAD));
  }

  void publishBrightnessChange(void) {
    snprintf(msgBuffer, sizeof(msgBuffer), "%d", arilux.getBrightness());
    publishToMQTT(ARILUX_MQTT_BRIGHTNESS_STATE_TOPIC, msgBuffer);
  }

  void publishColorChange(void) {
    snprintf(msgBuffer, sizeof(msgBuffer), "%d,%d,%d", arilux.getRedValue(), arilux.getGreenValue(), arilux.getBlueValue());
    publishToMQTT(ARILUX_MQTT_COLOR_STATE_TOPIC, msgBuffer);
  }

  void publishWhiteChange(void) {
    snprintf(msgBuffer, sizeof(msgBuffer), "%d,%d", arilux.getWhite1Value(), arilux.getWhite2Value());
    publishToMQTT(ARILUX_MQTT_WHITE_STATE_TOPIC, msgBuffer);
  }
#endif

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
//  SETUP() AND LOOP()
///////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  delay(500);

#ifdef DEBUG_TELNET
  // Start the Telnet server
  telnetServer.begin();
  telnetServer.setNoDelay(true);
#endif

  sprintf(chipid, "%08X", ESP.getChipId());
  sprintf(MQTT_CLIENT_ID, HOST, chipid);
  sprintf(friendlyName, "Arilux %s %s LED Controller %s", DEVICE_MODEL, arilux.getColorString(), chipid);
  Serial.print("Hostname:");
  Serial.println(MQTT_CLIENT_ID);
  WiFi.hostname(MQTT_CLIENT_ID);

  // Setup Wi-Fi
  setupWiFi();

  // Init the Arilux LED controller
  if (arilux.init())
    cmd = ARILUX_CMD_STATE_CHANGED;

#ifdef IR_REMOTE
  // Start the IR receiver
  irRecv.enableIRIn();
#endif

#ifdef RF_REMOTE
  // Start the RF receiver
  rcSwitch.enableReceive(ARILUX_RF_PIN);
#endif

#ifdef TLS
  // Check the fingerprint of CloudMQTT's SSL cert
  verifyFingerprint();
#endif

  sprintf(MQTT_TOPIC_PREFIX, MQTT_TOPIC_PREFIX_TEMPLATE, arilux.getColorString(), chipid);

  sprintf(ARILUX_MQTT_STATUS_TOPIC, MQTT_STATUS_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);

#ifdef HOME_ASSISTANT_MQTT_DISCOVERY
  sprintf(HOME_ASSISTANT_MQTT_DISCOVERY_TOPIC,"%s/light/ARILUX_%s_%s_%s/config",HOME_ASSISTANT_MQTT_DISCOVERY_PREFIX,DEVICE_MODEL,arilux.getColorString(),chipid);
#endif

#ifdef JSON
  sprintf(ARILUX_MQTT_JSON_STATE_TOPIC, MQTT_JSON_STATE_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);
  sprintf(ARILUX_MQTT_JSON_COMMAND_TOPIC, MQTT_JSON_COMMAND_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);
#else
  sprintf(ARILUX_MQTT_STATE_STATE_TOPIC, MQTT_STATE_STATE_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);
  sprintf(ARILUX_MQTT_STATE_COMMAND_TOPIC, MQTT_STATE_COMMAND_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);
  sprintf(ARILUX_MQTT_BRIGHTNESS_STATE_TOPIC, MQTT_BRIGHTNESS_STATE_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);
  sprintf(ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC, MQTT_BRIGHTNESS_COMMAND_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);
  sprintf(ARILUX_MQTT_COLOR_STATE_TOPIC, MQTT_COLOR_STATE_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);
  sprintf(ARILUX_MQTT_COLOR_COMMAND_TOPIC, MQTT_COLOR_COMMAND_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);

  #if defined(RGBW) || defined (RGBWW)
    sprintf(ARILUX_MQTT_WHITE_STATE_TOPIC, MQTT_WHITE_STATE_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);
    sprintf(ARILUX_MQTT_WHITE_COMMAND_TOPIC, MQTT_WHITE_COMMAND_TOPIC_TEMPLATE, MQTT_TOPIC_PREFIX);
  #endif
#endif

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(callback);
  connectMQTT();

  // Set hostname and start OTA
  ArduinoOTA.setHostname(MQTT_CLIENT_ID);
  ArduinoOTA.onStart([]() {
    DEBUG_PRINTLN("OTA Beginning!");
    flashSuccess(true);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PRINT("ArduinoOTA Error[");
    DEBUG_PRINT(error);
    DEBUG_PRINT("]: ");
    if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
    else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
  });
  ArduinoOTA.begin();
}

void loop() {
#ifdef DEBUG_TELNET
  // Handle Telnet connection for debugging
  handleTelnet();
#endif

#ifdef IR_REMOTE
  // Handle received IR codes from the remote
  handleIRRemote();
#endif

#ifdef RF_REMOTE
  // Handle received RF codes from the remote
  handleRFRemote();
#endif

  yield();

  // Handle commands
  handleCMD();
  yield();
  handleEffects();
  connectMQTT();
  mqttClient.loop();
  yield();
  ArduinoOTA.handle();
  yield();
}

///////////////////////////////////////////////////////////////////////////
//  Utilities
///////////////////////////////////////////////////////////////////////////

void flashSuccess(bool success) {
  flashLength = 5000;

  flashBrightness = 255;

  if(success) {
    flashRed = 0;
    flashGreen = 255;
  } else {
    flashRed = 255;
    flashGreen = 0;
  }
  flashBlue = 0;

  flashRed = map(flashRed, 0, 255, 0, flashBrightness);
  flashGreen = map(flashGreen, 0, 255, 0, flashBrightness);
  flashBlue = map(flashBlue, 0, 255, 0, flashBrightness);

  flash = true;
  startFlash = true;
}

void handleEffects(void) {
  if (flash) {
    if (startFlash) {
      startFlash = false;
      flashStartTime = millis();
      arilux.setWhite(0, 0);
    }
    if ((millis() - flashStartTime) <= flashLength) {
      if ((millis() - flashStartTime) % 1000 <= 500) {
        arilux.setColor(flashRed, flashGreen, flashBlue);
      } else {
        arilux.setColor(0, 0, 0);
        // If you'd prefer the flashing to happen "on top of"
        // the current color, uncomment the next line.
        // arilux.setColor(realRed, realGreen, realBlue);
      }
    } else {
      flash = false;
      arilux.setColor(realRed, realGreen, realBlue);
    }
  }

  if (startFade) {
    // If we don't want to fade, skip it.
    if (transitionTime == 0) {
      arilux.setColor(realRed, realGreen, realBlue);

      redVal = realRed;
      grnVal = realGreen;
      bluVal = realBlue;

      startFade = false;
    } else {
      loopCount = 0;
      stepR = calculateStep(redVal, realRed);
      stepG = calculateStep(grnVal, realGreen);
      stepB = calculateStep(bluVal, realBlue);

      inFade = true;
    }
  }

  if (inFade) {
    startFade = false;
    unsigned long now = millis();
    if (now - lastLoop > transitionTime) {
      if (loopCount <= 1020) {
        lastLoop = now;

        redVal = calculateVal(stepR, redVal, loopCount);
        grnVal = calculateVal(stepG, grnVal, loopCount);
        bluVal = calculateVal(stepB, bluVal, loopCount);

        arilux.setColor(redVal, grnVal, bluVal); // Write current values to LED pins

        DEBUG_PRINT("Fade Loop count: ");
        DEBUG_PRINTLN(loopCount);
        loopCount++;
      } else {
        inFade = false;
      }
    }
  }
}

// From https://www.arduino.cc/en/Tutorial/ColorCrossfader
/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
*
* The program works like this:
* Imagine a crossfade that moves the red LED from 0-10,
*   the green from 0-5, and the blue from 10 to 7, in
*   ten steps.
*   We'd want to count the 10 steps and increase or
*   decrease color values in evenly stepped increments.
*   Imagine a + indicates raising a value by 1, and a -
*   equals lowering it. Our 10 step fade would look like:
*
*   1 2 3 4 5 6 7 8 9 10
* R + + + + + + + + + +
* G   +   +   +   +   +
* B     -     -     -
*
* The red rises from 0 to 10 in ten steps, the green from
* 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
*
* In the real program, the color percentages are converted to
* 0-255 values, and there are 1020 steps (255*4).
*
* To figure out how big a step there should be between one up- or
* down-tick of one of the LED values, we call calculateStep(),
* which calculates the absolute gap between the start and end values,
* and then divides that gap by 1020 to determine the size of the step
* between adjustments in the value.
*/
int calculateStep(int prevValue, int endValue) {
    int step = endValue - prevValue; // What's the overall gap?
    if (step) {                      // If its non-zero,
        step = 1020/step;            //   divide by 1020
    }

    return step;
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1.
*  (R, G, and B are each calculated separately.)
*/
int calculateVal(int step, int val, int i) {
    if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
        if (step > 0) {              //   increment the value if step is positive...
            val += 1;
        }
        else if (step < 0) {         //   ...or decrement it if step is negative
            val -= 1;
        }
    }

    // Defensive driving: make sure val stays in the range 0-255
    if (val > 255) {
        val = 255;
    }
    else if (val < 0) {
        val = 0;
    }

    return val;
}
