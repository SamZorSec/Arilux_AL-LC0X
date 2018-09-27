#pragma once

#include "config.h"
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino

// in a terminal: telnet arilux.local
#if defined ARILUX_DEBUG_TELNET || defined DEBUG_SERIAL
#ifdef ARILUX_DEBUG_TELNET
extern WiFiServer telnetServer;
extern WiFiClient telnetClient;

#define DEBUG_PRINT(x) telnetClient.print(x)
#define DEBUG_PRINT_WITH_FMT(x, fmt) telnetClient.print(x, fmt)
#define DEBUG_PRINTLN(x) telnetClient.println(x)
#define DEBUG_PRINTLN_WITH_FMT(x, fmt) telnetClient.println(x, fmt)

void handleTelnet(void);
void startTelnet(void);

#else
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINT_WITH_FMT(x, fmt) Serial.print(x, fmt)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTLN_WITH_FMT(x, fmt) Serial.println(x, fmt)
#endif
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINT_WITH_FMT(x, fmt)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTLN_WITH_FMT(x, fmt)
#endif
