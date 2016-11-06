/*
 *  Arilux AL-LC03
 *  Alternative firmware
 *
 *  Work in progress...
 */
#include <ESP8266WiFi.h>    // https://github.com/esp8266/Arduino
#include <ArduinoOTA.h>

// Analog output
// http://esp8266.github.io/Arduino/versions/2.0.0/doc/reference.html
const PROGMEM uint8_t   PWM_RANGE     = 255; // 0-255, 1023 by default
const PROGMEM uint16_t  PWM_FREQUENCY = 500; // 1 [kHz] by default

// RGB pins
const PROGMEM uint8_t   GREEN_PIN     = 5;
const PROGMEM uint8_t   RED_PIN       = 14;
const PROGMEM uint8_t   BLUE_PIN      = 12;

// IR pin
const PROGMEM uint8_t   IR_PIN         = 4;

#define IR_REMOTE
#ifdef IR_REMOTE
  #include <IRremoteESP8266.h> // https://github.com/markszabo/IRremoteESP8266
  IRrecv irRecv(IR_PIN);
#endif

//#define DEBUG_SERIAL
#define DEBUG_TELNET
#ifdef DEBUG_TELNET
  WiFiServer telnetServer(23);
  WiFiClient telnetClient;
#endif

/*
 * IR Remote
 * Encoding: NEC
 * +------+------+------+------+
 * |  UP  | Down | OFF  |  ON  |
 * +------+------+------+------+
 * |  R   |  G   |  B   |  W   |
 * +------+------+------+------+
 * |  1   |  2   |  3   |FLASH |
 * +------+------+------+------+
 * |  4   |  5   |  6   |STROBE|
 * +------+------+------+------+
 * |  7   |  8   |  9   | FADE |
 * +------+------+------+------+
 * |  10  |  11  |  12  |SMOOTH|
 * +------+------+------+------+
 */
#ifdef IR_REMOTE
const unsigned int  IR_CODE_KEY_UP      = 0xFF906F;
const unsigned int  IR_CODE_KEY_DOWN    = 0xFFB847;
const unsigned int  IR_CODE_KEY_OFF     = 0xFFF807;
const unsigned int  IR_CODE_KEY_ON      = 0xFFB04F;
const unsigned int  IR_CODE_KEY_R       = 0xFF9867;
const unsigned int  IR_CODE_KEY_G       = 0xFFD827;
const unsigned int  IR_CODE_KEY_B       = 0xFF8877;
const unsigned int  IR_CODE_KEY_W       = 0xFFA857;
const unsigned int  IR_CODE_KEY_1       = 0xFFE817;
const unsigned int  IR_CODE_KEY_2       = 0xFF48B7;
const unsigned int  IR_CODE_KEY_3       = 0xFF6897;
const unsigned int  IR_CODE_KEY_FLASH   = 0xFFB24D;
const unsigned int  IR_CODE_KEY_4       = 0xFF02FD;
const unsigned int  IR_CODE_KEY_5       = 0xFF32CD;
const unsigned int  IR_CODE_KEY_6       = 0xFF20DF;
const unsigned int  IR_CODE_KEY_STROBE  = 0xFF00FF;
const unsigned int  IR_CODE_KEY_7       = 0xFF50AF;
const unsigned int  IR_CODE_KEY_8       = 0xFF7887;
const unsigned int  IR_CODE_KEY_9       = 0xFF708F;
const unsigned int  IR_CODE_KEY_FADE    = 0xFF58A7;
const unsigned int  IR_CODE_KEY_10      = 0xFF38C7;
const unsigned int  IR_CODE_KEY_11      = 0xFF28D7;
const unsigned int  IR_CODE_KEY_12      = 0xFFF00F;
const unsigned int  IR_CODE_KEY_SMOOTH  = 0xFF30CF;
#endif

// Macros for debugging
#ifdef DEBUG_SERIAL
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif
#ifdef DEBUG_TELNET
  #define DEBUG_PRINT(x)    telnetClient.print(x)
  #define DEBUG_PRINTLN(x)  telnetClient.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Wi-Fi
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

/*
 * Function called to handle Telnet clients
 * https://www.youtube.com/watch?v=j9yW10OcahI
 */
#ifdef DEBUG_TELNET
void handleTelnet() {
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
 * Function called to handle received IR codes from the remote
 */
#ifdef IR_REMOTE
void handleIRRemote() {
  decode_results  results;

  if (irRecv.decode(&results)) {
    switch(results.value) {
      case IR_CODE_KEY_UP:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_UP"));
        break;
      case IR_CODE_KEY_DOWN:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_DOWN"));
        break;
      case IR_CODE_KEY_OFF:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_OFF"));
        break;
      case IR_CODE_KEY_ON:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_ON"));
        break;
      case IR_CODE_KEY_R:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_R"));
        break;
      case IR_CODE_KEY_G:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_G"));
        break;
      case IR_CODE_KEY_B:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_B"));
        break;
      case IR_CODE_KEY_W:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_W"));
        break;
      case IR_CODE_KEY_1:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_1"));
        break;
      case IR_CODE_KEY_2:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_2"));
        break;
      case IR_CODE_KEY_3:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_3"));
        break;
      case IR_CODE_KEY_FLASH:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_FLASH"));
        break;
      case IR_CODE_KEY_4:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_4"));
        break;
      case IR_CODE_KEY_5:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_5"));
        break;
      case IR_CODE_KEY_6:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_6"));
        break;
      case IR_CODE_KEY_STROBE:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_STROBE"));
        break;
      case IR_CODE_KEY_7:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_7"));
        break;
      case IR_CODE_KEY_8:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_8"));
        break;
      case IR_CODE_KEY_9:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_9"));
        break;
      case IR_CODE_KEY_FADE:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_FADE"));
        break;
      case IR_CODE_KEY_10:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_10"));
        break;
      case IR_CODE_KEY_11:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_11"));
        break;
      case IR_CODE_KEY_12:
        DEBUG_PRINTLN(F("INFO: IR_CODE_KEY_12"));
        break;
      case IR_CODE_KEY_SMOOTH:
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

void setupWiFi() {
  delay(10);
  //DEBUG_PRINT(F("INFO: Connecting to: "));
  //DEBUG_PRINTLN(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //DEBUG_PRINT(".");
  }

  randomSeed(micros());

  //DEBUG_PRINTLN();
  //DEBUG_PRINTLN(F("INFO: WiFi connected"));
  //DEBUG_PRINT(F("INFO: IP address: "));
  //DEBUG_PRINTLN(WiFi.localIP());
}

void setup() {
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
#endif
#ifdef DEBUG_TELNET
  telnetServer.begin();
  telnetServer.setNoDelay(true);
#endif

  // Wi-Fi
  setupWiFi();

  // set the PWM frequency and range
  analogWriteFreq(PWM_FREQUENCY);
  analogWriteRange(PWM_RANGE);

  // init the pins as OUTPUT
  pinMode(RED_PIN,    OUTPUT);
  pinMode(GREEN_PIN,  OUTPUT);
  pinMode(BLUE_PIN,   OUTPUT);

  // start the IR receiver
  pinMode(IR_PIN,     INPUT);
  irRecv.enableIRIn();


  ArduinoOTA.setHostname("esp8266");
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

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
}
