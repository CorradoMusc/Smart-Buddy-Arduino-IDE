# V3 validation checklist (record actual results before claiming completion)

| Test | Expected behavior | Result / date |
| --- | --- | --- |
| Boot from USB | Home page renders with no reset loop | Pending |
| Initial 2.4 GHz Wi-Fi | ONLINE indicator appears | Pending |
| NTP time | Correct Ontario time and DST | Pending |
| Weather HTTPS | Outdoor condition appears; TLS verified | Pending |
| Sensor | DHT11 updates approximately every 2.5s | Pending |
| Joystick ±X | Exactly one page step per off-centre movement | Pending |
| Focus click | Start, pause, resume and complete | Pending |
| Background weather fetch | Timer continues without freezing | Pending |
| Idle entry | Robot appears after 60s of no interaction | Pending |
| Idle animation | Blink/bob happens with no full-screen flash | Pending |
| Wake | First input wakes only, no accidental page/timer action | Pending |
| Wi-Fi unplug test | Clock continues; last weather retained with age/stale label | Pending |
| Wi-Fi restoration | Automatic reconnect and weather update | Pending |
| 24h stability | No hang, repeated reboot or hot components | Pending |
| Power cycle | Recovers Wi-Fi / clock / weather | Pending |
| Enclosure fit | Joystick, ventilation, access and cable clearance | Pending |

Suggested evidence: screenshot of Serial Monitor, video of transitions, elapsed-time check of timer, photo of internals, measured sensor comparison and results log.

**Do not leave an unsecured breadboard and jumper-wire prototype unattended for an overnight 24h test.** Secure and inspect the assembly first.
