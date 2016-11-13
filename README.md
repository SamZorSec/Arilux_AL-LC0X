# Arilux_AL-LC03
Alternative firmware for Arilux AL-LC03 LED controllers, based on the MQTT protocol.

## Features
- Remote control over the MQTT protocol
- Remote Control over IR codes

## MQTT topics
### State
| #          | Topic                 | Payload   |
| -----------|-----------------------|-----------|
| State      | `arilux/state/state`  | `ON`/`OFF`|
| Command    | `arilux/state/set`    | `ON`/`OFF`|

### Brightness
| #          | Topic                      | Payload   |
| -----------|----------------------------|-----------|
| State      | `arilux/brightness/state`  |  `0-255`  |
| Command    | `arilux/brightness/set`    |  `0-255`  |

### Color
| #          | Topic                 | Payload             |
| -----------|-----------------------|---------------------|
| State      | `arilux/color/state`  | `0-255,0-255,0-255` |
| Command    | `arilux/color/set`    | `0-255,0-255,0-255` |
