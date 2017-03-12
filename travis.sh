#!/bin/bash
echo "WRITING CONFIGURATION FILE"

echo -e "#define DEVICE_MODEL \"Travis\"\n$(cat config.example.h)" > config.h

echo "Wrote Travis device model to config.h"

if [ -z "$RGB_TYPE" ]; then
  RGB_TYPE="RGB"
fi

echo -e "#define $RGB_TYPE\n$(cat config.h)" > config.h

echo "Wrote #define $RGB_TYPE to config.h"

if [ -n "$REMOTE_TYPE" ]; then
  echo -e "#define $REMOTE_TYPE\n$(cat config.h)" > config.h
  echo "Wrote #define $REMOTE_TYPE to config.h"
fi

if [ -n "$MQTT_MODE" ]; then
  echo -e "#define $MQTT_MODE\n$(cat config.h)" > config.h
  echo "Wrote #define $MQTT_MODE to config.h"
fi

if [ -n "$MQTT_DISCOVERY" ]; then
  echo -e "#define $MQTT_DISCOVERY\n$(cat config.h)" > config.h
  echo "Wrote #define $MQTT_DISCOVERY to config.h"
fi

echo "FINISHED WRITING CONFIGURATION FILE"
