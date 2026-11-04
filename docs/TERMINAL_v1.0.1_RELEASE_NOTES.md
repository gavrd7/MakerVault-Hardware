# MakerVault Terminal v1.0.1

This maintenance release packages the validated v1.0 firmware with its corrected **1.0.1** version identifier. The release-tag validation requires the compiled firmware version to match the GitHub tag. No new terminal features were introduced solely by this version correction.

The first full release of the handheld MakerVault RFID/NFC terminal, intended for MakerVault v1.5.0.

## Available now

- Secure, scoped pairing to your self-hosted MakerVault instance; trusted HTTPS, local CA, Wi-Fi setup and connection status
- Native ESP-IDF `esp_http_client` transport with persistent TLS, unified five-second pending-work polling, controlled reconnection and C++ worker-buffer cleanup. The maintainer's two-hour-plus idle test did not reproduce the previous ~96-byte-per-request heap leak
- MakerTag scanning, assignment and reassignment, inventory browsing with A–Z jump, and stable MakerVault identifiers
- Verified NTAG215 MakerTag URL writing with overwrite confirmation and cancellation protection
- **Clean MakerTag**: clear and verify the physical NTAG215 NDEF user data while keeping the MakerTag record; **Remove MakerTag**: remove only the registration, or opt to clean and verify the same tag before removal. Requires the matching MakerVault app implementation in PR #165
- Browser-initiated terminal scans and physical spool UID registration
- Read-only Creality CFS filament ID, HEX colour and physical spool UID lookup, including offline fallback and 1- or 2-tag MakerVault spool identification
- **Experimental** Bambu Lab MIFARE Classic and Elegoo NTAG RFID readers, based on public formats. Their decoders have synthetic software tests, **not real manufacturer tag validation**. A protected or unknown tag can fall back to generic UID display. No Bambu or Elegoo tag writing.

## Validation and compatibility

The maintainer reports the full release acceptance checklist passed on physical ESP32-S3 hardware: TLS and pairing security, Wi-Fi/server recovery, MakerTag cancellation and wrong-UID protection, general RFID, UI responsiveness, and a sustained idle memory test. The stable heap in the multi-hour run applies to the tested firmware/network combination; continue to monitor field deployments. MakerTag clean/remove and the consolidated `/api/device/v1/pending-work/` endpoint require the matching MakerVault app branch or a published release containing it. No WebSockets, automatic printer database writes, OTA or third-party filament tag writes are claimed.

## Downloads

The release publishing action generates:

- `MakerVault-Terminal-v1.0.1-Firmware.zip` — firmware, bootloader and partition binaries, with flashing manifest
- `MakerVault-Terminal-v1.0.1-App.bin` — application binary for suitable existing installations
- `MakerVault-Terminal-v1.0.1-Arduino-Sketch.zip` — complete sketch and source; external Arduino libraries remain separate
- `MakerVault-Terminal-v1.0.1-STL.zip` and `MakerVault-Terminal-v1.0.1-STEP.zip` — unchanged reusable enclosure assets from an earlier release
- `SHA256SUMS.txt` and `CAD-SOURCE.txt` — integrity and CAD provenance

**Flashing:** verify the ESP32-S3 hardware and partition configuration. Normal firmware updates must not erase NVS, which holds Wi-Fi/certificates/pairing information. Published binaries must be built from the final `v1.0.1` tag. If asset automation fails, do not describe the release as having complete downloads until corrected.

See the README, firmware flashing guide and the MakerVault v1.5 user guide for setup and limitations.
