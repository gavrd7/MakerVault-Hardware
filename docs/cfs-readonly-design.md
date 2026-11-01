# Creality CFS RFID — read-only MakerVault Terminal integration

**Development status:** code prepared on [MakerVault-Hardware PR #11](https://github.com/gavrd7/MakerVault-Hardware/pull/11), companion backend on [MakerVault PR #163](https://github.com/gavrd7/MakerVault/pull/163). Neither is released or physically validated. Keep both PRs in draft until the hardware acceptance cases pass.

## Purpose

Use the handheld ESP32-S3 Terminal's existing MFRC522 reader to inspect compatible Creality CFS **MIFARE Classic 1K** filament tags, show material information and resolve an existing physical spool **by the scanned UID** against MakerVault inventory.

This is a read-only alternative to carrying the separate CFS-RFID ESP32-C6 device for scans. **It is not a second CFS tag writer or a substitute for printer material database sync.**

## Implementation

- Normal **Scan RFID Tag** screen: when a compatible MIFARE Classic 1K is selected, attempt a read-only CFS sector inspection. MakerTag registration and NFC writing screens are unchanged.
- Authenticate sector 1 (blocks 4–6) with factory Key A, then try the reference reader's UID-derived Key A on previously personalised tags. Recheck the same UID after reselecting the tag.
- Use the fixed AES-128 constants and block decoding from [CFS-RFID-ESP32](https://github.com/gavrd7/CFS-RFID-ESP32) (XIAO ESP32-C6 sketch and AES source), implemented using the ESP32's mbedTLS rather than copying the original AES implementation. No credentials or CFS printer code are reused.
- Decode the `AB124` signature, five-character material code, six-character colour hex and weight code from the 48-byte sector. Reject malformed/unsupported data instead of displaying it as CFS.
- Display friendly manufacturer/material names when available from the read-only CFS presentation catalogue, plus a compact RGB565 rounded colour swatch, matching hex, UID and spool-link status. Keep the raw ID for unknown materials and never infer a name from an arbitrary tag ID. A background **GET** uses the paired MakerVault HTTPS connection and `inventory:read` scope, with the existing endpoint and device-owner permission guards.
- The companion server endpoint `/api/device/v1/spools/by-rfid/?uid=...` looks up physical `Spool.rfid_uid`, returning an owner-scoped matched/unmatched response; it never associates by material/colour alone and never creates/changes records.

### Security boundaries

Only Key A authentication and MIFARE **read** operations are used. No `MIFARE_Write`, CFS-sector writes, printer database reads/writes, printer push/pull, or automatic inventory changes. Server-side matching does not trust the tag as authentication: CFS tag sectors and IDs can be cloned or reprogrammed.

If the read fails or the tag is not recognised as CFS, normal generic RFID UID handling remains available. If the server is unavailable or lacks `inventory:read`, the tag's decoded metadata can still be displayed; the spool is shown as unavailable rather than inventing a match. An existing linked `Spool.rfid_uid` is required for a match. The firmware does not try to pair an unlinked spool automatically.

## Automated checks

- Native CFS parser fixtures: signature, lengths, material/colour, weight codes and invalid payload rejection.
- Standard ESP32-S3 sketch compilation and existing hardware logic regression suite.
- Backend tests: owner isolation, valid/invalid UID, matched/unmatched spool, `inventory:read` bearer scope, account view permission and no session-cookie authentication.

## Physical acceptance required before merging

- [ ] Read a known **unpersonalised** compatible test CFS tag; verify material ID and exact colour against the dedicated CFS reader. For ID `10437`, verify `eSUN / PLA Basic / #E01B24` and the rounded swatch.
- [ ] Read a known **personalised/encrypted** compatible CFS tag; confirm computed-key authentication and decrypted sector fields.
- [ ] Present a generic blank MIFARE Classic tag; ensure it remains UID-only and no false CFS details are shown.
- [ ] Confirm a correctly linked UID displays the expected MakerVault spool; unlinked UID displays **Spool not linked**.
- [ ] Confirm a tag belonging to another user's spool never discloses that spool.
- [ ] Disconnect Wi-Fi / invalidate HTTPS trust; tag reading remains safe and the inventory lookup reports unavailable.
- [ ] Scan NTAG215 and use MakerTag create/write workflows to confirm no regression.
- [ ] Confirm no Creality CFS sector has been modified and no printer database sync/push took place.

**No action is required on the user's CFS ESP32-C6 firmware or the printer.**
