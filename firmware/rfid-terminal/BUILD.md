# Build the MakerVault RFID/NFC handheld

**Current tested firmware:** v0.6.0 — online Maker Tag UID lookup, registration, reassignment and cancellation, compiled and flashed to physical hardware; v0.5.1.14 remains the prior acceptance baseline. Source in [MakerVault_Terminal_Firmware](MakerVault_Terminal_Firmware/). **The firmware is a tested candidate, not an Arduino-CI-verified published binary.**

## Hardware

- ESP32-S3 SuperMini with USB-C
- ST7735S 1.44-inch 128×128 TFT
- MFRC522/RC522 SPI reader
- KY-040 rotary encoder (push-to-select)
- Two independent Back and Confirm buttons

### Validated pin map

| Function | GPIO |
|---|---|
| Shared SPI SCK / MOSI / MISO | 12 / 11 / 13 |
| TFT CS / DC / RESET | 1 / 2 / 43 |
| MFRC522 CS / RESET | 10 / 9 |
| Encoder CLK / DT / SW | 4 / 5 / 6 |
| Back / Confirm | 7 / 8 |
| TFT power and backlight | 3V3 |
| Common ground | GND |

Use 3.3 V logic; do not connect the RC522 to 5 V. The prototype has its TFT backlight directly on 3V3; this is a tested wiring observation, **not** confirmation that every third-party display module safely limits its backlight current.

## Arduino IDE

- Board: **ESP32S3 Dev Module**, matching your device flash size
- Tested development environment: Arduino-ESP32 core 3.x (last reported 3.3.12), Arduino IDE 2.x
- USB CDC On Boot: Enabled for serial diagnostic output
- Monitor speed: 115200 baud
- Partition scheme: **Huge APP (3MB No OTA)**, where supported by the actual flash size
- When upgrading, do not erase NVS/flash if you want to keep saved networks and pairing credentials. Be aware that changing partition layouts can affect stored settings.

### Libraries and source layout

In Arduino IDE install supported versions of:
- Adafruit GFX Library
- Adafruit ST7735 and ST7789 Library
- MFRC522 (compatible GithubCommunity/miguelbalboa derivative)
- Adafruit NeoPixel (if required by the current code)
- QRCode library by ricmoo is **bundled locally** as `qrcode.c` and `qrcode.h`, so no manual source copying is required
- ESP32CertBundle support used for public CA roots

Keep all files in the **same sketch folder**:

```text
MakerVault_Terminal_Firmware/
  MakerVault_Terminal_Firmware.ino
  maker_backend.h
  provisioning.h
  connected_registration.h
  scan_lookup.h
  remote_lookup.h
  brand_images.h
  ui_icons.h
  qrcode.c
  qrcode.h
```

Open `MakerVault_Terminal_Firmware.ino`, select the ESP32-S3 serial port, compile and upload over USB. Confirm secure HTTPS operation with the correct MakerVault Local CA before pairing.

**Do not use insecure TLS**, expose the LAN provisioning portal to the public internet, or commit actual Wi-Fi/API credentials.

## Working and supported in v1.0.0 release-candidate source

- Trusted HTTPS and eight-digit MakerVault Connected Devices pairing; native ESP-IDF `esp_http_client` with persistent TLS and strict local/public CA verification.
- Terminal-initiated and browser-led MakerTag registration, reassignment, NFC URL writing/verification, cancellation, overwrite protection, record browsing with A–Z letter jump.
- Clean registered writable NTAG215 NFC user data while retaining the MakerTag record; optionally clean before removing a registration. These operations require the matching MakerVault v1.5.0 app implementation.
- Read-only CFS material information and experimental Bambu/Elegoo RFID decoding; third-party tag sector writing remains intentionally unsupported.
- FreeRTOS worker cleanup to avoid retained C++ `String` allocations. The maintainer reports stable heap through a two-hour-plus idle validation run.

See [flashing and regression tests](FLASHING.md), [release notes](../../docs/TERMINAL_v1.0.0_RELEASE_NOTES.md) and [NFC writer safety](../../docs/ntag215-ndef-writer-safety.md). OTA updating is not supported by this release.

## Version and release asset validation

Source is prepared as **v1.0.0** for MakerVault **v1.5.0**. Published release assets are generated from a **tagged commit**, not from a draft PR. Check the exact resulting release tag and compare the startup serial banner, System info and MakerVault heartbeat firmware version. A normal update must preserve NVS (Wi-Fi networks, CA certificate and device pairing).

The publishing workflow produces an application binary, full flashing binaries and sketch ZIP, plus STL/STEP archives reused unchanged from previously released CAD assets. Third-party Arduino dependencies are installed separately and must match the versions above. Check that GitHub Actions succeeds and actual assets exist before announcing publication.
