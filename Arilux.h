#pragma once
#ifndef _ARILUX_H_
#define _ARILUX_H_

#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino

#define ARILUX_RED_PIN            14
#define ARILUX_GREEN_PIN          5
#define ARILUX_BLUE_PIN           12
#define ARILUX_IR_PIN             4

#define ARILUX_PWM_RANGE          255
#define ARILUX_PWM_FREQUENCY      500

#define ARILUX_BRIGHTNESS_STEP    25

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
#define ARILUX_IR_CODE_KEY_UP     0xFF906F
#define ARILUX_IR_CODE_KEY_DOWN   0xFFB847
#define ARILUX_IR_CODE_KEY_OFF    0xFFF807
#define ARILUX_IR_CODE_KEY_ON     0xFFB04F
#define ARILUX_IR_CODE_KEY_R      0xFF9867
#define ARILUX_IR_CODE_KEY_G      0xFFD827
#define ARILUX_IR_CODE_KEY_B      0xFF8877
#define ARILUX_IR_CODE_KEY_W      0xFFA857
#define ARILUX_IR_CODE_KEY_1      0xFFE817
#define ARILUX_IR_CODE_KEY_2      0xFF48B7
#define ARILUX_IR_CODE_KEY_3      0xFF6897
#define ARILUX_IR_CODE_KEY_FLASH  0xFFB24D
#define ARILUX_IR_CODE_KEY_4      0xFF02FD
#define ARILUX_IR_CODE_KEY_5      0xFF32CD
#define ARILUX_IR_CODE_KEY_6      0xFF20DF
#define ARILUX_IR_CODE_KEY_STROBE 0xFF00FF
#define ARILUX_IR_CODE_KEY_7      0xFF50AF
#define ARILUX_IR_CODE_KEY_8      0xFF7887
#define ARILUX_IR_CODE_KEY_9      0xFF708F
#define ARILUX_IR_CODE_KEY_FADE   0xFF58A7
#define ARILUX_IR_CODE_KEY_10     0xFF38C7
#define ARILUX_IR_CODE_KEY_11     0xFF28D7
#define ARILUX_IR_CODE_KEY_12     0xFFF00F
#define ARILUX_IR_CODE_KEY_SMOOTH 0xFF30CF
#endif

enum {
  ARILUX_CMD_NOT_DEFINED,
  ARILUX_CMD_STATE_CHANGED,
  ARILUX_CMD_BRIGHTNESS_CHANGED,
  ARILUX_CMD_COLOR_CHANGED
};

typedef struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
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
    uint8_t setColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue);
  private:
    uint8_t m_redPin;
    uint8_t m_greenPin;
    uint8_t m_bluePin;
    uint8_t m_state;
    uint8_t m_brightness;
    Color   m_color;
    uint8_t setState(uint8_t p_state);
    uint8_t setColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue, uint8_t p_retain);
};
#endif
