# Chopper Head V2

> This firmware was strongly influenced by the [Amidala](https://github.com/thePunderWoman/Amidala) firmware by [thePunderWoman](https://github.com/thePunderWoman).

Firmware for Chopper (C1-1OP) head built on the **ESP32-WROOM-32**. Controls servo animations, RGB LED lighting effects, and communicates wirelessly with a main controller via ESP-NOW.

## Features

- **Servo animations** — Arms, periscope, and greeting motions driven by a Pololu Maestro controller over UART
- **RGB LED effects** — Jewel eyes (7-pixel each), ladder (11-pixel), center eye, and periscope lights with pulsing, bouncing, and color-change animations
- **ESP-NOW communication** — Encrypted peer-to-peer wireless protocol for low-latency remote control
- **Web interface** — Device dashboard, real-time serial monitor, and command sender
- **OTA firmware updates** — Upload new firmware over WiFi through the web interface
- **Unit tests** — Native-platform tests with mock hardware abstractions

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32-WROOM-32 |
| Servo controller | Pololu Maestro (UART2, GPIO 16 TX / GPIO 17 RX) |
| Left jewel eye | NeoPixel, 7 pixels, GPIO 4 |
| Right jewel eye | NeoPixel, 7 pixels, GPIO 18 |
| Center eye | NeoPixel, 1 pixel, GPIO 5 |
| Ladder | NeoPixel, 11 pixels, GPIO 19 |
| Periscope | NeoPixel, 1 pixel, GPIO 2 |

PCB design files are in the `PCB/` directory.

## Project Structure

```
src/
  main.cpp                    Entry point and main loop
  config.h                    Pin definitions, MAC addresses (keys come from secrets.h)
  lights/                     LED control (JewelEye, Ladder, PixelLight, LightController)
  servo/                      Pololu Maestro servo scripts
  communication/              ESP-NOW receiver and message types
  web/                        Embedded web server, logging, serial buffer
include/
  web_pages.h                 Auto-generated embedded HTML (built by scripts/embed_web.py)
  secrets.h                   Auto-generated ESP-NOW keys (built by scripts/load_secrets.py), gitignored
web/                          Web UI source files (HTML, CSS)
test/                         Unit tests and hardware mocks
scripts/
  embed_web.py                Pre-build script that inlines web assets into firmware
  load_secrets.py             Pre-build script: .env -> include/secrets.h
partitions/
  ota_4MB.csv                 OTA partition table
PCB/                          PCB design and CNC routing files
.env.example                  Template for ESP-NOW keys — copy to .env
```

## Prerequisites

- [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) extension
- Python 3 (used by the `embed_web.py` and `load_secrets.py` build scripts)

## Setup

1. Clone the repository and open it in VS Code.

2. Set up `.env` — see [Generating ESP-NOW keys](#generating-esp-now-keys) below. This covers `PMK_KEY`/`LMK_KEY`, `CONTROLLER_MAC`, and `WIFI_AP_PASSWORD`. The build fails with a clear error if `.env` is missing.

3. Optionally edit `src/config.h`:
   - Change `WIFI_AP_SSID` if desired.

4. Build the firmware:
   ```bash
   pio run -e esp32dev
   ```

5. Upload to the ESP32:
   ```bash
   pio run -e esp32dev -t upload
   ```

6. Open the serial monitor:
   ```bash
   pio device monitor -b 115200
   ```

## Web Interface

Connect to the **ChopperHead** WiFi access point, then open one of:

- `http://chopper.head`
- `http://chopper.local`
- `http://192.168.5.2`

### Pages

| Page | Description |
|---|---|
| Home | Device info — firmware version, MAC address, uptime, heap memory, ESP-NOW status |
| Serial Monitor | Real-time log viewer with command buttons and macro sender |
| Firmware Update | OTA upload with progress tracking |

### API

| Endpoint | Method | Description |
|---|---|---|
| `/api/info` | GET | Device telemetry (JSON) |
| `/api/monitor` | GET | Live log stream (JSON) |
| `/api/command` | POST | Send button or macro commands |

## Controls

### Button Commands (via controller or web)

| Button | Action |
|---|---|
| Y | Toggle arms in/out |
| X | Say hi (both arms) |
| B | Say hi (left arm) |
| D-pad Up | Periscope up |
| D-pad Down | Periscope down |
| D-pad Left/Right | Periscope animation |

### Macros

| Macro ID | Effect |
|---|---|
| 101 | Red eye animation |
| 103 | Periscope up sequence |
| 107 | Custom animation |

## Communication Protocol

The head communicates with the controller using packed structs over ESP-NOW:

**HeadCommand** (controller → head): button presses, macro sequences, volume, idle mode, and connection status.

**HeadTelemetry** (head → controller): connection state, animation status, and uptime. Sent every 1 second.

Connection is considered lost after 5 seconds without data.

## Running Tests

Unit tests run on the native platform (no hardware required):

```bash
pio test -e native
```

Tests cover: LightController, ServoController, ESP-NOW controller, JewelEye, PixelLight, MessageTypes, and startup animation.

## Generating ESP-NOW keys

This board talks to the other Chopper boards over encrypted ESP-NOW. Before
your first build, generate two 16-byte (128-bit) keys — a Primary Master Key
(PMK) and a Local Master Key (LMK) — and put them in a `.env` file
(gitignored, never commit it) at the project root:

```bash
cp .env.example .env
```

Generate two random 16-byte keys, each as 32 hex characters:

```bash
# Option 1: openssl
openssl rand -hex 16   # run twice — once for PMK_KEY, once for LMK_KEY

# Option 2: python
python3 -c "import secrets; print(secrets.token_hex(16))"
```

Edit `.env` and paste the values in:

```
PMK_KEY=<first random hex string>
LMK_KEY=<second random hex string>
```

**These two keys must be byte-for-byte identical across chopper body,
chopper head, and chopper receiver** — generate them once and copy the same
`.env` values into all three projects. Mismatched keys mean the boards can't
decrypt each other's ESP-NOW packets and the mesh will never connect.

`.env` also holds `CONTROLLER_MAC` — the controller's WiFi MAC address (as
hex, e.g. `C0CDD6CA29E0`), which must likewise be identical across all three
firmware projects — and `WIFI_AP_PASSWORD`, the password for this board's
local OTA-update access point (pick your own, at least 8 characters; unlike
the other values, this one doesn't need to match the other boards).

`scripts/load_secrets.py` runs automatically before every build (see
`platformio.ini`) and turns `.env` into `include/secrets.h`, which
`config.h` includes. Neither `.env` nor the generated `secrets.h` are
committed to git.

## License

All rights reserved.
