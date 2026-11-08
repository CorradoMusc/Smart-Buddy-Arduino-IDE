# Smart Buddy V3 — Design Brief

**Project:** ESP32 desktop companion / environmental dashboard  
**Hardware:** ELEGOO ESP32-WROOM-32, DWEII ST7796S 4-inch 480×320 SPI TFT, DHT11, 5-pin joystick  
**Status:** Existing hardware and four-page firmware have been reported working. V3 remains a design specification, not tested firmware.

## Product goals

1. A clean, readable, cohesive graphical interface with four functional pages.
2. A cute, lightweight animated robot that appears when the joystick is idle.
3. A practical, 3D-printable housing with secure mounts, service access, usable controls, and ventilated room sensing.
4. Retain Wi-Fi time, outdoor weather, indoor environment data, and the 25-minute study timer.
5. Avoid unnecessary new hardware and avoid a full-screen framebuffer on the original ESP32.

## Design system

- **Display:** 480×320 landscape; screen coordinates are in pixels, origin at upper left.
- **Visual language:** dark navy, crisp white text, electric cyan accents, light rounded cards, restrained amber/green status colors.
- **Palette (RGB888, convert to RGB565 in firmware):** background `#0B1020`, card `#16243A`, outline `#2C4058`, primary `#F3F7FC`, secondary `#9BB0C6`, cyan `#51D6FF`, mint `#7CE5B6`, amber `#FFC968`, error `#FB7C88`.
- **Regions:** top status bar y=0..37; main content y=40..282; footer/navigation y=288..319.
- **Navigation:** joystick horizontal = next/previous page; press on timer page = start/pause; in idle mode, any intentional input wakes without also activating a command. Allow a return-to-center before another page movement. Optional press-and-hold = return Home.
- **Pages:** Home, Weather, Room, Focus. Idle Buddy is an automatic overlay, not a fifth manually navigated page.

### Home wireframe (480×320)

- Header: `SMART BUDDY` at x=18, y=12; Wi-Fi state at right.
- Time: large at x=20, y=58, about 50–62 pixels high.
- Date: below clock at x=22, y=123.
- Small mascot portrait at x=375, y=66 (about 72×72 px).
- Outdoor card: x=18, y=175, w=214, h=95; weather icon, temperature, condition.
- Room card: x=247, y=175, w=215, h=95; temperature, humidity icon/value.
- Footer: navigation dots and joystick hint.

### Weather wireframe

- Large weather icon (~84×84) at x=35, y=91.
- Outdoor temperature x=148, y=84; description underneath.
- Two lower cards: Feels like, Outdoor humidity; optional updated timestamp.
- On failure, preserve last known values and show `Offline` or `Updated X min ago` rather than zeros.

### Room wireframe

- Large indoor temperature left and droplet/humidity value right.
- Color-coded comfort label based on configurable thresholds, explicitly an approximation because DHT11 has limited accuracy.
- Small line `DHT11 • Updated just now` or show sensor error if readings fail.
- Avoid full-page redraw on every sensor read.

### Focus wireframe

- Large countdown centered, e.g. `24:59`.
- Thin filled progress bar below, elapsed/remaining clearly labelled.
- Robot eyes in focused expression.
- Joystick press starts/pauses; pressing after completion resets/starts.
- Timer must be based on elapsed milliseconds, not loop delay.
- If idle mode appears while running, show a small countdown badge on idle screen.

## Robot idle behavior

- Enter after **60 s** without deliberate joystick interaction (configurable).
- Exit when the joystick moves past a threshold or is pressed. The wake input is consumed: no accidental timer toggle or page skip.
- Face design: rounded 160×142 px body; two cyan eyes and small cheek highlights; simple mouth; optional antenna.
- States: neutral, blink, happy (timer complete), focus (timer active), sleepy (extended idle), offline (small connectivity mark only; don't imply emotions from errors).
- Animation: eye blink every ~4–8 s, 100–180 ms close phase; body bob ~1–2 px with ~200–350 ms step; optional rare eye glance. No need for video or many bitmap frames.
- Target visual update ~5–10 frames/s in a **small robot area**, not the whole TFT. Ensure weather/time/sensor tasks continue while idle.
- Use `millis()` schedules, not long blocking `delay()` calls.

## Memory-aware rendering architecture

- Initial static page backgrounds are drawn once on page change.
- Draw the clock, changing temperature values, weather status, and timer as separate fields.
- Use small TFT_eSprite regions only as necessary. Examples: 220×48 at 16-bit ≈ 21 KB; 160×142 at 8-bit ≈ 23 KB.
- A full 480×320 16-bit buffer requires 307,200 bytes before overhead and is unsuitable as the default strategy on this ESP32 without PSRAM.
- Use built-in vector primitives to draw rounded cards, icons, eyes and progress bars. Store any small icon bitmaps in program flash where appropriate.
- Avoid `fillScreen()` or clearing the entire content region once per second.
- Avoid fetching weather in a display-render callback. Use independent data/scheduling logic.
- Weather HTTPS: prototype currently skips TLS verification; use certificate validation for production, and retry with backoff on errors.

## Physical enclosure — first CAD proposal

**All dimensions below are starting targets, not verified fabrication dimensions. Measure the actual modules before finalizing.**

- **Form:** desk-standing angled mini terminal, screen landscape, joystick lower-right.
- **Suggested outer face:** 146 mm wide × 138 mm tall; **depth:** approximately 60 mm; **face tilt:** 12° backward from vertical. Adjust for cable clearance and stability.
- **Wall thickness:** 2.4–2.8 mm for a first FDM print; nominal 2.5 mm.
- **Corner radii:** approx. 9–12 mm for an inviting robot aesthetic.
- **Screen module published PCB size:** approximately 108.04×61.74 mm in landscape orientation; **published active display area:** 83.52×55.68 mm. These are not interchangeable with total glass/module dimensions. Verify actual placement of glass, mounting holes and header.
- **Suggested LCD PCB placement on front-face drawing:** x≈19–127, y≈14–76 (landscape); adjust based on actual mounting holes and glass offsets.
- **Joystick rough location:** center x≈110, y≈109 on front face. Confirm board footprint, stick swing and clearance from LCD PCB.
- **Screen bezel window:** based on measured visible display region (not the PCB outline). Begin with measured width/height + ~0.5 mm clearance each side, then prototype a small gauge before cutting the final bezel.
- **Mounting:** LCD by existing corner board holes, standoffs plus appropriate washers/screws; confirm hardware diameter and keep screws clear of glass/traces. ESP32 on plastic tray or standoffs with USB accessible. Joystick supported by its own bracket so users do not push on solder joints.
- **Airflow:** DHT11 on a side or rear vented bay, separated from display/regulator heat as much as practical; note that enclosed temperature may read warmer.
- **Serviceability:** removable rear panel with 4 screws; don't permanently glue the front shut.
- **USB:** opening lined up with real ESP32 connector, with room for USB-C plug housing and cable bend. Add simple strain relief for longer-term use.
- **Wire organization:** label/route wires, use retention clips, secure shared power distribution; breadboard and loose Dupont connections are fine for prototyping but need strain relief and verification for long-term 24/7 use.
- **Material:** PLA suitable for first dimensional prototype on a cool desk; PETG optional for a tougher enclosure. Avoid high-temperature environments.

### Suggested SolidWorks part structure

1. `SBV3_Front_Bezel.SLDPRT` — display aperture, mounting bosses, joystick hole, design accents.
2. `SBV3_Main_Housing.SLDPRT` — wedge/stand shell, electronics rails/tray, vents, USB access, cover mounting.
3. `SBV3_Rear_Cover.SLDPRT` — removable panel, screw clearance, optional grille/branding.
4. `SBV3_Assembly.SLDASM` — fit/clearance evaluation with simplified imported or modeled LCD, ESP32, joystick, DHT11 components.

### Measurement checklist before CAD

- LCD PCB external L/W/thickness; glass outline; visible active area x/y offsets; mounting hole center-to-center distances and diameters; header protrusion/connection clearance.
- ESP32 external dimensions, mounting hole spacing/diameter, USB-C connector location and plug insertion envelope.
- Joystick board L/W/thickness, shaft height, cap diameter, maximum travel, button travel, mounting hole spacing.
- DHT11 PCB L/W, sensor position, header direction and air-exposure clearance.
- Actual jumper connector height and minimum comfortable wire bend radius.

### SolidWorks sequence

1. Create LCD, ESP32, joystick, DHT11 simplified reference parts using measured dimensions.
2. Make a master front-face layout sketch with display aperture, board keepouts and joystick center.
3. Model the front bezel at 2.5 mm nominal thickness and add LCD locating features; test for glass clearance.
4. Model an angled housing and check depth for screen pins, ESP32, USB plug and wire loops.
5. Add standoffs, joystick bracket, DHT11 ventilated bay, cable channels and rear-cover bosses.
6. Make the removable rear cover, checking tool access and screw engagement.
7. Run interference and clearance checks in assembly and verify stand stability.
8. Print an inexpensive *LCD fit gauge* and *joystick hole gauge* before the full body.
9. Print a full prototype, test fit and adjust; then print final enclosure.

### First-print guidance

- Typical 0.4 mm nozzle, 0.2 mm layers, 3 walls/perimeters, around 15–25% infill for housing pieces.
- Orient large flat parts on print bed where possible; split geometry if required to minimize supports.
- Check hole tolerances with printer calibration; aim for small test coupons before committing to screw holes.
- Use screws appropriate to actual hole diameters and plastic boss geometry; do not force hardware against the LCD PCB.

## Verification & co-op portfolio evidence

- Screen remains responsive during weather refresh and joystick usage.
- No persistent flicker at once-per-second clock updates.
- Confirm joystick navigation, wake-on-input and no accidental button press action.
- Wi-Fi-off / Wi-Fi-on recovery; last successful weather retained with age indicator.
- 24-hour operation with time/weather/sensor logs; test more than one power cycle.
- DHT11 readings checked against a second sensor if possible; acknowledge accuracy limits.
- Photos of prototype and final enclosure, circuit diagram, CAD renders, printed-part screenshots, and a 30–60 second demonstration video.
- GitHub README should describe *tested* features separately from planned features.

## Implementation order / milestones

1. **V3 UI base:** create colors, status/navigation components, cards and four static polished pages.
2. **Dynamic updates:** field-sized sprites or selective drawing for clock, weather, temperature, timer. Verify no perceptible flicker.
3. **Robot:** draw one clean robot using TFT primitives and test blink/bob timing.
4. **Idle controller:** 60 s inactivity, wake-input consumption, optional timer mini-badge.
5. **Resilience:** Wi-Fi reconnection, stale weather status, TLS validation, timer unaffected by network requests.
6. **CAD:** measure, sketch, assemble simplified models, print fit gauges, prototype housing.
7. **Portfolio:** photos, diagram, design files, test log and demo.

## References

- Display module geometry and electrical specs: https://www.lcdwiki.com/4.0inch_SPI_Module_ST7796
- TFT_eSPI library and sprite guidance: https://github.com/Bodmer/TFT_eSPI

---

*Design brief only. Do not treat the provisional enclosure dimensions as verified manufacturing dimensions or V3 as already implemented firmware.*
