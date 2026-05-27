<h1 align="center">Phantom Firmware</h1>
<h3 align="center">Custom Flipper Zero firmware for security research</h3>

<p align="center">
    Based on <a href="https://github.com/DarkFlippers/unleashed-firmware">Unleashed Firmware</a> with additional pentesting tools and applications
</p>

---

> [!WARNING]
> This firmware is intended solely for **authorized security testing**, **educational purposes**, and **security research**.
> Always obtain proper authorization before testing systems you do not own.
> This project is not affiliated with Flipper Devices or DarkFlippers.

---

## What Is Phantom?

Phantom is a Flipper Zero firmware that combines the stability and protocol support of **Unleashed** with **10 custom security research applications** built specifically for penetration testing workflows.

Everything Unleashed offers is included — 117 SubGHz protocols, 41 NFC card parsers, removed TX restrictions, extended frequencies, BadUSB/BadKB, rolling code support, and all community plugins. Phantom adds a curated set of offensive and defensive security tools on top.

---

## Phantom Apps

### WiFi & Network (require ESP32 devboard with Marauder firmware)

| App | Description |
|-----|-------------|
| **WiFi Marauder** | Full ESP32 Marauder companion — scan APs/stations, SSID spam (random & rickroll), deauth flood, probe flood, PMKID capture, beacon/deauth/pwnagotchi sniffing, channel hopping |
| **Evil Portal** | Captive portal credential harvesting — deploy fake Google, Facebook, or Microsoft login pages, view and clear captured credentials |
| **WiFi Deauther** | DSTIKE deauther control — scan networks, select targets, launch deauth/beacon/probe attacks |
| **WiFi Scanner** | Visual WiFi network scanner — scrollable list with signal strength bars, SSID, channel, and encryption type |
| **Packet Monitor** | Real-time WiFi packet visualization — rolling bar graph of packets/sec, per-channel monitoring, live statistics |
| **BT Scanner** | Bluetooth reconnaissance — BLE device scanning, credit card skimmer detection, AirTag detection, Flipper device detection |

### Native Flipper (no extra hardware needed)

| App | Description |
|-----|-------------|
| **BLE Spam** | BLE advertisement attacks — Apple Continuity popups (AirPods, Beats, AppleTV), Google Fast Pair, Windows Swift Pair, Samsung BLE spam, all-in-one cycling mode |
| **RFID Fuzzer** | UID generator and fuzzer for EM4100, HID Prox, NTAG/Ultralight, and Mifare Classic — sequential, random, and pattern modes with automatic file generation to SD card |
| **Password Gen** | Cryptographic password generator using hardware RNG — alphanumeric, full symbols, numeric PIN, and hex modes with configurable length (8-32 characters) |
| **Hash Calc** | Cryptographic hash calculator — compute MD5, SHA-1, and SHA-256 hashes of arbitrary text input with on-screen keyboard |

---

## Inherited from Unleashed

<details>
<summary><strong>Sub-GHz</strong></summary>

- 117 protocol implementations (garage openers, sensors, alarms, smart home)
- Regional TX restrictions removed
- Extended frequency range (300-868MHz)
- Rolling code support (KeeLoq, CAME Atomo, Nice Flor-S, Somfy Telis, Security+ 2.0, etc.)
- External CC1101 module support
- Frequency analyzer with real-time detection
- Sub-GHz bruteforce and remote control plugins
- Custom button codes for rolling code remotes
- Save-last-settings and protocol names in filenames
</details>

<details>
<summary><strong>NFC / RFID / iButton</strong></summary>

- 41 NFC card parsers (transit, payment, access control systems worldwide)
- Extra Mifare Classic keys in system dictionary
- EMV protocol + public data parser
- NFC "Add manually" with custom UID for Mifare Classic
- LFRFID and iButton fuzzer plugins
</details>

<details>
<summary><strong>Infrared</strong></summary>

- Universal remotes for TVs, projectors, fans, A/C units, and audio equipment
- RCA protocol support
- External IR module support with auto-detection
</details>

<details>
<summary><strong>BadUSB / HID</strong></summary>

- BadKB (Bluetooth HID) integrated into BadUSB app
- Multiple keyboard layout support
- USB and Bluetooth HID emulation
</details>

<details>
<summary><strong>Quality of Life</strong></summary>

- Customizable Flipper name (Settings > Desktop)
- Desktop clock display
- Battery percentage with multiple display styles
- PIN lock via UP button hold
- Text input cursor feature
- Byte input nibble editor
</details>

---

## Hardware Requirements

| Component | Required For |
|-----------|-------------|
| **Flipper Zero** | Everything |
| **ESP32 WiFi Devboard** | WiFi Marauder, Evil Portal, Deauther, Scanner, Packet Monitor, BT Scanner |
| **ESP32 Marauder Firmware** | Must be flashed on the ESP32 devboard for WiFi/BT apps |

The ESP32 devboard connects to the Flipper's GPIO pins (UART). Flash it with [ESP32 Marauder](https://github.com/justcallmekoko/ESP32Marauder) using the [web flasher](https://fzeeflasher.com) or [FZEasyMarauderFlash](https://github.com/SkeletonMan03/FZEasyMarauderFlash).

BLE Spam, RFID Fuzzer, Password Generator, and Hash Calculator work with just the Flipper Zero — no extra hardware.

---

## Build & Flash

### Prerequisites

- Linux, macOS, or WSL2 on Windows
- Git
- Python 3
- The ARM GCC toolchain is downloaded automatically on first build

### Build

```bash
git clone https://github.com/dylan-saronic/phantom-firmware.git
cd phantom-firmware
./fbt fw_dist
```

The build output (firmware + all FAPs) will be in `dist/`.

### Flash

```bash
# Flash over USB (Flipper connected via USB cable)
./fbt flash_usb

# Or copy the update package to SD card for self-update
./fbt updater_package
```

### Build Only the Phantom Apps (as FAPs for SD card)

```bash
./fbt fap_dist
```

This produces `.fap` files you can copy to the Flipper's SD card under `apps/Phantom/`.

---

## Project Structure

```
phantom-firmware/
├── applications/           # Core firmware applications (from Unleashed)
│   ├── main/              # SubGHz, NFC, RFID, IR, BadUSB, GPIO, etc.
│   ├── services/          # System services (GUI, BLE, storage, CLI)
│   ├── settings/          # Settings apps
│   └── system/            # System apps (HID, updater, MFKey, JS runtime)
├── applications_user/     # Custom Phantom apps
│   ├── phantom_wifi_marauder/
│   ├── phantom_evil_portal/
│   ├── phantom_wifi_deauther/
│   ├── phantom_wifi_scanner/
│   ├── phantom_packet_monitor/
│   ├── phantom_bt_scanner/
│   ├── phantom_ble_spam/
│   ├── phantom_rfid_fuzzer/
│   ├── phantom_password_gen/
│   └── phantom_hash_calc/
├── furi/                  # FURI core (FreeRTOS abstraction)
├── lib/                   # Libraries (SubGHz, NFC, IR, BLE, mbedTLS, etc.)
├── targets/               # Hardware targets (STM32WB55)
└── fbt                    # Flipper Build Tool
```

---

## Credits

- **[Unleashed Firmware](https://github.com/DarkFlippers/unleashed-firmware)** by @xMasterX and the DarkFlippers team — the foundation this firmware is built on
- **[Official Firmware](https://github.com/flipperdevices/flipperzero-firmware)** by Flipper Devices — the original firmware
- **[ESP32 Marauder](https://github.com/justcallmekoko/ESP32Marauder)** by @justcallmekoko — WiFi/BT pentesting suite for the ESP32 companion board
- **[Awesome Flipper Zero](https://github.com/djsime1/awesome-flipperzero)** — community resource list
- Phantom apps by @dylan-saronic

---

## License

This project is licensed under the [GNU General Public License v3.0](/LICENSE), the same license as the Unleashed and Official firmwares.
