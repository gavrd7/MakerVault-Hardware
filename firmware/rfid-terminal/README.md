# MakerVault RFID/NFC handheld terminal

**Current tested baseline: v0.5.1.13.** The source is in [MakerVault_v0523](MakerVault_v0523/); this replaces the obsolete v0.4.0 experimental README.

- [Build and validated wiring](BUILD.md)
- [Hardware acceptance](ACCEPTANCE_v0.5.1.13.md)
- [Pairing protocol](PAIRING_COMPATIBILITY.md)
- [TLS testing](TLS_SECURITY_ACCEPTANCE_v0.5.1.7.md)
- [Third-party QRCode MIT licence](third_party/QRCode-LICENSE.txt)

Use Arduino ESP32S3 Dev Module, core 3.x and a suitable Huge APP partition. The QRCode C files are included in the sketch folder; do not need to be copied manually. Preserve NVS during ordinary updates.

Current capabilities: Wi-Fi provisioning, secure MakerVault pairing, Local CA management, HTTP/TLS diagnostics, background heartbeat, permission status, UID reading, TFT/encoder controls.

Not yet available: MakerTag-to-record online lookup, creating MakerTags, assigning tags to records, decoding Creality CFS payload and writing RFID blocks. The source also contains an outdated on-device firmware version label pending a retested metadata-only fix.

**Security:** Configuration and API requests to MakerVault must use verified HTTPS; the terminal's temporary phone setup portal is local HTTP only, intended for a trusted LAN/AP. NVS persistence should not be described as encrypted at rest. The terminal-side Forget pairing does not revoke the server-side key.
