# MakerVault Hardware

Firmware and hardware documentation for [MakerVault](https://github.com/gavrd7/MakerVault).


## Related MakerVault repositories

- **[MakerVault](https://github.com/gavrd7/MakerVault)** — self-hosted application, Docker deployment and releases.
- **[MakerVault User Guide](https://gavrd7.github.io/MakerVault-docs/)** ([documentation source](https://github.com/gavrd7/MakerVault-docs)) — installation, configuration and usage guides.
- **[MakerVault-Hardware](https://github.com/gavrd7/MakerVault-Hardware)** — RFID/NFC handheld firmware, wiring, printable enclosure files and future connected hardware.

**Licence:** MakerVault-authored code is licensed under the **GNU Affero General Public License v3 (AGPL-3.0)**, matching the main MakerVault project. See [LICENSE](LICENSE) and [third-party notices](THIRD_PARTY_NOTICES.md). The vendored QRCode implementation retains its original MIT licence.

**Repository visibility:** Pre-publication review ongoing. See [public-release checklist](PUBLIC_RELEASE_CHECKLIST.md). Do not treat the source as a production-certified firmware binary.

## RFID/NFC handheld terminal

**Published baseline firmware:** v0.5.1.14 (merged to main; v0.5.1.13 remains the original acceptance baseline) (ESP32-S3 SuperMini, MFRC522 SPI, ST7735S 128×128 TFT, KY-040 encoder and two buttons).

- [Arduino source](firmware/rfid-terminal/MakerVault_Terminal_Firmware/)
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

### Connected MakerTag workflows (draft PR #6)

When used with MakerVault app PR #150, this branch supports read-only MakerTag lookup, browser-initiated UID registration, explicit on-terminal reassignment of existing tags, browser/terminal cancellation, and recovery of interrupted requests. **No tag-memory writes, NDEF encoding or Creality CFS payload decoding are implemented.** The published v0.5.1.14 baseline on `main` predates these integrations.

The v0.5.1.14 maintenance update corrected the displayed and heartbeat firmware versions and passed physical regression tests.

## Other hardware

A filament weigh station is planned. Final Rev3 enclosure STL, STEP, 3MF and CAD sources are included in [hardware/rfid-terminal/](hardware/rfid-terminal/).

## Responsible distribution

Do not commit credentials, provisioning logs with secrets, local CA private keys or personal network details. Do not publish before reviewing historical commits and the provenance/licensing of embedded icon and brand images.

**Next firmware release candidate: v0.6.0 (PR #6).** This branch now reports v0.6.0 on its device information screen and serial output. It adds Maker Tag UID lookup, browser-initiated registration, explicit reassignment, and request cancellation/recovery. It is not released until the PR merges and a release is published. The published stable firmware remains v0.5.1.14. Physical tag-memory writes, NDEF encoding, and Creality CFS payload writing are not supported.
