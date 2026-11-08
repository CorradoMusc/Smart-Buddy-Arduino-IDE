# Smart Buddy V3 · ESP32 Desktop Companion

An ESP32-powered desk assistant with a polished **480×320 TFT interface**, real weather, automatic time, room sensing, focus timer, joystick navigation and an animated idle robot.

> **Project status:** V1 hardware and core features were confirmed working by the builder. **This V3 firmware has been newly authored and still needs compiling, uploading and testing on the actual board.** Do not describe V3 as field-tested until you've run the checklist.

## Features

- **Home:** clock, date, outdoor and indoor mini-dashboard, small Buddy portrait
- **Weather:** vector icon, outside temp, feels-like, humidity, wind, data age/stale flag
- **Room:** DHT11 temperature, humidity, approximate comfort indicator
- **Focus:** 25-minute elapsed-time-based countdown, pause/start on joystick click, progress bar
- **Idle Buddy:** after 60 seconds, automatic big robot, 4–8-second blinks, subtle bobbing, sleepy and focused states; a joystick gesture wakes it without accidentally navigating
- **Navigation transition:** quick nonblocking cyan accent sweep during screen switches
- **Reliability:** Wi-Fi retry, background HTTPS weather task, keep last valid readings, automatic Ontario DST time via NTP, page-only full clears and small sprites for live data
- **Security:** server certificate validation is enabled using the included ISRG Root X1 CA; no `setInsecure()` bypass

## Hardware / pin map

| Module | Pin | ESP32 |
| --- | --- | --- |
| ST7796S LCD | VCC | VIN / USB 5V (previously tested wiring) |
| ST7796S LCD | GND | GND |
| ST7796S LCD | LED | 3V3 (matching-module backlight enable only) |
| ST7796S LCD | CS | GPIO5 |
| ST7796S LCD | RESET | GPIO33 |
| ST7796S LCD | DC/RS | GPIO27 |
| ST7796S LCD | SDI/MOSI | GPIO23 |
| ST7796S LCD | SCK | GPIO18 |
| ST7796S LCD | SDO / all T_ touch pins | Not connected |
| DHT11 | S | GPIO4 |
| DHT11 | + | 3V3 |
| DHT11 | - | GND |
| Joystick | VRx | GPIO34 |
| Joystick | VRy | GPIO35 (reserved / not yet used for menus) |
| Joystick | SW | GPIO32 |
| Joystick | VCC | 3V3, **not 5V** |
| Joystick | GND | GND |

**Power precautions:** Unplug before rewiring. Ensure LCD VCC is not also tied to the 3V3 rail. 5V must never enter ESP32 GPIO. Screen VCC and backlight assignments are specific to the already working DWEII board; verify against board markings if hardware changes. For permanent use, secure loose Dupont wires, check power/current and prevent shorts before putting it in an enclosure.

## Install / upload (Arduino IDE 2)

1. Download/extract this repository. Keep **all `.cpp` and `.h` files beside `SmartBuddy_V3.ino`** in the `SmartBuddy_V3` sketch folder.
2. In **Boards Manager**, install `esp32 by Espressif Systems` and choose the same **ESP32 Dev Module** and COM port that worked for V1/V2.
3. In **Library Manager**, install:
   - `TFT_eSPI` by Bodmer
   - `ArduinoJson` by Benoit Blanchon (**v7.x**)
   - `DHT sensor library` by Adafruit
   - `Adafruit Unified Sensor` by Adafruit
4. Keep your **already working TFT_eSPI User_Setup.h** with `ST7796_DRIVER`, MOSI=23, SCLK=18, CS=5, DC=27, RST=33, `SPI_FREQUENCY=20000000`, fonts 2, 4 and 7 enabled. A reference is in `config/User_Setup_SmartBuddy.h`. **Don't activate two setups at once.**
5. Copy `Secrets.h.example` to a new file named `Secrets.h` in the sketch folder. Enter your **own Wi-Fi SSID and password**. Do not share or commit that file. `.gitignore` already excludes it.
6. Optionally change coordinates and `LOCATION_NAME` inside `Config.h`. By default they're for Markham, Ontario. The time zone is `America/Toronto` equivalent (EST/EDT DST rules).
7. Open `SmartBuddy_V3.ino`, select the ESP32 board/port and click **Upload**. Open **Serial Monitor at 115200 baud** to inspect startup, sprite memory and weather messages.
8. When the screen appears, test: joystick left/right (recenter each time), click on Focus page, idle after 60 seconds, motion/press wake, weather update and sensor readings.

### If something fails

- **Blank/white screen:** Restore the previously working `TFT_eSPI` configuration and double-check TFT wiring while USB is unplugged.
- **Compile error `Secrets.h: No such file`:** You didn't copy `Secrets.h.example` to `Secrets.h`.
- **Clock not synchronizing:** ESP32 needs 2.4 GHz Wi-Fi and UDP NTP access; inspect Serial Monitor. Weather TLS validation also needs a valid clock.
- **Weather never appears:** Validate Wi-Fi/time, read the HTTP/TLS error in Serial Monitor, and check whether Open-Meteo's certificate chain still terminates at the included CA. **Do not use `setInsecure()` as a permanent fix.**
- **Joystick direction backwards:** Change `INVERT_JOYSTICK_X` in `Config.h` to `true`.
- **No sprite memory:** The program logs allocation outcomes and has direct-render fallbacks; close other allocations, consider reducing sprite sizes.
- **Wrong display proportions:** Keep `setRotation(1)` with `TFT_WIDTH 320`, `TFT_HEIGHT 480` in your previously working TFT_eSPI setup.

## Files

```text
SmartBuddy_V3/
  SmartBuddy_V3.ino     Main setup, UI state and scheduling
  Config.h              Pins, colours, timing, weather location
  Model.h               Small shared data types
  Input.h/.cpp          Joystick debouncing / wake handling
  Sensors.h/.cpp        DHT11 polling
  FocusTimer.h/.cpp     Delay-free 25-minute timer
  Weather.h/.cpp        Background HTTPS fetch + validated JSON
  RootCA.h              ISRG Root X1 certificate for HTTPS verification
  Robot.h/.cpp          Procedural 2D mascot
  UI.h/.cpp             Polished pages, icons, sprites and idle UI
  Secrets.h.example     Template; copy to ignored Secrets.h
  config/               Display setup reference / original CA PEM
  docs/                 Wiring, test plan and design brief
```

## Engineering choices and limitations

- Original ESP32-WROOM-32 has constrained RAM. Instead of creating a 480×320 16-bit sprite (~307 KB), this uses **three small 8-bit RGB332 sprites** (~84 KB total) and redraws only page-sized fields.
- Background weather calls run in a **FreeRTOS worker on core 0** and pass successful readings to the UI via a one-item queue. HTTPS or slow DNS therefore should not pause joystick animation/timer operation, though Wi-Fi/TLS still uses heap and CPU.
- 8-bit sprites slightly quantize colours. Page changes still deliberately redraw the full screen once. The direct-render fallback may show some flashing if sprite allocation fails.
- Open-Meteo values are modeled outdoor conditions, not physical local measurement. DHT11 room data is approximate and may be warm if enclosed near electronics.
- Weather refreshes every ~15 minutes after success, retries ~1 minute after a failure, and labels cached data as **stale** after 45 minutes.
- The HTTPS root CA is a public trust anchor. If Open-Meteo changes certificate providers or chain, update `RootCA.h` from a trustworthy source rather than disabling validation.
- Joystick `VRy` is connected but currently reserved for a future Settings page. Screen brightness requires confirmed backlight PWM circuitry and is **not** implemented.
- No physical sound effects are included because the buzzer isn't wired in the current final pin map.
- Do not run the breadboard/loose wiring unattended until mounted and electrically checked for long-term use.

## Open-source weather source

Weather data is supplied by [Open-Meteo](https://open-meteo.com/) for noncommercial use. Review their current attribution and terms when publishing or distributing the device.

## Next steps for portfolio

- Capture photos/video showing the four UI pages and idle robot.
- Run and document the test plan in `docs/TEST_PLAN.md`.
- Create SolidWorks assembly, printable enclosure and export screenshots to the repository.
- Include actual measurements, performance metrics and failures encountered; label planned features separately from completed ones.
