# MakerVault Terminal v0.7 — release notes

Compatible application: **MakerVault v1.4.5**. These notes describe the release after acceptance and publication.

## New
- Single-scan MakerTags registration and verified NTAG215 NFC Forum URI writing, with one confirmation on the handheld.
- Server-generated permanent `/t/MV-XXXXXXXX/` URLs, preserving the MakerTag identity when reassigning records.
- Explicit display of the old and proposed targets during reassignment; protected, unknown and unsupported tag memory is rejected.
- Exact UID, server grant, write-policy and memory validation before programming; independent fresh read-back verification afterwards.
- Browser cancellation and request expiry invalidate stale writing sessions; loss of tag contact fails without false success.
- Improved RFID label centring, long-name layout and unidentified-tag positioning.

## Preserved
- Existing pairing, HTTPS certificate validation, device heartbeat, UID scanning and MakerVault record lookup.
- MIFARE Classic / Creality CFS UID-only lookup without writing their existing material sectors.
- The standard v0.7 source and release binaries include the validated NTAG215 writer by default; a custom read-only build may disable it explicitly.

## Setup / upgrade
1. Update MakerVault to v1.4.5, configure trusted HTTPS (including any non-default port), and verify the terminal is paired.
2. Flash the designated verified v0.7 build; confirm the terminal advertises `ntag215_ndef_write` and grant its `tags:write` permission only to intended devices.
3. Follow [physical acceptance](docs/ntag215-esp32s3-acceptance.md) and [writer safety](docs/ntag215-ndef-writer-safety.md) for supported tags and failure behaviour.
4. Validate an ordinary read, a disposable NTAG215 write and a phone tap before relying on newly written tags.

Do not present generic RFID UID association as NFC memory programming, and never format a Creality CFS tag.
