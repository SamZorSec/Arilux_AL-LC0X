#pragma once
#include "config.h"
#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino

#ifdef IR_REMOTE
#include <IRremoteESP8266.h> // https://github.com/markszabo/IRremoteESP8266
#endif

#if defined(RGBWW)
#define ARILUX_RED_PIN 5
#define ARILUX_GREEN_PIN 4
#define ARILUX_BLUE_PIN 14
#define ARILUX_WHITE1_PIN 12
#define ARILUX_WHITE2_PIN 13
#define ARILUX_COLOR_STRING "RGBWW"
#elif defined(RGBW)
//FF2143 (Other)
#define ARILUX_RED_PIN 5
#define ARILUX_GREEN_PIN 14
//1F162E
//#define ARILUX_RED_PIN            14
//#define ARILUX_GREEN_PIN          5

#define ARILUX_BLUE_PIN 12
#define ARILUX_WHITE1_PIN 13
#define ARILUX_COLOR_STRING "RGBW"
#else
#define ARILUX_RED_PIN 14
#define ARILUX_GREEN_PIN 5
#define ARILUX_BLUE_PIN 12
#define ARILUX_COLOR_STRING "RGB"
#endif

#define ARILUX_IR_PIN 4
#define ARILUX_RF_PIN 4

#define ARILUX_PWM_RANGE 1023
#define ARILUX_PWM_FREQUENCY 1000

/*
   IR Remote
   Encoding: NEC
   +------+------+------+------+
   |  UP  | Down | OFF  |  ON  |
   +------+------+------+------+
   |  R   |  G   |  B   |  W   |
   +------+------+------+------+
   |  1   |  2   |  3   |FLASH |
   +------+------+------+------+
   |  4   |  5   |  6   |STROBE|
   +------+------+------+------+
   |  7   |  8   |  9   | FADE |
   +------+------+------+------+
   |  10  |  11  |  12  |SMOOTH|
   +------+------+------+------+
*/

#ifdef IR_REMOTE
#define ARILUX_IR_CODE_KEY_UP 0xFF906F
#define ARILUX_IR_CODE_KEY_DOWN 0xFFB847
#define ARILUX_IR_CODE_KEY_OFF 0xFFF807
#define ARILUX_IR_CODE_KEY_ON 0xFFB04F
#define ARILUX_IR_CODE_KEY_R 0xFF9867
#define ARILUX_IR_CODE_KEY_G 0xFFD827
#define ARILUX_IR_CODE_KEY_B 0xFF8877
#define ARILUX_IR_CODE_KEY_W 0xFFA857
#define ARILUX_IR_CODE_KEY_1 0xFFE817
#define ARILUX_IR_CODE_KEY_2 0xFF48B7
#define ARILUX_IR_CODE_KEY_3 0xFF6897
#define ARILUX_IR_CODE_KEY_FLASH 0xFFB24D
#define ARILUX_IR_CODE_KEY_4 0xFF02FD
#define ARILUX_IR_CODE_KEY_5 0xFF32CD
#define ARILUX_IR_CODE_KEY_6 0xFF20DF
#define ARILUX_IR_CODE_KEY_STROBE 0xFF00FF
#define ARILUX_IR_CODE_KEY_7 0xFF50AF
#define ARILUX_IR_CODE_KEY_8 0xFF7887
#define ARILUX_IR_CODE_KEY_9 0xFF708F
#define ARILUX_IR_CODE_KEY_FADE 0xFF58A7
#define ARILUX_IR_CODE_KEY_10 0xFF38C7
#define ARILUX_IR_CODE_KEY_11 0xFF28D7
#define ARILUX_IR_CODE_KEY_12 0xFFF00F
#define ARILUX_IR_CODE_KEY_SMOOTH 0xFF30CF
#endif

/*
   RF Remote
   Encoding: Chinese Protocol 1
   Codes provided by KmanOz (https://github.com/KmanOz)
   +--------+--------+--------+
   |   ON   | Toggle |   OFF  |
   +--------+--------+--------+
   | Speed+ | Mode+  | Bright+|
   +--------+--------+--------+
   | Speed- | Mode-  | Bright-|
   +--------+--------+--------+
   |  RED   | GREEN  |  BLUE  |
   +--------+--------+--------+
   | ORANGE | LT GRN | LT BLUE|
   +--------+--------+--------+
   | AMBER  |  CYAN  | PURPLE |
   +--------+--------+--------+
   | YELLOW |  PINK  | WHITE  |
   +--------+--------+--------+
*/

#ifdef RF_REMOTE
#define ARILUX_REMOTE_KEY_ON (REMOTE_CODE + 0x01)
#define ARILUX_REMOTE_KEY_TOGGLE (REMOTE_CODE + 0x02)
#define ARILUX_REMOTE_KEY_OFF (REMOTE_CODE + 0x03)
#define ARILUX_REMOTE_KEY_SPEED_PLUS (REMOTE_CODE + 0x04)
#define ARILUX_REMOTE_KEY_MODE_PLUS (REMOTE_CODE + 0x05)
#define ARILUX_REMOTE_KEY_BRIGHT_PLUS (REMOTE_CODE + 0x06)
#define ARILUX_REMOTE_KEY_SPEED_MINUS (REMOTE_CODE + 0x07)
#define ARILUX_REMOTE_KEY_MODE_MINUS (REMOTE_CODE + 0x08)
#define ARILUX_REMOTE_KEY_BRIGHT_MINUS (REMOTE_CODE + 0x09)
#define ARILUX_REMOTE_KEY_RED (REMOTE_CODE + 0x0A)
#define ARILUX_REMOTE_KEY_GREEN (REMOTE_CODE + 0x0B)
#define ARILUX_REMOTE_KEY_BLUE (REMOTE_CODE + 0x0C)
#define ARILUX_REMOTE_KEY_ORANGE (REMOTE_CODE + 0x0D)
#define ARILUX_REMOTE_KEY_LTGRN (REMOTE_CODE + 0x0E)
#define ARILUX_REMOTE_KEY_LTBLUE (REMOTE_CODE + 0x0F)
#define ARILUX_REMOTE_KEY_AMBER (REMOTE_CODE + 0x10)
#define ARILUX_REMOTE_KEY_CYAN (REMOTE_CODE + 0x11)
#define ARILUX_REMOTE_KEY_PURPLE (REMOTE_CODE + 0x12)
#define ARILUX_REMOTE_KEY_YELLOW (REMOTE_CODE + 0x13)
#define ARILUX_REMOTE_KEY_PINK (REMOTE_CODE + 0x14)
#define ARILUX_REMOTE_KEY_WHITE (REMOTE_CODE + 0x15)
#endif

class Arilux {
public:
    Arilux();
    bool init(void) const;
    bool setAll(const uint16_t p_red, const uint16_t p_green, const uint16_t p_blue, const uint16_t p_white1, const uint16_t p_white2) const;
    const char* getColorString(void) const;

private:
    const uint8_t m_redPin;
    const uint8_t m_greenPin;
    const uint8_t m_bluePin;
#if defined(RGBW) || defined(RGBWW)
    const uint8_t m_white1Pin;
#ifdef RGBWW
    const uint8_t m_white2Pin;
#endif
#endif
};
