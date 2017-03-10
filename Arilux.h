#include "config.h"
#pragma once
#ifndef _ARILUX_H_
#define _ARILUX_H_

#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#if defined(RGBWW)
#define ARILUX_RED_PIN            5
#define ARILUX_GREEN_PIN          4
#define ARILUX_BLUE_PIN           14
#define ARILUX_WHITE1_PIN         12
#define ARILUX_WHITE2_PIN         13
#define ARILUX_COLOR_STRING       "RGBWW"
#elif defined(RGBW)
#define ARILUX_RED_PIN            5
#define ARILUX_GREEN_PIN          14
#define ARILUX_BLUE_PIN           12
#define ARILUX_WHITE1_PIN         13
#define ARILUX_COLOR_STRING       "RGBW"
#else
#define ARILUX_RED_PIN            14
#define ARILUX_GREEN_PIN          5
#define ARILUX_BLUE_PIN           12
#define ARILUX_COLOR_STRING       "RGB"
#endif

#define ARILUX_IR_PIN             4
#define ARILUX_RF_PIN             4

#define ARILUX_PWM_RANGE          255
#define ARILUX_PWM_FREQUENCY      500

#define ARILUX_BRIGHTNESS_STEP    25

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
#define ARILUX_IR_CODE_KEY_UP           0xFF906F
#define ARILUX_IR_CODE_KEY_DOWN         0xFFB847
#define ARILUX_IR_CODE_KEY_OFF          0xFFF807
#define ARILUX_IR_CODE_KEY_ON           0xFFB04F
#define ARILUX_IR_CODE_KEY_R            0xFF9867
#define ARILUX_IR_CODE_KEY_G            0xFFD827
#define ARILUX_IR_CODE_KEY_B            0xFF8877
#define ARILUX_IR_CODE_KEY_W            0xFFA857
#define ARILUX_IR_CODE_KEY_1            0xFFE817
#define ARILUX_IR_CODE_KEY_2            0xFF48B7
#define ARILUX_IR_CODE_KEY_3            0xFF6897
#define ARILUX_IR_CODE_KEY_FLASH        0xFFB24D
#define ARILUX_IR_CODE_KEY_4            0xFF02FD
#define ARILUX_IR_CODE_KEY_5            0xFF32CD
#define ARILUX_IR_CODE_KEY_6            0xFF20DF
#define ARILUX_IR_CODE_KEY_STROBE       0xFF00FF
#define ARILUX_IR_CODE_KEY_7            0xFF50AF
#define ARILUX_IR_CODE_KEY_8            0xFF7887
#define ARILUX_IR_CODE_KEY_9            0xFF708F
#define ARILUX_IR_CODE_KEY_FADE         0xFF58A7
#define ARILUX_IR_CODE_KEY_10           0xFF38C7
#define ARILUX_IR_CODE_KEY_11           0xFF28D7
#define ARILUX_IR_CODE_KEY_12           0xFFF00F
#define ARILUX_IR_CODE_KEY_SMOOTH       0xFF30CF
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
#define ARILUX_RF_CODE_KEY_ON           7808513
#define ARILUX_RF_CODE_KEY_TOGGLE       7808514
#define ARILUX_RF_CODE_KEY_OFF          7808515
#define ARILUX_RF_CODE_KEY_SPEED_PLUS   7808516
#define ARILUX_RF_CODE_KEY_MODE_PLUS    7808517
#define ARILUX_RF_CODE_KEY_BRIGHT_PLUS  7808518
#define ARILUX_RF_CODE_KEY_SPEED_MINUS  7808519
#define ARILUX_RF_CODE_KEY_MODE_MINUS   7808520
#define ARILUX_RF_CODE_KEY_BRIGHT_MINUS 7808521
#define ARILUX_RF_CODE_KEY_RED          7808522
#define ARILUX_RF_CODE_KEY_GREEN        7808523
#define ARILUX_RF_CODE_KEY_BLUE         7808524
#define ARILUX_RF_CODE_KEY_ORANGE       7808525
#define ARILUX_RF_CODE_KEY_LTGRN        7808526
#define ARILUX_RF_CODE_KEY_LTBLUE       7808527
#define ARILUX_RF_CODE_KEY_AMBER        7808528
#define ARILUX_RF_CODE_KEY_CYAN         7808529
#define ARILUX_RF_CODE_KEY_PURPLE       7808530
#define ARILUX_RF_CODE_KEY_YELLOW       7808531
#define ARILUX_RF_CODE_KEY_PINK         7808532
#define ARILUX_RF_CODE_KEY_WHITE        7808533
#endif

enum {
  ARILUX_CMD_NOT_DEFINED,
  ARILUX_CMD_STATE_CHANGED,
  ARILUX_CMD_BRIGHTNESS_CHANGED,
  ARILUX_CMD_COLOR_CHANGED,
  ARILUX_CMD_WHITE_CHANGED,
  ARILUX_CMD_PING
};

typedef struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white1;
  uint8_t white2;
};

class Arilux {
  public:
    Arilux(void);
    uint8_t init(void);
    uint8_t getState(void);
    uint8_t turnOn(void);
    uint8_t turnOff(void);
    uint8_t getBrightness(void);
    uint8_t increaseBrightness(void);
    uint8_t decreaseBrightness(void);
    uint8_t setBrightness(uint8_t p_brightness);
    uint8_t getRedValue(void);
    uint8_t getGreenValue(void);
    uint8_t getBlueValue(void);
    uint8_t getWhite1Value(void);
    uint8_t getWhite2Value(void);
    char * getColorString(void);

    uint8_t setColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue);
    uint8_t setAll(uint8_t p_red, uint8_t p_green, uint8_t p_blue, uint8_t p_white1, uint8_t p_white2);
    uint8_t setWhite(uint8_t p_white1, uint8_t p_white2);
  private:
    uint8_t m_redPin;
    uint8_t m_greenPin;
    uint8_t m_bluePin;
#if defined(RGBW) || defined (RGBWW)
    uint8_t m_white1Pin;
#ifdef RGBWW
    uint8_t m_white2Pin;
#endif
#endif
    uint8_t m_state;
    uint8_t m_brightness;
    Color   m_color;
    uint8_t setState(uint8_t p_state);
    uint8_t setColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue, uint8_t p_retain);
    uint8_t setAll(uint8_t p_red, uint8_t p_green, uint8_t p_blue, uint8_t p_white1, uint8_t p_white2, uint8_t p_retain);
    uint8_t setWhite(uint8_t p_white1, uint8_t p_white2, uint8_t p_retain);
};
#endif
