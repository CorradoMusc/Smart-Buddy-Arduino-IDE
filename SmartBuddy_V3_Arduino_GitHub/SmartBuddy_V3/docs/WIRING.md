# Verified prototype wiring reference

Same connections as the builder's previously working V1/V2 hardware. **Unplug USB before changing anything.**

```text
ESP32                              DWEII 4-inch ST7796S
VIN (USB 5V) ---------------------- VCC
GND ----------+------------------- GND
              |---- DHT11 -
              |---- JOYSTICK GND
3V3 ----------+------------------- LCD LED / backlight control*
              |---- DHT11 +
              |---- JOYSTICK VCC (+5V label, powered at 3.3V)
GPIO5  ---------------------------- CS
GPIO33 ---------------------------- RESET
GPIO27 ---------------------------- DC/RS
GPIO23 ---------------------------- SDI (MOSI)
GPIO18 ---------------------------- SCK
GPIO4  ---------------------------- DHT11 S
GPIO34 ---------------------------- Joystick VRx
GPIO35 ---------------------------- Joystick VRy
GPIO32 ---------------------------- Joystick SW
```

`*` This applies only to the board/backlight arrangement you already tested. Verify board version if you replace it. LCD SDO (MISO) and all `T_*` pins are unused.

- Use the breadboard power rails as distribution buses for **3V3** and **GND**; do not bridge them or bridge 3V3 to VIN.
- The ESP32's GPIO 34 and 35 are input-only ADC pins, appropriate for joystick analog outputs.
- The DHT11's common 3-pin module orientation was S / + / −; confirm actual hardware before rewiring other modules.
- Check all wiring, especially GPIO5 CS, GPIO27 DC, GPIO33 RESET, before applying power.
- Never connect 5V joystick analog outputs to ESP32 GPIO; power joystick from 3V3.
