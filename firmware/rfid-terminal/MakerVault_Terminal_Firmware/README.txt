MakerVault Terminal v1.0.1 — Arduino sketch
==========================================

This directory is the complete MakerVault Terminal sketch source. Open
MakerVault_Terminal_Firmware.ino in Arduino IDE, leaving all sketch files in
the same directory. External Arduino libraries and the ESP32 board package
are installed separately; see ../../BUILD.md and ../../FLASHING.md.

Tested target: ESP32-S3 SuperMini, RC522 SPI, ST7735S 128x128 colour TFT,
KY-040 encoder and Back/Confirm buttons. Keep the documented GPIO wiring.

Features in the matching v1.0.1 release-candidate source:
- Verified HTTPS and 8-digit Connected Devices pairing with scoped API keys.
- Native ESP-IDF esp_http_client transport with reusable TLS session; one
  authenticated 5-second pending-work poll, heartbeat and status verification.
- MakerTag scan, on-terminal and browser-led registration, reassignment,
  terminal record browsing and A-Z letter jump.
- Physical NTAG215 NDEF URL writing, verification, overwrite and cancellation
  safeguards. Never bypass matching UID, confirmed operation or permissions.
- Clean registered writable NTAG215 NFC user data without removing the record;
  optionally clean before removing a MakerTag registration. Requires the
  companion MakerVault app implementation (PR #165).
- Read-only Creality CFS filament material decoding and experimental read-only
  Bambu/Elegoo decoding; third-party tag writing is not implemented.

The firmware fixes the earlier repeated idle HTTPS heap loss by replacing the
Arduino HTTP client and ensuring C++ temporary buffers unwind before FreeRTOS
worker deletion. The maintainer's two-hour-plus idle test showed stable heap;
this is a validated observation, not a guarantee for every network setup.

Flash using the exact ESP32-S3 partition/board configuration in BUILD.md.
Do not use 'Erase All Flash' on a paired terminal: NVS stores Wi-Fi, local CA,
device key and endpoint binding. Never disable TLS verification or copy the
local CA private key onto the terminal.

Release download ZIPs may be generated from tags after PR merge; a feature
branch build is not automatically the previously published release. Verify
the installed version, release asset checksums, trusted server endpoint,
online pairing and MakerTag safety before deploying.

The bundled QRCode qrcode.c/qrcode.h files retain their third-party MIT
licence notice.
