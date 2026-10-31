# MakerVault Hardware

Firmware and hardware documentation for [MakerVault](https://github.com/gavrd7/MakerVault).


## Related MakerVault repositories

- **[MakerVault](https://github.com/gavrd7/MakerVault)** — self-hosted application, Docker deployment and releases.
- **[MakerVault User Guide](https://gavrd7.github.io/MakerVault-docs/)** ([documentation source](https://github.com/gavrd7/MakerVault-docs)) — installation, configuration and usage guides.
- **[MakerVault-Hardware](https://github.com/gavrd7/MakerVault-Hardware)** — RFID/NFC handheld firmware, wiring, printable enclosure files and future connected hardware.

**Licence:** MakerVault-authored code is licensed under the **GNU Affero General Public License v3 (AGPL-3.0)**, matching the main MakerVault project. See [LICENSE](LICENSE) and [third-party notices](THIRD_PARTY_NOTICES.md). The vendored QRCode implementation retains its original MIT licence.

**Repository status:** Public, with published source releases. Historical security/publication checks remain available in [public-release checklist](PUBLIC_RELEASE_CHECKLIST.md). The project does not provide a production-certified firmware binary.

## RFID/NFC handheld terminal

**Current firmware:** v0.7 (ESP32-S3 SuperMini, MFRC522 SPI, ST7735S 128×128 TFT, KY-040 encoder and two buttons). The v0.5.1.14 release is the prior stable baseline; acceptance records are retained for historical reference.

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

### Connected MakerTag workflows (v0.6.0)

When used with MakerVault v1.4.0, this firmware supports read-only MakerTag lookup, browser-initiated UID registration, explicit on-terminal reassignment of existing tags, browser/terminal cancellation, and recovery of interrupted requests. **No tag-memory writes, NDEF encoding or Creality CFS payload decoding are implemented.** The earlier v0.5.1.14 firmware predates these integrations.

The v0.5.1.14 maintenance update corrected the displayed and heartbeat firmware versions and passed physical regression tests.

## Other hardware

A filament weigh station is planned. Final Rev3 enclosure STL, STEP, 3MF and CAD sources are included in [hardware/rfid-terminal/](hardware/rfid-terminal/).

## Responsible distribution

Do not commit credentials, provisioning logs with secrets, local CA private keys or personal network details. Do not publish before reviewing historical commits and the provenance/licensing of embedded icon and brand images.

**Release:** [Firmware v0.6.0](https://github.com/gavrd7/MakerVault-Hardware/releases) adds online MakerTag UID lookup, registration, confirmed reassignment and cancellation/recovery. Physical tag-memory writes, NDEF encoding and Creality CFS payload writing are not supported. Note: the existing published release tag is `v.0.6.0`; that spelling is preserved here rather than silently changing the Git tag.


## MakerVault Terminal v0.7 — MakerTags NFC writing

Firmware v0.7 pairs with MakerVault v1.4.5. From MakerVault, select a record and **Use MakerVault Terminal**, confirm the generated HTTPS URL (including any port), present an NTAG215 once and confirm on the handheld. The terminal checks UID, tag capability container and lock/protection status, reviews existing content, writes the canonical `/t/MV-XXXXXXXX/` NFC Forum URL, reads back the memory and acknowledges verification to MakerVault. Cancellation and expiry prevent reuse of stale authorisations. Reassignment preserves the stable URL.

Generic RFID tags (including Creality CFS MIFARE Classic 1K) use UID association and lookup only; they are not reformatted or written by the NTAG215 workflow. NFC writing is enabled by default in v0.7 (`MAKERVAULT_NDEF_WRITER_EXPERIMENTAL=1`); use the v0.7 release's designated verified build, not an older read-only binary. HTTPS trust and `tags:write` permission remain mandatory. For safety requirements and verification, see [writer safety](docs/ntag215-ndef-writer-safety.md) and [physical acceptance](docs/ntag215-esp32s3-acceptance.md).
