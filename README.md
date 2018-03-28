[![Build Status](https://travis-ci.org/mertenats/Arilux_AL-LC0X.svg?branch=master)](https://travis-ci.org/mertenats/Arilux_AL-LC0X)

# Alternative - Alternative firmware for Arilux LED controllers

This is an alternative version of the [Alternative firmware] for Arilux LED Controllers with a few modifications and enhancements.

Differences are between the original Arilux alternative firmware:
- Only support for HSB
- Only support for JSON mqtt messages
- Almost complete rewrite of the code

Enhancements are:
- Fade from any color to any other color smoothly without apparent brightness changes
- ON/OFF states will correctly fade and remember the last color (in EEPROM)
- Easy to make new effects, See Effect.h and some of the including Effects
- You can send partial updates for the color, for example just the hue, brightness or white values
- After startup the LED will always turn on as a safety feature (handy if the arilux is behind a switch)
- Change: Remote control Speed + and - will change through colors

Current effects are:
- Rainbow: Will keep fading over the rainbow of colors
- Transition: Change from color1 to color2 over a period of time 
- Flash:  Flash between two colors or betwene black and the current color
- Strobe: Strobe between two colors, period can be given

Old functionality to be re-added
- IR Remote control

## Todo

 - IR Remote, need to add some better interface for this instead of separate routines for each remote control device
 - Configure using some form of web based interface, eg, no need to configure for each device using the setup.h file
 - ASync TCP / MQTT client
 - Store the current active Filter in EEPROM/MQTT (properly need to create some configure object)
 - CHeck if we can use a better way of debugging lines
 
Tested with the [ESP8266 Wi-Fi chip][esp8266].


# Alternative firmware for Arilux LED controllers
This is an alternative firmware for Arilux LED controllers which uses [MQTT] instead of the default "Magic Home"/"Flux LED" protocol which has numerous reliability problems.
The LED controller is a cheap product available on sites like Banggood.com, Aliexpress, eBay and even Amazon which can be easily reprogrammed as it is based on the popular [ESP8266 Wi-Fi chip][esp8266].
The controllers are also known to sell under different manufacturer names such as "Firstd". If the product you bought looks similar to one of the Arilux controllers below, it most likely is.


**WARNING: DUE TO A NEW PINOUT, THIS FIRMWARE SEEMS TO BE NOT MORE COMPATIBLE WITH THE NEWEST MODELS (PCB version > 1.4).**


![Arilux](images/Arilux.png)

## Features
- Remote control over the MQTT protocol via individual topics or JSON
- Supports transitions, flashing and other effects
- Remote control with the included IR control (uncomment `#define IR_REMOTE` in `config.h`)
- Remote control with the included RF control (uncomment `#define RF_REMOTE` in `config.h`)
- TLS support (uncomment `#define TLS` in `setup.h` and change the fingerprint if not using CloudMQTT)
- Debug printing over Telnet (add `#define DEBUG_TELNET` in `setup.h`)
- ArduinoOTA support for over-the-air firmware updates
- Native support for Home Assistant, including MQTT discovery (to be tested again).

## Supported devices
| Model | Color Support | Voltages | Remote | Price | Link                      |
|-------|---------------|----------|--------|-------|---------------------------|
| LC01  | RGB           | 5-28V    | None   | ~$8   | [Banggood][LC01-banggood] |
| LC02  | RGBW          | 9-12V    | None   | ~$11  | [Banggood][LC02-banggood] |
| LC03  | RGB           | 5-28V    | IR     | ~$12  | [Banggood][LC03-banggood] |
| LC04  | RGBW          | 9-12V    | IR     | ~$13  | [Banggood][LC04-banggood] |
| LC08  | RGBWW         | 5-28V    | None   | ~$12  | [Banggood][LC08-banggood] |
| LC09  | RGB           | 5-28V    | RF     | ~$12  | [Banggood][LC09-banggood] |
| LC10  | RGBW          | 9-28V    | RF     | ~$14  | [Banggood][LC10-banggood] |
| LC11  | RGBWW         | 9-28V    | RF     | ~$15  | [Banggood][LC11-banggood] |

## Demonstration

[![Arilux AL-LC03 + IR + MQTT + Home Assistant](images/Youtube.png)](https://www.youtube.com/watch?v=IKh0inaLvAU "Arilux AL-LC03 + IR + MQTT + Home Assistant")

## Flash the firmware
Whichever flashing option you choose, ensure your Arduino IDE settings match the following:

### Configuration
You must copy `setup.example.h` to `setup.h` and change settings to match your environment before flashing.
Within the config.h file there are more options you can override.

### Settings for the Arduino IDE

| Parameter       | Value                    |
| ----------------|--------------------------|
| Board           | Generic ESP8266 Module   |
| Flash Mode      | DIO                      |
| Flash Frequency | 40 MHz                   |
| Upload Using    | Serial                   |
| CPU Frequency   | 80 MHz                   |
| Flash Size      | 1M (64K SPIFFS)          |
| Reset Method    | ck                       |
| Upload Speed    | 115200                   |
| Port            | COMX, /dev/ttyUSB0, etc. |

*Note: If you own a board labeled 1.4 or the board isn't booting, use the DOUT mode instead of the DIO mode to flash the firmware*

### Option 1
#### Schematic
| Arilux | Left FTDI         | Right FTDI |
|--------|-------------------|------------|
| VCC    | VCC (set to 3.3V) |            |
| RX     |                   | TX         |
| TX     |                   | RX         |
| GPIO0  | GND               |            |
| GND    |                   | GND        |

Note: To enter in programming mode, you need to pull GPIO0 LOW while powering the board via the FTDI. It's not possible to reprogram the module without soldering the wire to the ESP8266 module.
If you are unable or don't know how to solder try option 2 below which can be accomplished without soldering.

The FTDI from the left gives power and it's connected to an USB charger (VCC, GND). The FTDI from the right is connected to the computer and is used to reprogram the ESP8266 (RX, TX, GND).

![ESP-12F Layout](images/ESP12-F_pinout3.jpg)

![Layout](images/Layout.JPG)

### Option 2
Using the following image, connect RX, TX and GND of a single FTDI to the shown pins on the underside of the board. Plug in the wall power supply and flash using the above settings.
It helps to have another person able to plug in the device and start the upload while you hold the pins.

![Option 2 Layout](images/option2.jpg)

## Updating
OTA is enabled on this firmware. Assuming the device is plugged in you should find the device as a Port option in the Arduino IDE. Make sure you are using the settings listed above.

## Control
### IR
The LED controller can be controlled with the IR remote included with the Arilux AL-LC03 and AL-LC04. The `Flash`, `Strobe`, `Fade` and `Smooth` functionalities are not yet implemented.

### RF
The LED controller can be controlled with the RF remote included with the Arilux AL-LC09, AL-LC10 and AL-LC11. The `Mode+`, `Mode-`, `Speed+`, `Speed-` and `toggle` functionalities are not yet implemented.

### MQTT

## Filter vs Effect
What.s the difference in this firmware

### Effect
An effect will set the color, effects are usually based on timings like flashing leds, strobe or
slow fades. Only one effect can be active at a time.

### Filter
A filter will take the output of a effect and apply an additional transformation. THis can include
color correct and/or fading the HSB to create smooth transitions between colors. Only one filter
can be active at a time. NOTE: List of filter is on my todo..


#### Control 

1. Only one topic will be published and subscribed to (as well as the Last Will and Testament topic). 
   JSON is great because it reduces roundtrips across the network. 
   If you are have Home Assistant MQTT Discovery enabled, the `light.mqtt_json` platform will be loaded by Home Assistant instead of the `light.mqtt` platform.

   ##### HSB properties
   | Name          | Data Type                                | Example | Description                                                                                                                                                                              |
   |---------------|------------------------------------------|---------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
   | `hsb`       | Object/dictionary                        | `{}`    | A dictionary with the below RGB values                                                                                                                                                   |
   | `hsb.h`     | Integer, 0-359                           | `120`   | Hue of the color                                                                                                                                                                |
   | `hsb.s`     | Integer, 0-255                           | `255`   | Saturation of the color                                                                                                                                                             |
   | `hsb.b`     | Integer, 0-255                           | `255`   | Brightness of the color                                                                                                                                                             |
   | `w1` | Integer, 0-255                           | `255`   | Controls the whiteness level of the lights. Only supported for RGBW.|
   | `w2` | Integer, 0-255                           | `255`   | Controls the whiteness level of the lights. Only supported for RGBWW.|
            
 
   ##### Filter properties
   | Name          | Data Type                                | Example | Description                                                                                                                                                                              |
   |---------------|------------------------------------------|---------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
   | `filter`       | Object/dictionary  or String            | `{}` or `fade`    | A dictionary or string for simple filters                                                                                                                                                |
   | `filter.name`     | Integer, 0-359                       | `fade`   | Name of the filter to activate                                                                                                                                                               |
   | `filter.XXX`     | properries                           |    | Properties                                                                                                                                              |

   ##### Effect properties
   | Name          | Data Type                                | Example | Description                                                                                                                                                                              |
   |---------------|------------------------------------------|---------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
   | `effect`       | Object/dictionary                      | `{}`  | A dictionary with effect config                                                                                                                                               |
   | `effect.name`     | Integer, 0-359                       | `rainbow`   | Name of the effect to activate                                                                                                                                                               |
   | `effect.XXX`     | properries                           |    | Properties                                                                                                                                              |


   ## State
   ```
     "state": "ON" 
   ```
   #### Example
   ```
     # Turn device on
     mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"state":"ON"}'
     # Turn device off
     mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"state":"OFF"}'
     # Turn device off and remove current effect
     mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"state":"OFF", "effect":{"name":"none"}}'
   ```
   Note: If you expect active transitions, it´ always a good idea to turn them off
   state ON/OFF is implemented as a birghtness of 0 or previous brightness
   soo effects might override this command.

   ## Available Filters

   ### Disable Filtering
   Disable any filtering on the colors, color switch between current and new color.
   ```
     "filter": {
       "name":"none" 
     }
     or
     "filter": "none"
   ```

   #### Example
   ```
     mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"filter":"none"}'
   ```

   ### fade filter
   Will smoothly fade between colors when a new color is set.
   ```
     "filter": {
       "name":"fade",
       "alpha":0.05   // How fast the colors will fade, the lower the value the longer it takes to fade between colors
     }
     or
     "filter": "fade" // Set a 
   ```
   #### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"filter":{"name":"fading", "alpha":0.02}}'
   mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"filter":"fading"}'
   ```
    
   ## Available effects 
   
   
   ### none
   Turn of any effect
   ```
     "effect": {
       "name":"none" 
     }
   ```
   
   ### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"effect":{"name":"none"}}}'
   ```
   
   ### rainbow
   Smoothly changes through all colors
   ```
     "effect": {
       "name":"rainbow" 
     }
   ```

   ### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"effect":{"name":"rainbow"}}}'
   ```

   ### Flash
   Change between two colors
   ```
     "effect": {
       "name":"flash",
       "width":2         // Width of the pulse (00..25)
     }
   ```
   ### Example
   ```
   # flash between current color and off
   mosquitto_pub  -t "RGBW/001F162E/json/set" -m '{"effect":{"name":flash}}}'
   # Flash between two colors red and green
   mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"hsb":{"h":0},"effect":{"name":flash,"hsb":{"h":180}}}'
   # Stobe light
   mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"hsb":{"h":0},"w1":0,"effect":{"name":flash,"width":1,"w1":255,"hsb":{"b":255,"s":0}}}'
   ```

   ### Fade
   Gradually fade between two colors in a custom time
   ```
     "effect": {
       "name":"fade",
       "duration":20000         // Duration in millis of the total transition
     }
   ```
   ### Example
   ```
   # Fade to 0 in 10 seconds
   mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"effect":{"name":fade,"duration":10000,"hsb":{"b":0}}}'
   # Fade from red to green with a little bit of white in 15 seconds
   mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"hsb":{"h":0,"b":25},"effect":{"name":fade,"duration":10000,"hsb":{"h":"120", "w1":10}}}'
   # Fade from current color to blue
   mosquitto_pub -u admin -P admin1234 -t "RGBW/001F162E/json/set" -m '{"effect":{"name":fade,"duration":10000,"hsb":{"h":240}}}'
   ```

   ### Other things you can do
   Restart the device from mqtt
   ```
     "restart": 1
   ```
   ### Example
   ```
   # Fade to 0 in 10 seconds
   mosquitto_pub -t "RGBW/001F162E/json/set" -m '{"restart":1}'
   ```

#### Last Will and Testament

The firmware will publish a [MQTT Last Will and Testament] at `rgb(w/ww)/<chipid>/status`.
When the device successfully connects it will publish `alive` to that topic and when it disconnects `dead` will automatically be published.

#### Discovery

This firmware supports [Home Assistant's MQTT discovery functionality], added in 0.40.
This allows for instant setup and use of your device without requiring any manual configuration in Home Assistant.
If you are using the MQTT JSON mode, the `light.mqtt_json` platform will be loaded. Otherwise, the `light.mqtt` platform will load. `light.mqtt_json` is required for full functionality.
There are a few one time steps that you need to take to get this working.

1. Add `discovery: true` to your `mqtt` configuration in Home Assistant, if it isn't there already.
2. Uncomment the `HOME_ASSISTANT_MQTT_DISCOVERY` definitions in your `config.h` file.
  - You can change the discovery prefix (default is `homeassistant`) by changing `HOME_ASSISTANT_MQTT_DISCOVERY_PREFIX`.
    Make sure this matches your Home Assistant MQTT configuration.
3. Upload the firmware once more after making the previous changes.

From now on your device will announce itself to Home Assistant with all of the proper configuration information.

### Configuration for Home Assistant
configuration.yaml
```yaml
mqtt:
  broker: 'm21.cloudmqtt.com'
  username: '[REDACTED]'
  password: '[REDACTED]'
  port: '[REDACTED]'
  discovery: true

light:
  - platform: mqtt
    name: 'Arilux RGB Led Controller'
    state_topic: 'rgb(w/ww)/<chipid>/state/state'
    command_topic: 'rgb(w/ww)/<chipid>/state/set'
    brightness_state_topic: 'rgb(w/ww)/<chipid>/brightness/state'
    brightness_command_topic: 'rgb(w/ww)/<chipid>/brightness/set'
    rgb_state_topic: 'rgb(w/ww)/<chipid>/color/state'
    rgb_command_topic: 'rgb(w/ww)/<chipid>/color/set'
```


## Licence
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

## Home Assistant Community Discussion Forum
For further information and to join the discussion for this firmware [check out this thread] on the Home Assistant Community Discussion Forum.

## Contributors
- [@KmanOz]: Codes for the RF remote (Arilux AL-LC09)
- [@DanGunvald]: RGBW/RGBWW support
- [@robbiet480]: General cleanup and merging of RGBW/RGBWW code

*If you like the content of this repo, please add a star! Thank you!*

[ArduionJson]: https://github.com/bblanchon/ArduinoJson
[@KmanOz]: https://github.com/KmanOz
[@DanGunvald]: https://github.com/DanGunvald
[@robbiet480]: https://github.com/robbiet480
[MQTT Last Will and Testament]: http://www.hivemq.com/blog/mqtt-essentials-part-9-last-will-and-testament
[LC01-banggood]: http://www.banggood.com/ARILUX-AL-LC01-Super-Mini-LED-WIFI-Smart-RGB-Controller-For-RGB-LED-Strip-Light-DC-9-12V-p-1058603.html?rmmds=search
[LC02-banggood]: http://www.banggood.com/ARILUX-AL-LC02-Super-Mini-LED-WIFI-APP-Controller-Dimmer-for-RGBW-LED-Strip-Light-DC-9-12V-p-1060222.html
[LC03-banggood]: http://www.banggood.com/ARILUX-AL-LC03-Super-Mini-LED-WIFI-APP-Controller-Remote-Control-For-RGB-LED-Strip-DC-9-12V-p-1060223.html
[LC04-banggood]: http://www.banggood.com/ARILUX-AL-LC04-Super-Mini-LED-WIFI-APP-Controller-Remote-Control-For-RGBW-LED-Strip-DC-9-12V-p-1060231.html
[LC08-banggood]: http://www.banggood.com/ARILUX-AL-LC08-Super-Mini-LED-WIFI-APP-Controller-Dimmer-for-RGBWW-LED-Strip-Light-DC-5-28V-p-1081241.html
[LC09-banggood]: http://www.banggood.com/ARILUX-AL-LC09-Super-Mini-LED-WIFI-APP-Controller-RF-Remote-Control-For-RGB-LED-Strip-DC9-28V-p-1081344.html
[LC10-banggood]: http://www.banggood.com/ARILUX-AL-LC10-Super-Mini-LED-WIFI-APP-Controller-RF-Remote-Control-For-RGBW-LED-Strip-DC9-28V-p-1085111.html
[LC11-banggood]: http://www.banggood.com/ARILUX-AL-LC11-Super-Mini-LED-WIFI-APP-Controller-RF-Remote-Control-For-RGBWW-LED-Strip-DC9-28V-p-1085112.html
[esp8266]: https://en.wikipedia.org/wiki/ESP8266
[Home Assistant's MQTT discovery functionality]: https://home-assistant.io/docs/mqtt/discovery/
[check out this thread]: https://community.home-assistant.io/t/alternative-firmware-for-arilux-al-lc03-for-use-with-mqtt-and-home-assistant-rgb-light-strip-controller/6328/16
[MQTT]: http://mqtt.org/
[Alternative firmware]: https://github.com/mertenats/Arilux_AL-LC0X 
