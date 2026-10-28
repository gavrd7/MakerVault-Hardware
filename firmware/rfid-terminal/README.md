# MakerVault RFID/NFC handheld terminal

**Current tested baseline: v0.5.1.13.** The source is in [MakerVault_Terminal_Firmware](MakerVault_Terminal_Firmware/); this replaces the obsolete v0.4.0 experimental README.

- [Build and validated wiring](BUILD.md)
- [Hardware acceptance](ACCEPTANCE_v0.5.1.13.md)
- [Pairing protocol](PAIRING_COMPATIBILITY.md)
- [TLS testing](TLS_SECURITY_ACCEPTANCE_v0.5.1.7.md)
- [Third-party QRCode MIT licence](third_party/QRCode-LICENSE.txt)

Use Arduino ESP32S3 Dev Module, core 3.x and a suitable Huge APP partition. The QRCode C files are included in the sketch folder; do not need to be copied manually. Preserve NVS during ordinary updates.

Current capabilities: Wi-Fi provisioning, secure MakerVault pairing, Local CA management, HTTP/TLS diagnostics, background heartbeat, permission status, UID reading, TFT/encoder controls.

In the draft connected-terminal branch, online UID lookup and browser-initiated MakerTag registration/reassignment have been physically tested with staging PR #150, including two-way cancellation and browser session recovery. Existing tag identity is retained during reassignment; tag memory is not written. **Still unavailable:** NDEF and RFID block writing, Creality CFS payload decoding. Final Arduino compilation after the folder rename remains required.

**Security:** Configuration and API requests to MakerVault must use verified HTTPS; the terminal's temporary phone setup portal is local HTTP only, intended for a trusted LAN/AP. NVS persistence should not be described as encrypted at rest. The terminal-side Forget pairing does not revoke the server-side key.
