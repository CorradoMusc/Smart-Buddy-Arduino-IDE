# Smart Buddy — ESP32 Desktop Assistant

An ESP32-based desktop dashboard with a **4-inch ST7796S 480×320 TFT LCD**, live outdoor weather, internet-synchronized clock, indoor temperature/humidity, a joystick-driven four-page interface, and a 25-minute study timer.

> **Project status:** The user reported that the prototype's display, Wi-Fi clock, Open-Meteo weather, DHT11 and joystick/timer all worked together. The V2 flicker-reduction revision below was supplied afterward but was **not confirmed tested** in our conversation. These files are reconstructed from the code developed in chat, rather than copies exported from Arduino IDE.

## Features

- Four joystick-controlled pages: Home, Weather, Room, Study Timer
- ESP32 Wi-Fi + NTP clock (Ontario timezone with DST)
- Open-Meteo weather, refreshed every ~15 minutes
- DHT11 indoor temperature and humidity (~2.5-second sampling)
- 25-minute focus countdown, start/pause using joystick press
- V2: partial screen updates instead of full-screen clearing every second

## Hardware

- ELEGOO ESP32 development board (ESP32-WROOM-32 family)
- DWEII 4.0-inch ST7796S SPI TFT LCD, 480×320, touchscreen pins unused
- DHT11 module
- 5-pin analog joystick module
- Breadboards, jumper wires, suitable USB supply

See [docs/WIRING.md](docs/WIRING.md) for the complete pin map and safety notes.

## Repository structure

```text
firmware/
  SmartBuddy_V1/       Original functional four-screen program (flickering)
  SmartBuddy_V2/       Reduced-flicker revision (not yet verified on hardware)
examples/
  01_Serial_Test/      First ESP32 upload
  02_WiFi_Test/        Wi-Fi connectivity
  03_NTP_Clock/        Ontario time synchronization
  04_Weather_API/      Open-Meteo JSON request
  05_DHT11_Test/       Indoor sensor test
  06_Display_Test/     ST7796 display test
  07_Hardware_Test/    LCD + DHT11 + joystick test
config/
  User_Setup_SmartBuddy.h  TFT_eSPI driver and pin reference
docs/
  WIRING.md
```

## Arduino IDE setup

1. Install Arduino IDE 2 and the **esp32 by Espressif Systems** board package.
2. Select **ESP32 Dev Module** and the correct COM port. The user's board required a CP210x USB-to-serial driver on Windows.
3. Install libraries: **TFT_eSPI by Bodmer**, **ArduinoJson by Benoit Blanchon** (v7 API), **DHT sensor library by Adafruit**, and **Adafruit Unified Sensor**.
4. Back up your TFT_eSPI `User_Setup.h`, then configure the ST7796S driver and GPIO mappings from `config/User_Setup_SmartBuddy.h`. Ensure `User_Setup_Select.h` selects the intended setup. Avoid duplicate active definitions.
5. Open `firmware/SmartBuddy_V1/SmartBuddy_V1.ino` for the confirmed functional baseline, or V2 for the later reduced-flicker revision.
6. Replace `YOUR_WIFI_NAME` and `YOUR_WIFI_PASSWORD` **locally only**. Never commit credentials to GitHub.
7. Upload and monitor Serial at 115200 baud.

## Display pins

SPI: MOSI=23, SCLK=18, CS=5, DC=27, RESET=33; no MISO used. See wiring document for power pins and sensor connections.

## Known limitations / future work

- V1 clears the entire LCD each second. V2 reduces flicker using rectangle-based field redraws, but V2's behavior is not yet confirmed on the device.
- HTTPS presently calls `WiFiClientSecure::setInsecure()`. **For an unattended or production device, verify the server certificate or CA chain.**
- HTTP requests can temporarily block joystick response; a future version could use an asynchronous task and sensible timeouts.
- On weather fetch failure, the previous reading is retained; the screen does not yet display a last-updated timestamp.
- 24/7 power stability, current draw, enclosure/strain relief, and long-run testing remain outstanding.
- The existing study timer does not yet drive a buzzer.
- Exact compatibility should be checked against the module revision and supply specifications.

## GitHub publishing

Create a new repository named `SmartBuddy-ESP32`, then upload the contents of this folder (or initialize Git locally). Confirm Wi-Fi passwords are placeholders before publishing.

## Weather data

Weather data: [Open-Meteo](https://open-meteo.com/). Example coordinates target Markham, Ontario; change the latitude/longitude to your intended location.
