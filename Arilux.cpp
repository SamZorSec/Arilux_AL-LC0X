#include "Arilux.h"

Arilux::Arilux(void):
  m_redPin(ARILUX_RED_PIN),
  m_greenPin(ARILUX_GREEN_PIN),
  m_bluePin(ARILUX_BLUE_PIN)
#if defined(RGBW) || defined (RGBWW)
  , m_white1Pin(ARILUX_WHITE1_PIN)
#ifdef RGBWW
  , m_white2Pin(ARILUX_WHITE2_PIN)
#endif
#endif
{
  m_state = false;
  m_brightness = ARILUX_PWM_RANGE / 3;
  m_color.red = ARILUX_PWM_RANGE;
  m_color.green = ARILUX_PWM_RANGE;
  m_color.blue = ARILUX_PWM_RANGE;
  m_color.white1 = ARILUX_PWM_RANGE;
  m_color.white2 = ARILUX_PWM_RANGE;
}

uint8_t Arilux::init(void) {
  pinMode(m_redPin, OUTPUT);
  pinMode(m_greenPin, OUTPUT);
  pinMode(m_bluePin, OUTPUT);

#if defined(RGBW) || defined (RGBWW)
  pinMode(m_white1Pin, OUTPUT);
#ifdef RGBWW
  pinMode(m_white2Pin, OUTPUT);
#endif
#endif
  analogWriteFreq(ARILUX_PWM_FREQUENCY);
  analogWriteRange(ARILUX_PWM_RANGE);

  return true;
}

uint8_t Arilux::getState(void) {
  return m_state;
}

uint8_t Arilux::turnOn(void) {
  return setState(true);
}

uint8_t Arilux::turnOff(void) {
  return setState(false);
}

char *Arilux::getColorString(void) {
  return (char *)ARILUX_COLOR_STRING;
}

uint8_t Arilux::setState(uint8_t p_state) {
  if (p_state == true) {
    m_state = true;
    return setAll(m_color.red, m_color.green, m_color.blue, m_color.white1, m_color.white2, false);
  } else {
    m_state = false;
    return setAll(0, 0, 0, 0, 0, false);
  }
}

uint8_t Arilux::getBrightness(void) {
  return m_brightness;
}

uint8_t Arilux::increaseBrightness(void) {
  if (!m_state)
    return false;
  if ((m_brightness + ARILUX_BRIGHTNESS_STEP) >= ARILUX_PWM_RANGE)
    return setBrightness(ARILUX_PWM_RANGE);  
  if (m_brightness < (ARILUX_PWM_RANGE - ARILUX_BRIGHTNESS_STEP))
    return setBrightness(m_brightness + ARILUX_BRIGHTNESS_STEP);
  return false;
}

uint8_t Arilux::decreaseBrightness(void) {
  if (!m_state)
    return false;

  if (m_brightness > (1 + ARILUX_BRIGHTNESS_STEP))
    return setBrightness(m_brightness - ARILUX_BRIGHTNESS_STEP);
  return false;
}

uint8_t Arilux::setBrightness(uint8_t p_brightness) {
  if (p_brightness < 0 || p_brightness > ARILUX_PWM_RANGE)
    return false;

  m_brightness = p_brightness;

  analogWrite(m_redPin, map(m_color.red, 0, ARILUX_PWM_RANGE, 0, m_brightness));
  analogWrite(m_greenPin, map(m_color.green, 0, ARILUX_PWM_RANGE, 0, m_brightness));
  analogWrite(m_bluePin, map(m_color.blue, 0, ARILUX_PWM_RANGE, 0, m_brightness));
#if defined(RGBW) || defined (RGBWW)
  analogWrite(m_white1Pin, map(m_color.white1, 0, ARILUX_PWM_RANGE, 0, m_brightness));
#ifdef RGBWW
  analogWrite(m_white2Pin, map(m_color.white2, 0, ARILUX_PWM_RANGE, 0, m_brightness));
#endif
#endif
  return true;
}

uint8_t Arilux::getRedValue(void) {
  return m_color.red;
}

uint8_t Arilux::getGreenValue(void) {
  return m_color.green;
}

uint8_t Arilux::getBlueValue(void) {
  return m_color.blue;
}

uint8_t Arilux::getWhite1Value(void) {
  return m_color.white1;
}

uint8_t Arilux::getWhite2Value(void) {
  return m_color.white2;
}

uint8_t Arilux::setAll(uint8_t p_red, uint8_t p_green, uint8_t p_blue, uint8_t p_white1, uint8_t p_white2) {
  if (!m_state)
    return false;

  return setAll(p_red, p_green, p_blue, p_white1, p_white2, true);
}

uint8_t Arilux::setColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue) {
  if (!m_state)
    m_state = true;
  return setAll(p_red, p_green, p_blue, getWhite1Value(), getWhite2Value(), true);
}

uint8_t Arilux::setFadeToColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue) {
    m_color.red = p_red;
    m_color.green = p_green;
    m_color.blue = p_blue;
}

uint8_t Arilux::setFadeColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue) {
  if (!m_state) {
    m_state = true;
  }
  return setAll(p_red, p_green, p_blue, getWhite1Value(), getWhite2Value(), false);
}

uint8_t Arilux::setWhite(uint8_t p_white1, uint8_t p_white2) {
  return setAll(getRedValue(), getGreenValue(), getBlueValue(), p_white1, p_white2, true);
}

uint8_t Arilux::setAll(uint8_t p_red, uint8_t p_green, uint8_t p_blue, uint8_t p_white1, uint8_t p_white2, uint8_t p_retain) {
  if ((p_red < 0 || p_red > ARILUX_PWM_RANGE) || (p_green < 0 || p_green > ARILUX_PWM_RANGE) || (p_blue < 0 || p_blue > ARILUX_PWM_RANGE || p_white1 < 0 || p_white1 > ARILUX_PWM_RANGE || p_white2 < 0 || p_white2 > ARILUX_PWM_RANGE))
    return false;

  if (p_retain) {
    m_color.red = p_red;
    m_color.green = p_green;
    m_color.blue = p_blue;
    m_color.white1 = p_white1;
    m_color.white2 = p_white2;
  }

  analogWrite(m_redPin, map(p_red, 0, ARILUX_PWM_RANGE, 0, m_brightness));
  analogWrite(m_greenPin, map(p_green, 0, ARILUX_PWM_RANGE, 0, m_brightness));
  analogWrite(m_bluePin, map(p_blue, 0, ARILUX_PWM_RANGE, 0, m_brightness));
#if defined(RGBW) || defined (RGBWW)
  analogWrite(m_white1Pin, map(p_white1, 0, ARILUX_PWM_RANGE, 0, m_brightness));
#ifdef RGBWW
  analogWrite(m_white2Pin, map(p_white2, 0, ARILUX_PWM_RANGE, 0, m_brightness));
#endif
#endif
  return true;
}
uint8_t Arilux::setColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue, uint8_t p_retain) {
  return setAll(p_red, p_green, p_blue, getWhite1Value(), getWhite2Value(), p_retain);
}
uint8_t Arilux::setWhite( uint8_t p_white1, uint8_t p_white2, uint8_t p_retain) {
  return setAll(getRedValue(), getGreenValue(), getBlueValue(), p_white1, p_white2, p_retain);
}

