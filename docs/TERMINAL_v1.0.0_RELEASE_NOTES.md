# MakerVault Terminal v1.0.0

The first full release of the handheld MakerVault RFID/NFC terminal, intended for MakerVault v1.5.0.

## Available now

- Secure, scoped pairing to your self-hosted MakerVault instance; trusted HTTPS, local CA, Wi-Fi setup and connection status
- MakerTag scanning, assignment and reassignment, inventory browsing with A–Z jump, and stable MakerVault identifiers
- Verified NTAG215 MakerTag URL writing with overwrite confirmation and cancellation protection
- Browser-initiated terminal scans and physical spool UID registration
- Read-only Creality CFS filament ID, HEX colour and physical spool UID lookup, including offline fallback and 1- or 2-tag MakerVault spool identification
- **Experimental** Bambu Lab MIFARE Classic and Elegoo NTAG RFID readers, based on public formats. Their decoders have synthetic software tests, **not real manufacturer tag validation**. A protected or unknown tag can fall back to generic UID display. No Bambu or Elegoo tag writing.

## Downloads

The release publishing action generates:

- `MakerVault-Terminal-v1.0.0-Firmware.zip` — firmware, bootloader and partition binaries, with flashing manifest
- `MakerVault-Terminal-v1.0.0-App.bin` — application binary for suitable existing installations
- `MakerVault-Terminal-v1.0.0-Arduino-Sketch.zip` — complete sketch and source; external Arduino libraries remain separate
- `MakerVault-Terminal-v1.0.0-STL.zip` and `MakerVault-Terminal-v1.0.0-STEP.zip` — unchanged reusable enclosure assets from an earlier release
- `SHA256SUMS.txt` and `CAD-SOURCE.txt` — integrity and CAD provenance

**Flashing:** verify the ESP32-S3 hardware and partition configuration. Normal firmware updates must not erase NVS, which holds Wi-Fi/certificates/pairing information. Published binaries must be built from the final `v1.0.0` tag. If asset automation fails, do not describe the release as having complete downloads until corrected.

See the README, firmware flashing guide and the MakerVault v1.5 user guide for setup and limitations.
