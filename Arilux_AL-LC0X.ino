/*
  Alternative firmware for Arilux AL-LC03, based on the MQTT protocol and a TLS connection

  This firmware can be easily interfaced with Home Assistant, with the MQTT light
  component: https://home-assistant.io/components/light.mqtt/

  CloudMQTT (free until 10 connections): https://www.cloudmqtt.com

  Libraries :
    - ESP8266 core for Arduino :  https://github.com/esp8266/Arduino
    - PubSubClient:               https://github.com/knolleary/pubsubclient
    - IRremoteESP8266:            https://github.com/markszabo/IRremoteESP8266

  Sources :
    - File > Examples > ES8266WiFi > WiFiClient
    - File > Examples > PubSubClient > mqtt_auth
    - https://io.adafruit.com/blog/security/2016/07/05/adafruit-io-security-esp8266/

  MQTT topics and payloads:
    State:
      - State:    arilux/<deviceid>/state/state        ON/OFF
      - Command:  arilux/<deviceid>/state/set          ON/OFF
    Brightness:
      - State:    arilux/<deviceid>/brightness/state   0-255
      - Command:  arilux/<deviceid>/brightness/set     0-255
    Color:
      - State:    arilux/<deviceid>/color/state        0-255,0-255,0-255
      - Command:  arilux/<deviceid>/color/set          0-255,0-255,0-255
    White:
      - State:    arilux/<deviceid>/white/state        0-255,0-255
      - Command:  arilux/<deviceid>/white/set          0-255,0-255
  Configuration (Home Assistant) :
    light:
      - platform: mqtt
        name: 'Arilux RGB Led Controller'
        state_topic: 'arilux/state/state'
        command_topic: 'arilux/state/set'
        brightness_state_topic: 'arilux/brightness/state'
        brightness_command_topic: 'arilux/brightness/set'
        rgb_state_topic: 'arilux/color/state'
        rgb_command_topic: 'arilux/color/set'

  Demo: https://www.youtube.com/watch?v=IKh0inaLvAU

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  Samuel M. - v1.0 - 11.2016
  If you like this example, please add a star! Thank you!
  https://github.com/mertenats/Arilux_AL-LC03
*/


//#define IR_REMOTE
// TLS support, make sure to edit the fingerprint and the MQTT broker IP address if
// you are not using CloudMQTT
//#define TLS
#define DEBUG_TELNET

#include <ESP8266WiFi.h>        // https://github.com/esp8266/Arduino
#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient/releases/tag/v2.6
#ifdef IR_REMOTE
#include <IRremoteESP8266.h>  // https://github.com/markszabo/IRremoteESP8266
#endif
#include <ArduinoOTA.h>
#include "Arilux.h"

#ifdef TLS
  // SHA1 fingerprint of the certificate
  // openssl x509 -fingerprint -in  <certificate>.crt
  const char* fingerprint = "A5 02 FF 13 99 9F 8B 39 8E F1 83 4F 11 23 65 0B 32 36 FC 07";
#endif

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
  #define     DEBUG_PRINT(x)    Serial.print(x)
  #define     DEBUG_PRINTLN(x)  Serial.println(x)
#endif

// Wi-Fi
const char*   WIFI_SSID     = "<ssid>";
const char*   WIFI_PASSWORD = "<password>";
#define HOST "ARILUX%s"
char chipid[12];

// MQTT topics
char   ARILUX_MQTT_STATE_STATE_TOPIC[44];
char   ARILUX_MQTT_STATE_COMMAND_TOPIC[44];
char   ARILUX_MQTT_BRIGHTNESS_STATE_TOPIC[44];
char   ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC[44];
char   ARILUX_MQTT_COLOR_STATE_TOPIC[44];
char   ARILUX_MQTT_COLOR_COMMAND_TOPIC[44];
char   ARILUX_MQTT_WHITE_STATE_TOPIC[44];
char   ARILUX_MQTT_WHITE_COMMAND_TOPIC[44];

#define DEFARILUX_MQTT_STATE_STATE_TOPIC "arilux/%s/state/state"
#define DEFARILUX_MQTT_STATE_COMMAND_TOPIC  "arilux/%s/state/set"
#define DEFARILUX_MQTT_BRIGHTNESS_STATE_TOPIC "arilux/%s/brightness/state"
#define DEFARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC "arilux/%s/brightness/set"
#define DEFARILUX_MQTT_COLOR_STATE_TOPIC  "arilux/%s/color/state"
#define DEFARILUX_MQTT_COLOR_COMMAND_TOPIC  "arilux/%s/color/set"
#define DEFARILUX_MQTT_WHITE_STATE_TOPIC  "arilux/%s/white/state"
#define DEFARILUX_MQTT_WHITE_COMMAND_TOPIC  "arilux/%s/white/set"
const char*   TOPICPING =  "ping";
const char*   TOPICPONG = "pong";


// MQTT payloads
const char*   ARILUX_MQTT_STATE_WHITEFULLON_PAYLOAD         = "2";
const char*   ARILUX_MQTT_STATE_ON_PAYLOAD          = "1";
const char*   ARILUX_MQTT_STATE_OFF_PAYLOAD         = "0";

// MQTT buffer
char msgBuffer[32];

// MQTT
const char*   MQTT_CLIENT_ID                        = "arilux";
const char*   MQTT_SERVER_IP                        = "192.168.1.200";
const int     MQTT_SERVER_PORT                      = 1883;
const char*   MQTT_USER                             = "";
const char*   MQTT_PASSWORD                         = "";

volatile uint8_t cmd = ARILUX_CMD_NOT_DEFINED;

Arilux              arilux;
#ifdef IR_REMOTE
  IRrecv            irRecv(ARILUX_IR_PIN);
#endif
#ifdef TLS
  WiFiClientSecure  wifiClient;
#else
  WiFiClient        wifiClient;
#endif
PubSubClient        mqttClient(wifiClient);

///////////////////////////////////////////////////////////////////////////
//  SSL/TLS
///////////////////////////////////////////////////////////////////////////
/*
  Function called to verify the fingerprint of the MQTT server certificate
 */
#ifdef TLS
void verifyFingerprint() {
  DEBUG_PRINT(F("INFO: Connecting to "));
  DEBUG_PRINTLN(MQTT_SERVER_IP);

  if (!wifiClient.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT)) {
    DEBUG_PRINTLN(F("ERROR: Connection failed. Halting execution"));
    delay(1000);
    ESP.reset();
  }

  if (wifiClient.verify(fingerprint, MQTT_SERVER_IP)) {
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
    } else if (payload.equals(String(ARILUX_MQTT_STATE_WHITEFULLON_PAYLOAD))) {
      if (arilux.turnOn())
        cmd = ARILUX_CMD_STATE_CHANGED;
      arilux.setWhite(255,255);
      arilux.setBrightness(255);
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
    } else if (String(ARILUX_MQTT_WHITE_COMMAND_TOPIC).equals(p_topic)) {
      uint8_t firstIndex = payload.indexOf(',');
      if (arilux.setWhite(payload.substring(0, firstIndex).toInt(), payload.substring(firstIndex + 1).toInt()))
        cmd = ARILUX_CMD_WHITE_CHANGED;
    } else if (String(TOPICPING).equals(p_topic)) {
        cmd = ARILUX_CMD_PING;
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
  if (mqttClient.subscribe(ARILUX_MQTT_WHITE_COMMAND_TOPIC)) {
    DEBUG_PRINT(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
    DEBUG_PRINTLN(ARILUX_MQTT_WHITE_COMMAND_TOPIC);
  } else {
    DEBUG_PRINT(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
    DEBUG_PRINTLN(ARILUX_MQTT_WHITE_COMMAND_TOPIC);
  }
  if (mqttClient.subscribe(TOPICPING)) {
    DEBUG_PRINT(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
    DEBUG_PRINTLN(TOPICPING);
  } else {
    DEBUG_PRINT(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
    DEBUG_PRINTLN(TOPICPING);
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
        DEBUG_PRINTLN(F("ERROR: IR code not defined"));
        break;
    }
    irRecv.resume();
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
      case ARILUX_CMD_WHITE_CHANGED:
      snprintf(msgBuffer, sizeof(msgBuffer), "%d,%d", arilux.getWhite1Value(), arilux.getWhite2Value());
      if (mqttClient.publish(ARILUX_MQTT_WHITE_STATE_TOPIC, msgBuffer, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
        DEBUG_PRINT(ARILUX_MQTT_WHITE_STATE_TOPIC);
        DEBUG_PRINT(F(". Payload: "));
        DEBUG_PRINTLN(msgBuffer);
      } else {
        DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
      }
      cmd = ARILUX_CMD_NOT_DEFINED;
      break;
      case ARILUX_CMD_PING:
      snprintf(msgBuffer, sizeof(msgBuffer), "arilux/%s", chipid);
      if (mqttClient.publish(TOPICPONG, msgBuffer, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
        DEBUG_PRINT(TOPICPONG);
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
  char host[64];
  Serial.begin(115200);
  delay(500);
#ifdef DEBUG_TELNET
  // start the Telnet server
  telnetServer.begin();
  telnetServer.setNoDelay(true);
#endif
  sprintf(chipid, "%08X", ESP.getChipId());
  sprintf(host, HOST, chipid);
  Serial.print("hostname:");
  Serial.println(host);
  WiFi.hostname(host);

  // setup the Wi-Fi
  setupWiFi();

  // init the Arilux LED controller
  if (arilux.init())
    cmd = ARILUX_CMD_STATE_CHANGED;

#ifdef IR_REMOTE
  // start the IR receiver
  irRecv.enableIRIn();
#endif

#ifdef TLS
  // check the fingerprint of io.adafruit.com's SSL cert
  verifyFingerprint();
#endif
  sprintf(ARILUX_MQTT_STATE_STATE_TOPIC, DEFARILUX_MQTT_STATE_STATE_TOPIC,chipid);
  sprintf(ARILUX_MQTT_STATE_COMMAND_TOPIC,DEFARILUX_MQTT_STATE_COMMAND_TOPIC,chipid);
  sprintf(ARILUX_MQTT_BRIGHTNESS_STATE_TOPIC,DEFARILUX_MQTT_BRIGHTNESS_STATE_TOPIC,chipid);
  sprintf(ARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC,DEFARILUX_MQTT_BRIGHTNESS_COMMAND_TOPIC,chipid);
  sprintf(ARILUX_MQTT_COLOR_STATE_TOPIC,DEFARILUX_MQTT_COLOR_STATE_TOPIC,chipid);
  sprintf(ARILUX_MQTT_COLOR_COMMAND_TOPIC,DEFARILUX_MQTT_COLOR_COMMAND_TOPIC,chipid);
  sprintf(ARILUX_MQTT_WHITE_STATE_TOPIC,DEFARILUX_MQTT_WHITE_STATE_TOPIC,chipid);
  sprintf(ARILUX_MQTT_WHITE_COMMAND_TOPIC,DEFARILUX_MQTT_WHITE_COMMAND_TOPIC,chipid);
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  mqttClient.setCallback(callback);
  connectMQTT();

  // set hostname and start OTA
  ArduinoOTA.setHostname(host);
   ArduinoOTA.onStart([]() {
      arilux.setAll(0,0,0,0,0);  
  
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void loop() {
#ifdef DEBUG_TELNET
  // handle Telnet connection for debugging
  handleTelnet();
#endif
#ifdef IR_REMOTE
  // handle received IR codes from the remote
  handleIRRemote();
#endif
  // handle commands
  handleCMD();
  yield();
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();
  ArduinoOTA.handle();
}
