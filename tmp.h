void callback(char* p_topic, byte* p_payload, uint16_t p_length) {
    /*
        if (strcmp(mqttCommandTopic, p_topic) == 0) {
            // Handle the MQTT topic of the received message
            if (p_length > sizeof(jsonBuffer) - 1) {
                DEBUG_PRINTLN(F("MQTT Payload to large."));
                return;
            }

            // Mem copy into own buffer, I am not sure if p_payload is null terminated
            memcpy(jsonBuffer, p_payload, p_length);
            jsonBuffer[p_length] = 0;
            DEBUG_PRINT(F("MQTT Message received: "));
            DynamicJsonBuffer incomingJsonPayload;
            const JsonObject& root = incomingJsonPayload.parseObject(jsonBuffer);

            if (!root.success()) {
                DEBUG_PRINTLN(F("parseObject() failed"));
                return;
            }

            DEBUG_PRINTLN(jsonBuffer);

            // Load filters
            if (root.containsKey(FILTER)) {
                DEBUG_PRINT(F("Filter :"));
                // Get the name straight from the filter or use the object
                const bool isFilterNameOnly = root[FILTER].is<char*>();
                const char* filterName = isFilterNameOnly ? root[FILTER] : root[FILTER][FNAME];
                const JsonObject& filterRoot = isFilterNameOnly ? emptyJsonRoot : root[FILTER];

                // Set Filters
                if (strcmp(filterName, FILTER_NONE) == 0) {
                    DEBUG_PRINT(F(" " FILTER_NONE));
                    currentFilter.reset(new NoFilter());
                } else if (strcmp(filterName, FILTER_FADING) == 0) {
                    DEBUG_PRINT(F(" " FILTER_FADING " "));

                    if (filterRoot.containsKey(FALPHA)) {
                        const float alpha = filterRoot[FALPHA];
                        DEBUG_PRINT(alpha);

                        if (alpha > 0.001 && alpha < 1.0) {
                            currentFilter.reset(new FadingFilter(workingHsb, alpha));
                        } else {
                            DEBUG_PRINT(F(FALPHA " must be > 0.001 && < 1.0"));
                        }
                    } else {
                        DEBUG_PRINT(F(" "));
                        DEBUG_PRINT(FILTER_FADING_ALPHA);
                        currentFilter.reset(new FadingFilter(workingHsb, FILTER_FADING_ALPHA));
                    }
                } else {
                    DEBUG_PRINT(F(" "));
                    DEBUG_PRINT(filterName);
                    DEBUG_PRINT(F(" not found."));
                }

                DEBUG_PRINTLN(F(" done"));
            }

            // Load transitions
            if (root.containsKey(EFFECT)) {
                DEBUG_PRINT(F("Transition :"));
                const JsonObject& transitionRoot = root[EFFECT];

                if (!transitionRoot.containsKey(TNAME)) {
                    DEBUG_PRINTLN(F(" no name found."));
                    return;
                }

                const char* transitionName = transitionRoot[TNAME];
                workingHsb = currentEffect->finalState(transitionCounter, millis(), workingHsb);
                workingHsb = getNewColorState(workingHsb, root);
                const HSB transitionHSB = getNewColorState(workingHsb, transitionRoot);

                if (strcmp(transitionName, EFFECT_NONE) == 0) {
                    DEBUG_PRINT(F(" " EFFECT_NONE));
                    currentEffect.reset(new NoEffect());
                } else if (strcmp(transitionName, EFFECT_FLASH) == 0) {
                    DEBUG_PRINT(F(" " EFFECT_FLASH " "));
                    const uint8_t pulseWidth = transitionRoot.containsKey(TWIDTH) ? transitionRoot[TWIDTH] : FRAMES_PER_SECOND >> 1;
                    DEBUG_PRINT(pulseWidth);

                    if (transitionHSB == workingHsb) {
                        currentEffect.reset(new FlashEffect(workingHsb.toBuilder().brightness(0).build(),
                                                            transitionCounter, FRAMES_PER_SECOND, pulseWidth));
                    } else {
                        currentEffect.reset(new FlashEffect(transitionHSB,
                                                            transitionCounter, FRAMES_PER_SECOND, pulseWidth));
                    }
                } else if (strcmp(transitionName, EFFECT_RAINBOW) == 0) {
                    DEBUG_PRINT(F(" " EFFECT_RAINBOW));
                    currentEffect.reset(new RainbowEffect());
                } else if (strcmp(transitionName, EFFECT_FADE) == 0) {
                    DEBUG_PRINT(F(" " EFFECT_FADE " "));
                    const uint16_t timeMillis = transitionRoot.containsKey(TDURATION) ? transitionRoot[TDURATION] : 1000;
                    currentEffect.reset(new TransitionEffect(transitionHSB, millis(), timeMillis));
                    DEBUG_PRINT(timeMillis);
                } else {
                    DEBUG_PRINT(F(" "));
                    DEBUG_PRINT(transitionName);
                    DEBUG_PRINT(F(" Unknown"));
                }

                DEBUG_PRINT(F(" done"));
            } else {
                workingHsb = getNewColorState(workingHsb, root);
            }

            // ON/OFF are light turning the device ON
            // So we load the values from eeProm but we ensure we have a brightness > 0
            if (root.containsKey(STATE)) {
                const char* state = root[STATE];
                HSB colorState = getNewColorState(workingHsb, root);

                if (strcmp(state, SON) == 0) {
                    workingHsb = getOnState(colorState);
                } else if (strcmp(state, SOFF) == 0) {
                    currentEffect.reset(new NoEffect());
                    workingHsb = getOffState(colorState);
                }
            }

            // Base address of the remote control
            if (root.containsKey(REMOTECMD)) {
                eepromSettingsDTO.remote(root[REMOTECMD]);
            }

            // Load transitions
            if (root.containsKey(RESTARTCMD)) {
                ESP.restart();
            }

            // Force storing settings in eeprom
            if (root.containsKey(STORECMD)) {
                eepromStore.forceStorage(eepromSettingsDTO);
            }
        }
        */
}