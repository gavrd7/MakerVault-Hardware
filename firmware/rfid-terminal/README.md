# MakerVault Terminal — RFID/NFC handheld

**Firmware source: v1.0.0 release candidate**, designed for the matching **MakerVault v1.5.0** app branch. Check [Releases](https://github.com/gavrd7/MakerVault-Hardware/releases) for the actual published tag and binaries. Published downloads may be older: check [Releases](https://github.com/gavrd7/MakerVault-Hardware/releases).

The ESP32-S3 handheld provides MakerTag scanning, creation and reassignment; NTAG215 URL writing with explicit overwrite protection; general RFID UID reading; online inventory browsing with A–Z jump; secure MakerVault pairing, heartbeats and certificate-verified HTTPS; and experimental read-only filament RFID decoding.

## Getting started

1. [Build, wiring and assembly](BUILD.md) and [flashing instructions](FLASHING.md).
2. Pair using MakerVault **Settings → Connected Devices**, over verified HTTPS. Grant permissions appropriate to the intended actions.
3. Use **MakerTags → Add MakerTag** for terminal-first assignment, or use browser-led **Use MakerVault Terminal** flows.
4. Scan compatible Creality CFS filament tags under generic **Scan RFID Tag**. The terminal reads colour and material ID locally and, when connected, resolves product details against MakerVault's cached printer material database.
5. To identify a particular physical spool, use its **RFID identity** setting in MakerVault. **Scan using terminal** fills the UID field, which must then be saved explicitly.

## Secure networking and reliability

The terminal uses native ESP-IDF `esp_http_client` with a reusable, certificate-verified HTTPS connection to the existing MakerVault REST API. Browser-initiated work is discovered through a single five-second `/api/device/v1/pending-work/` poll; heartbeat and permission verification use the same managed connection. All existing physical confirmations, UID matching, cancellation and operation-state checks remain required. A Wi-Fi interruption, invalid certificate, change of approved origin or server restart must not silently bypass security or pair a different server.

The earlier persistent ~96-byte-per-request heap decline is not reproduced in the maintainer's two-hour-plus v1.0.0 validation run, after switching transport and ensuring that worker-owned `String` buffers are destroyed before FreeRTOS tasks exit. The terminal still retains safety recovery and low-noise heap diagnostics; report sustained declines or unexplained resets against the [hardware repository](https://github.com/gavrd7/MakerVault-Hardware/issues).

**Clean and remove:** `Home → MakerTags → Clean MakerTag` clears supported, unlocked NTAG215 NDEF user data **without deleting the MakerTag record**. `Remove MakerTag` can remove the server record alone or clean the physical tag first; a failed or unverified clean must never delete the record. These actions require the companion app PR #165 (or a release containing it). Never use them on third-party filament tags. See the [release notes](../../docs/TERMINAL_v1.0.0_RELEASE_NOTES.md).

## Offline and experimental support

The terminal can still read compatible tag UIDs and decoded CFS material IDs and colours offline. Full material names, manufacturer names and inventory spool associations require a trusted MakerVault connection. An unknown material is shown by its identifier; no product or spool is invented.

**Bambu Lab and Elegoo** filament tag support is experimental and limited by the specific RFID technology, accessible memory, and publicly documented fields. They are not guaranteed to decode on every tag. The terminal does not write third-party filament material tags. See [third-party RFID limitations](../../docs/terminal-third-party-rfid.md).

## Reference

- [Pairing and compatibility](PAIRING_COMPATIBILITY.md)
- [TLS security acceptance](TLS_SECURITY_ACCEPTANCE_v0.5.1.7.md)
- [CFS read-only design and physical tests](../../docs/cfs-readonly-design.md)
- [Hardware project overview](../../README.md)

**Important:** Never erase NVS during a normal update of a paired terminal. Use the board configuration and Arduino-ESP32 version documented in the build instructions. The maintainer reports that the v1.0.0 release test matrix passed on physical hardware. Verify the exact final tagged release assets and installed version before publication.
