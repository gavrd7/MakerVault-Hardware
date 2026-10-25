# MakerVault Hardware

Hardware and firmware for [MakerVault](https://github.com/gavrd7/MakerVault).

## RFID/NFC handheld terminal

**Current physically tested baseline:** **v0.5.1.13 release candidate** (25 October 2026). The exact tested source has not yet been imported into this repository; see [source import requirements](firmware/rfid-terminal/SOURCE_IMPORT_v0.5.1.13.md) and [hardware acceptance](firmware/rfid-terminal/ACCEPTANCE_v0.5.1.13.md) before merging or publishing a release.

ESP32-S3 SuperMini, MFRC522 shared SPI reader, ST7735S 128×128 TFT, KY-040 encoder with push and separate Back/Confirm buttons.

Working and tested: multi-network 2.4 GHz Wi-Fi, QR/LAN and temporary AP setup, MakerVault pairing-v1 over validated HTTPS, Local CA import/removal, endpoint-bound persistent credentials, heartbeat, scope/status reporting, offline RFID UID reads, and responsive TFT menus.

**Not yet implemented:** MakerTag record resolution, tag creation/registration and writing physical RFID memory. Do not advertise these as complete merely because backend API scopes exist.

The source imports the MIT-licensed [ricmoo/QRCode](https://github.com/ricmoo/QRCode) C library. Retain its source notices and [licence](firmware/rfid-terminal/third_party/QRCode-LICENSE.txt).

See [pairing compatibility](firmware/rfid-terminal/PAIRING_COMPATIBILITY.md) and the [MakerVault Connected Devices spec](https://github.com/gavrd7/MakerVault/blob/feature/143-connected-device-pairing/docs/CONNECTED_DEVICES_PAIRING.md).

## Other devices

The filament weigh station is planned. It is not part of this handheld firmware baseline.

## Release safeguards

Keep this repository private until the exact tested sketch, dependencies, build instructions and security review are complete. Never commit Wi-Fi credentials, API keys or CA private keys. The v0.5.1.13 source contains an older version label in System info/heartbeat metadata; correct and test it before declaring a published stable version.

Choose an appropriate project licence before making this repository public.
