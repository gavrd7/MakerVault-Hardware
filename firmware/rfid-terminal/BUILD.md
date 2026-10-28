# Build the MakerVault RFID/NFC handheld

**Latest tested firmware:** v0.5.1.14 — version reporting corrected, flashed and regression-tested on physical hardware; v0.5.1.13 is the original acceptance baseline. Source in [MakerVault_Terminal_Firmware](MakerVault_Terminal_Firmware/). **The firmware is a tested candidate, not an Arduino-CI-verified published binary.**

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

## Working vs planned

Working: UID/type scanning of compatible RFID/NFC tags, multi-SSID 2.4 GHz Wi-Fi, MakerVault secure pairing-v1, CA management, HTTPS diagnostics, heartbeats, persistent credentials and TFT navigation.

Available on the draft connected-terminal branch alongside MakerVault PR #150: MakerTag UID resolution, browser-initiated registration and physical-confirmed reassignment, read-only browser lookup, cancellation and recovery. Not implemented: reading Creality CFS payload details, physical RFID/NDEF writing, OTA firmware update, comprehensive factory reset. The experimental hardware has not been independently certified.

## Version validation

Firmware v0.5.1.14 is merged to `main` and the owner confirmed the terminal displays its updated version, heartbeat reporting is correct, and RFID/NFC, Wi-Fi, HTTPS/pairing and controls work as previously tested. The `MakerVault_Terminal_Firmware` source-folder name is historical, not the firmware version.
