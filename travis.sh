function build() {

  echo -e "#define DEVICE_MODEL \"Travis\"\n$(cat config.example.h)" > config.h

  if [ -n "$RGB_TYPE" ]; then
    echo -e "#define $RGB_TYPE\n$(cat config.h)" > config.h
  fi

  if [ -n "$REMOTE_TYPE" ]; then
    echo -e "#define $REMOTE_TYPE\n$(cat config.h)" > config.h
  fi

  if [ -n "$MQTT_MODE" ]; then
    echo -e "#define $MQTT_MODE\n$(cat config.h)" > config.h
  fi

  if [ -n "$MQTT_DISCOVERY" ]; then
    echo -e "#define $MQTT_DISCOVERY\n$(cat config.h)" > config.h
  fi

  # verify the ino, and save stdout & stderr to a variable
  # we have to avoid reading the exit code of local:
  # "when declaring a local variable in a function, the local acts as a command in its own right"
  local build_stdout
  build_stdout=$(arduino -v --verbose-build --verify --board esp8266:esp8266:generic Arilux_AL-LC0X.ino 2>&1)

  # echo output if the build failed
  if [ $? -ne 0 ]; then

    # heavy X
    echo -e "\xe2\x9c\x96"

    echo -e "----------------------------- DEBUG OUTPUT -----------------------------\n"
    echo "$build_stdout"
    echo -e "\n------------------------------------------------------------------------\n"

    # mark as fail
    exit_code=1

  else

    # heavy checkmark
    echo -e "\xe2\x9c\x93"

  fi

  return $exit_code

}
