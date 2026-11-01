# Creality CFS tag reading — isolated implementation track

**Status:** groundwork only; not yet a firmware feature. This branch introduces a read-only parser for an *already decoded* 48-byte CFS material sector, with native regression coverage. It does **not** yet authenticate or decrypt a physical MIFARE Classic tag or resolve it against MakerVault spool inventory.

## Reference and boundaries

Reference: [CFS-RFID-ESP32](https://github.com/gavrd7/CFS-RFID-ESP32), specifically `performTagRead` and `createKey` in the XIAO ESP32-C6 sketch. It authenticates MIFARE Classic sector 1 (blocks 4–6), optionally decrypts it, and reads sector 2 (blocks 8–10) for printer-type text. The first sector uses the `AB124` prefix; material code, RGB hex and weight code are parsed at offsets 12–16, 18–23, and 24–27 respectively.

This code **must not** modify the CFS-RFID-ESP32 project, printer-transfer endpoints, printer material database, or CFS tag sectors. The terminal only reads.

## Remaining implementation before it can be called complete

- Validate licensing and portable AES/key derivation from the CFS reader without copying credentials, printer code or write paths.
- Implement bounded MIFARE Classic Key A authentication, block reads and conditional decrypt on the ESP32-S3 terminal; always stop crypto/finish the tag session, even on errors.
- Verify source tag family, physical UID, sector length and parser output; avoid displaying malformed payload as a successful scan.
- Add a strictly read-only, owner-scoped MakerVault device lookup endpoint matching decoded CFS metadata against physical spool inventory; distinguish matched, ambiguous and unmatched without creating or updating records.
- Integrate a concise TFT scan view with material, colour and spool matching status. Maintain generic UID-only MakerTag workflow independently.
- Add native fixtures (blank, encrypted, unknown, malformed and wrong UID) and CI; then perform physical tests with disposable known CFS tags.

No write commands or printer sync calls are permitted. No automatic database changes.
