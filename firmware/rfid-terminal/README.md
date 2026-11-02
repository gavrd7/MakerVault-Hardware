# MakerVault Terminal — RFID/NFC handheld

**Firmware source: v1.0 release candidate**, designed for **MakerVault v1.5.0**. Published downloads may be older: check [Releases](https://github.com/gavrd7/MakerVault-Hardware/releases).

The ESP32-S3 handheld provides MakerTag scanning, creation and reassignment; NTAG215 URL writing with explicit overwrite protection; general RFID UID reading; online inventory browsing with A–Z jump; secure MakerVault pairing, heartbeats and certificate-verified HTTPS; and experimental read-only filament RFID decoding.

## Getting started

1. [Build, wiring and assembly](BUILD.md) and [flashing instructions](FLASHING.md).
2. Pair using MakerVault **Settings → Connected Devices**, over verified HTTPS. Grant permissions appropriate to the intended actions.
3. Use **MakerTags → Add MakerTag** for terminal-first assignment, or use browser-led **Use MakerVault Terminal** flows.
4. Scan compatible Creality CFS filament tags under generic **Scan RFID Tag**. The terminal reads colour and material ID locally and, when connected, resolves product details against MakerVault's cached printer material database.
5. To identify a particular physical spool, use its **RFID identity** setting in MakerVault. **Scan using terminal** fills the UID field, which must then be saved explicitly.

## Offline and experimental support

The terminal can still read compatible tag UIDs and decoded CFS material IDs and colours offline. Full material names, manufacturer names and inventory spool associations require a trusted MakerVault connection. An unknown material is shown by its identifier; no product or spool is invented.

**Bambu Lab and Elegoo** filament tag support is experimental and limited by the specific RFID technology, accessible memory, and publicly documented fields. They are not guaranteed to decode on every tag. The terminal does not write third-party filament material tags. See [third-party RFID limitations](../../docs/terminal-third-party-rfid.md).

## Reference

- [Pairing and compatibility](PAIRING_COMPATIBILITY.md)
- [TLS security acceptance](TLS_SECURITY_ACCEPTANCE_v0.5.1.7.md)
- [CFS read-only design and physical tests](../../docs/cfs-readonly-design.md)
- [Hardware project overview](../../README.md)

**Important:** Never erase NVS during a normal update of a paired terminal. Use the board configuration and Arduino-ESP32 version documented in the build instructions. Always validate new firmware on real hardware before publishing a v1.0 release.
