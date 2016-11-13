#include "Arilux.h"

Arilux::Arilux(void):
  m_redPin(ARILUX_RED_PIN),
  m_greenPin(ARILUX_GREEN_PIN),
  m_bluePin(ARILUX_BLUE_PIN)
{
  m_state = false;
  m_brightness = ARILUX_PWM_RANGE;
  m_color.red = ARILUX_PWM_RANGE;
  m_color.green = ARILUX_PWM_RANGE;
  m_color.blue = ARILUX_PWM_RANGE;
}

uint8_t Arilux::init(void) {
  pinMode(m_redPin, OUTPUT);
  pinMode(m_greenPin, OUTPUT);
  pinMode(m_bluePin, OUTPUT);

  analogWriteFreq(ARILUX_PWM_FREQUENCY);
  analogWriteRange(ARILUX_PWM_RANGE);

  if (!turnOn())
    return false;

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

uint8_t Arilux::setState(uint8_t p_state) {
  if (p_state == true && m_state == false) {
    m_state = true;
    return setColor(m_color.red, m_color.green, m_color.blue, false);
  } else if (p_state == false && m_state == true) {
    m_state = false;
    return setColor(0, 0, 0, false);
  } else {
    return false;
  }
}

uint8_t Arilux::getBrightness(void) {
  return m_brightness;
}

uint8_t Arilux::increaseBrightness(void) {
  if (!m_state)
    return false;
    
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

uint8_t Arilux::setColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue) {
  if (!m_state)
    return false;
    
  return setColor(p_red, p_green, p_blue, true);
}

uint8_t Arilux::setColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue, uint8_t p_retain) {
  if ((p_red < 0 || p_red > ARILUX_PWM_RANGE) || (p_green < 0 || p_green > ARILUX_PWM_RANGE) || (p_blue < 0 || p_blue > ARILUX_PWM_RANGE))
    return false;

  if (p_retain) {
    m_color.red = p_red;
    m_color.green = p_green;
    m_color.blue = p_blue;
  }

  analogWrite(m_redPin, map(p_red, 0, ARILUX_PWM_RANGE, 0, m_brightness));
  analogWrite(m_greenPin, map(p_green, 0, ARILUX_PWM_RANGE, 0, m_brightness));
  analogWrite(m_bluePin, map(p_blue, 0, ARILUX_PWM_RANGE, 0, m_brightness));

  return true;
}
