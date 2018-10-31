[![Build Status](https://travis-ci.org/rvt/Arilux_AL-LC0X.svg?branch=master)](https://travis-ci.org/rvt/Arilux_AL-LC0X)

# LEDstrip firmware for esp8266 

This is an alternative version of the [Alternative firmware] for Arilux LED Controllers with a few modifications and enhancements.
This firmware has been tested with both OpenHAB and just a bit om Home Assistance.
- In OpenHAB both brightness, color ON/OFF will work because OpenHAB understands both HSB and RGB color model.
- In Home Assistant brightness and ON/OFF will work. Note: RGB to HSB model needs to be added to the example of Home Assistant, let me know if somebody is really interested. I don´ run HA myself so I have very limited knowledge.

Differences are between the original (from Martenats) firmware:
- Took the Martenats as my original to get into esp8266, but rewrote it mostly.

Enhancements are:
- Fade from any color to any other color smoothly without apparent brightness changes
- ON/OFF states will correctly fade and remember the last color (in EEPROM)
- Easy to make new effects, See Effect.h and some of the including Effects
- You can send partial updates for the color, for example just can just send he hue, brightness or white values
- After startup the LED will always turn on as a safety feature (handy if the arilux is behind a switch, mqtt down, wifi down etc..)
- Solid reconnect to mosquitto 

Current effects are:
- Rainbow: Will keep fading over the rainbow of colors
- Transition: Change from color1 to color2 over a period of time 
- Flash:  Flash between two colors or between black and the current color
- Strobe: Strobe between two colors, period can be given

Old functionality to be re-added
- IR Remote control

Ideas
- instead of using remote use a config topic to store specific configurations

## Remote Controle changes

- Remote control Speed + and - will change through colors
- Remote control Mode + and - changes saturation
- Remote control toggle stops the current effect


## Todo

 - Configure using some form of web based interface,ard eg, no need to configure for each device using the setup.h file
 - ASync TCP / MQTT client
 - Store and reload the current active Filter in EEPROM/MQTT so the device comes back up correctly after reboot
 - Check if we can use a better way of debugging lines
 - Check travis and see if that still works (never worked with it...sorry)
 
Tested with the [ESP8266 Wi-Fi chip][esp8266].


# Alternative firmware for Arilux LED controllers
This is an alternative firmware for Arilux LED controllers which uses [MQTT] instead of the default "Magic Home"/"Flux LED" protocol which has numerous reliability problems.
The LED controller is a cheap product available on sites like Banggood.com, Aliexpress, eBay and even Amazon which can be easily reprogrammed as it is based on the popular [ESP8266 Wi-Fi chip][esp8266].
The controllers are also known to sell under different manufacturer names such as "Firstd". If the product you bought looks similar to one of the Arilux controllers below, it most likely is.


**WARNING: DUE TO A NEW PINOUT, THIS FIRMWARE SEEMS TO BE NOT MORE COMPATIBLE WITH THE NEWEST MODELS (PCB version > 1.4).**


![Arilux](../images/Arilux.png)

## Features
- Remote control over the MQTT protocol via individual topics
- Supports transitions, flashing and other effects
- Remote control with the included IR control (uncomment `#define IR_REMOTE` in `config.h`)
- Remote control with the included RF control (uncomment `#define RF_REMOTE` in `config.h`)
- TLS support (uncomment `#define TLS` in `setup.h` and change the fingerprint if not using CloudMQTT)
- Debug printing over Telnet (add `#define ARILUX_DEBUG_TELNET` in `setup.h`)
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

[![Arilux AL-LC03 + IR + MQTT + Home Assistant](../images/Youtube.png)](https://www.youtube.com/watch?v=IKh0inaLvAU "Arilux AL-LC03 + IR + MQTT + Home Assistant")

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

![ESP-12F Layout](../images/ESP12-F_pinout3.jpg)

![Layout](../images/Layout.JPG)

### Option 2
Using the following image, connect RX, TX and GND of a single FTDI to the shown pins on the underside of the board. Plug in the wall power supply and flash using the above settings.
It helps to have another person able to plug in the device and start the upload while you hold the pins.

![Option 2 Layout](../images/option2.jpg)

## Updating
OTA is enabled on this firmware. Assuming the device is plugged in you should find the device as a Port option in the Arduino IDE. Make sure you are using the settings listed above.

## Control
### IR
TODO

### RF
The LED controller can be controlled with the RF remote included with the Arilux AL-LC09, AL-LC10 and AL-LC11. 
The `S+` and `S-` buttons can be used to change the color. The `M+` and `M-` buttons can be used to change the saturation. Brightness buttons work as usual.

You can pair a remote control with your arilux by pressing any button on the remote control after powering up
your arilux device. The pairing will be stored in EEPROM and send over MQTT.

### MQTT

## Filter vs Effect

### Effect
An effect will set the color, effects are usually based on timings like flashing leds, strobe or
slow fades. Only one effect can be active at a time.

### Filter
A filter will take the output of a effect and apply an additional transformation. THis can include
color correct and/or fading the HSB to create smooth transitions between colors. Only one filter
can be active at a time. NOTE: List of filter is on my todo..

### Boot Sequence
For savety reasons we want to turn the lights on regardless of any settings,
for this reason the firmware provides a specific boot sequence to guarantee this.

Bootsequence is done by prioritising where the initial light status come from and will insuring 
that during boot the lights will always be on with the last set hue and saturation value.


Bootorder:

* Load default settings
* Get HSB values from EEPROM and turn on LED with these values immediatly
* Subscribe state topic for two seconds, use any found settings over a period of two seconds
* Subscribe to command topic and overwrite any settings found in state over a period of two seconds

Considarations:

* After bootsequence the device will always be on in the last configured color setting
* If no wifi and/or mqtt server found we load up color from EEPROM 
* When EEPROM is empty we load up a brightness of 50
* When EEPROM has stored brightness of 0 we load up a brightness of minimum 5


#### Control 

1. We use individual topics to take advantage of retain and other features MQTT has to offer
   If you are have Home Assistant MQTT Discovery enabled, the `light.mqtt_json` platform will be loaded by Home Assistant instead of the `light.mqtt` platform.

   Topic: ``/color``
   | Name             | format                | Example                | Description                                                                                                                                                                              |
   |------------------|---------------------- | ---------------------- |-------------------------------------------------------------------------------------------|
   | `simple format`  | int,float,float       | 0,100,100              | Set Hue, Saturation and Brightness                                                        |
   | `hsb`            | hsb=int,float,float   | hsb=0,100,100          | Set Hue, Saturation and Brightnes with assignment                                         |
   | `hsb`            | hsb=int,float,float,float,float   | hsb=0,100,100,20,30          | Set Hue, Saturation and Brightness white1 and white 2 with assignment   |
   | `seperate`       | h=int s=float b=float w1=float w2=float | h=0 s=100 w1=25 w2=100 | Set as separate assignments  |
   | `combined`       | hsb=int,float,float b=float         | hsb=0,100,100 b=25     | Wil take brightness as 25  |

   ## Available Filters

   #### Disable Filtering
   Topic: ``/filter`` name=``none``

   Disable any filtering on the colors, color switch between current and new color.

   ##### Example
   ```
     mosquitto_pub -t "RGBW/001F162E/filter" -m 'name=none'
   ```

   #### Fade filter
   Topic: ``/filter`` name=``fading``

   Will smoothly fade between colors when a new color is set.
   This filter is implemented as fromValue + (toValue - fromValue) * m_alpha;
   
   | Parameter  | type     | default  | Description          |
   | ---------- | -------- | -------- | -------------------- |
   | alpha      | float    | 0.f5     | Speed of fading, keep this between 0.f01 and 0.99   |

   ##### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/filter" -m 'name=fading alpha=0.1'
   mosquitto_pub -t "RGBW/001F162E/filter" -m 'name=fading'
   ```
    
   ## Available effects 
   All effects require the `name` parameter.   
      
   #### none
   Topic: ``/effect`` name=``none``
  
   Turn of any running effect   
   ##### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=none'
   ```
   
   #### rainbow
   Topic: ``/effect`` name=``rainbow``
   
   Smoothly fades between all colors
   ##### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=rainbow'
   ```

   #### Flash
   Topic: ``/effect`` name=``flash``
   Flash or strobe between off/on or between two colors
   Note: For some effects you might want to turn the filter off, specially for strobe style effects.

   | Parameter  | type     | default  | Description          |
   | ---------- | -------- | -------- | -------------------- |
   | period     | int      | 50       | Total period measured in ticks. There are 50 ticks per second   |
   | pulse      | int      | 25       | Width of the on/color pulse measured in ticks    |
   | hsb        | hsb      |          | When a color is given we flash between this color and the current color insteadof off     |

   ##### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=flash'  # 50% duty cycle, on/off
   mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=flash pulse=1 b=100 s=0' # 2% duty cycle strobe to white once a second
    
   # two commands red short and blue longer 
   mosquitto_pub -t "RGBW/001F162E/color" -m 'hsb=0,100,100,0,0'
   mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=flash period=100 pulse=10 hsb=240,100,100 period=25'
   ```
  
   #### Fade
   Topic: ``/effect`` name=``fade``
   Gradually fade between two colors with a given duration

   | Parameter  | type     | default  | Description          |
   | ---------- | -------- | -------- | -------------------- |
   | duration   | long     |          | Total time in ms the fade will take   |

   ##### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=fade duration=15000 hsb=240,100,60' # Fade from current color to blue over 15 seconds
   ```

   ### Other things you can do

   #### Restart the device from mqtt
   Topic: ``/restart``

   Restart the device (handy for development)
   ##### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/restart" -m '1'
   ```

   #### Set base address of the remote control
   Topic: ``/remote``
   
   Set the base address of the remote control, value will be stored in EEPROM. Currently only tested with RF.
   You can also pair the remote control by pressing in the first few seconds a key on the remote control.
   The value will be stored in EEPROM and in mqtt.

   ##### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/remote" -m '10622464'
   ```

   #### Force storage of settings
   Topic: ``/store``

   See also EEPROM_COMMIT_WAIT_DELAY   
   ##### Example
   ```
   mosquitto_pub -t "RGBW/001F162E/store" -m '1'
   ```

#### Last Will and Testament

The firmware will publish a [MQTT Last Will and Testament] at `rgb(w/ww)/<chipid>/status`.
When the device successfully connects it will publish `online` to that topic and when it disconnects `offline` will automatically be published.

#### Home Assistance Discovery

##### Current working in progress with a issue publishing the discovery package 

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
  - platform: mqtt_template
    name: 'Arilux RGB Led Controller'
    command_topic: "RGB(W|WW)/<chipid>/color"
    state_topic: "RGBW(W|WW)/<chipid>/color/state"
    state_template: "{{value.split('state=')[1] | lower}}"
    command_on_template: "state=ON {%if brightness%}b={{brightness/2.55}}{%endif%}"
    command_off_template: "state=OFF"
    brightness_template: "{{(value.split('hsb=')[1].split(' ')[0].split(',')[2] | float *2.55) | int}}"
    availability_topic: "RGB(W|WW)/<chipid>/lastwill"
    payload_available: "online"
    payload_not_available: "offline"
```

#### Configuration with OpenHAB 2

You can use this device to connect to OpenHAB 2 with MQTT configured. Make sure MQTT is configured and working
and both OpenHAB and your light are connecting to the same MQTT broker.

The below config will also allow you to say to Siri when Homekit is configured on OpenHAB: 
 - ``Set Arilux to Red``
 - ``Set briightness of Arilux to fifty percent``
 - ``Turn off Arilux``
 - etc...

Most simple configuration would be as follows:
UPDATE: I have new scripts, let m know if I should post them

File: ``items/default.items``
```
Color  Item_Arilux_Color "Arilux" <light> ["Lighting"] {mqtt="<[mosquitto:RGB(W|WW)/<chipid>/color/state:state:JS(ariluxhsbToHsb.js)],>[mosquitto:RGB(W|WW)/<chipid>/color:command:*:default]"}
```
This will receive updates and send them to the correct mqtt topic.

A transformation to turns the device created payload to something OpenHAB can understand.
Essentially take out the HSB value from the color state and return that to OpenHAB.
File: ``transform/ariluxhsbToHsb.js``
```
(function(i){
    if (i.indexOf('OFF') !== -1) return 'OFF';
    regex = /([,.\d]+)/;
    m=null;
    if ((m = regex.exec(i)) !== null) {
        return m[0].split(',').slice(0,3).join(',');
    }
    return i;
})(input);
```

Adding a color picker to the default sitemap.
File: ``sitemaps/default.sitemap``
```
    Frame label="Arilux" {
        Colorpicker item=Item_Arilux_Color label="Color" icon="colorwheel"
    }
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
- [@mertenats]: Initial creator of the project, documention and code
- [@KmanOz]: Codes for the RF remote (Arilux AL-LC09)
- [@DanGunvald]: RGBW/RGBWW support
- [@robbiet480]: General cleanup and merging of RGBW/RGBWW code

*If you like the content of this repo, please add a star! Thank you!*

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
