# MakerVault Hardware

Firmware and hardware documentation for [MakerVault](https://github.com/gavrd7/MakerVault).

**Licence:** MakerVault-authored code is licensed under the **GNU Affero General Public License v3 (AGPL-3.0)**, matching the main MakerVault project. See [LICENSE](LICENSE) and [third-party notices](THIRD_PARTY_NOTICES.md). The vendored QRCode implementation retains its original MIT licence.

**Repository visibility:** Pre-publication review ongoing. See [public-release checklist](PUBLIC_RELEASE_CHECKLIST.md). Do not treat the source as a production-certified firmware binary.

## RFID/NFC handheld terminal

**Hardware-tested firmware:** v0.5.1.13 release candidate (ESP32-S3 SuperMini, MFRC522 SPI, ST7735S 128×128 TFT, KY-040 encoder and two buttons).

- [Arduino source](firmware/rfid-terminal/MakerVault_v0523/)
- [Firmware build setup](firmware/rfid-terminal/BUILD.md)
- [Firmware flashing / troubleshooting](firmware/rfid-terminal/FLASHING.md)
- [Exact Rev C.3 wiring reference](hardware/rfid-terminal/WIRING.md)
- [Rev3 hardware BOM](hardware/rfid-terminal/BOM.md)
- [Rev3 enclosure assembly / CAD](hardware/rfid-terminal/BUILD_GUIDE.md)
- [Hardware test evidence](firmware/rfid-terminal/ACCEPTANCE_v0.5.1.13.md)
- [Secure pairing compatibility](firmware/rfid-terminal/PAIRING_COMPATIBILITY.md)
- [TLS security checks](firmware/rfid-terminal/TLS_SECURITY_ACCEPTANCE_v0.5.1.7.md)

### Available now

2.4 GHz Wi-Fi with multiple saved networks; temporary access point or LAN QR phone setup; certificate-verified MakerVault HTTPS, Local CA management, secure eight-digit pairing-v1, NVS credential persistence and endpoint guard, background heartbeat, TFT navigation, and basic RFID/NFC tag UID scanning.

### Not implemented yet

Online MakerTag record resolution, MakerTag creation/registration, physical RFID writes and Creality CFS payload decoding. Backend API permission scopes do not imply these features are implemented in the handheld.

The tested build contains an older version string in System info and some heartbeat metadata; do not silently change the validated source.

## Other hardware

A filament weigh station is planned. **Finished printable enclosure models are not yet included here**; assembly documentation and source-model licensing require a separate review.

## Responsible distribution

Do not commit credentials, provisioning logs with secrets, local CA private keys or personal network details. Do not publish before reviewing historical commits and the provenance/licensing of embedded icon and brand images.
