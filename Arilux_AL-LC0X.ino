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
      - State:    arilux/state/state        ON/OFF
      - Command:  arilux/state/set          ON/OFF
    Brightness:
      - State:    arilux/brightness/state   0-255
      - Command:  arilux/brightness/set     0-255
    Color:
      - State:    arilux/color/state        0-255,0-255,0-255
      - Command:  arilux/color/set          0-255,0-255,0-255

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
#define RF_REMOTE

// TLS support, make sure to edit the fingerprint and the MQTT broker IP address if
// you are not using CloudMQTT
#define TLS
//#define DEBUG_TELNET

#include <ESP8266WiFi.h>        // https://github.com/esp8266/Arduino
#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient/releases/tag/v2.6
#ifdef IR_REMOTE
  #include <IRremoteESP8266.h>  // https://github.com/markszabo/IRremoteESP8266
#endif
#ifdef RF_REMOTE
  #include <RCSwitch.h>         // https://github.com/sui77/rc-switch
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
const char*   MQTT_SERVER_IP                        = "m21.cloudmqtt.com";
const int     MQTT_SERVER_PORT                      = 12345;
const char*   MQTT_USER                             = "user";
const char*   MQTT_PASSWORD                         = "password";

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

#ifdef RF_REMOTE
  // start the RF receiver
  rcSwitch.enableReceive(ARILUX_RF_PIN);
#endif

#ifdef TLS
  // check the fingerprint of io.adafruit.com's SSL cert
  verifyFingerprint();
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

#ifdef RF_REMOTE
  // handle received RF codes from the remote
  handleRFRemote();
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
