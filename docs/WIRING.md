# Wiring reference

These are the connections used in the successful prototype reported in the chat. **Power off before rewiring.** Verify your specific board labels and module electrical requirements.

| Component pin | ESP32 pin | Notes |
|---|---|---|
| LCD VCC | VIN (USB-powered 5V) | Only for a display module rated for 5V supply and verified USB-powered VIN |
| LCD GND | GND | Common ground |
| LCD CS | GPIO 5 (D5) | TFT chip select |
| LCD RESET | GPIO 33 | Reset |
| LCD DC/RS | GPIO 27 | Data/command |
| LCD SDI/MOSI | GPIO 23 | SPI data |
| LCD SCK | GPIO 18 | SPI clock |
| LCD LED | 3V3 | **Only if the exact LCD module's LED pin is a 3.3V enable/control input** |
| LCD SDO/MISO | Not connected | Not needed for basic drawing |
| LCD touch pins (T_*) | Not connected | Touch not used |
| DHT11 S | GPIO 4 (D4) | Sensor signal |
| DHT11 + | 3V3 | Verify physical module pin order |
| DHT11 - | GND | Verify physical module pin order |
| Joystick VCC | 3V3 | Do not feed 5V into ESP32 ADC inputs |
| Joystick GND | GND | Common ground |
| Joystick VRx | GPIO 34 | Analog X |
| Joystick VRy | GPIO 35 | Analog Y, reserved for future use |
| Joystick SW | GPIO 32 | Button, active LOW |

## Shared breadboard rails

Connect ESP32 **3V3** to one positive rail and **GND** to a separate negative rail. DHT11 and joystick share these rails. LCD main VCC stays on VIN; **never bridge VIN and 3V3**. Some breadboards have split rails. Check continuity and avoid accidental shorts.

## Safety / reliability

A powered-on display test was successful, but this is not a measurement of regulator headroom or proof of 24/7 electrical safety. Check for overheating, resets, supply voltage drop, and ensure adequate USB supply. Avoid unattended continuous operation on a loose breadboard or unverified wiring.
