# ESP32-S3 NFC writing — first hardware acceptance

This procedure applies only to the MakerVault ESP32-S3 handheld terminal.
It is **not** a released or production-approved writer.

## Before flashing

- Keep a copy of the last known working Arduino sketch.
- Build from the draft hardware PR with the experimental NFC writer explicitly enabled.
- Confirm full Arduino compilation passes. Preserve Wi-Fi/pairing settings; never erase all flash to test this feature.
- Stage MakerVault's corresponding NFC backend PR on a disposable/staging installation.
- Use only spare NTAG215 tags; **do not write existing important tags** until inspection diagnostics pass.

## 1. Regression

Boot; check TFT, encoder, Back and Confirm, Wi-Fi, TLS pairing, heartbeats,
ordinary UID scan, browser-requested registration and lookup.

## 2. Read-only pre-write inspection

Create/assign a MakerTag using the unified Add MakerTag flow, confirm its
seven-byte UID, then select Write / overwrite NFC tag -> paired ESP32-S3.
Check the preview contains the staging HTTPS scheme, address, port and
`/t/MV-XXXXXXXX/` identity. Start the request and present the tag once.

**Do not press physical Confirm yet.** Record the *exact* display result:
- `Confirm physical write?`: inspection accepted;
- `Wrong tag UID`: wrong physical tag;
- `Cannot read tag config`, `Not NTAG215 (CC)`,
  `Locked / protected tag`, `Cannot read tag memory`:
  collect a serial log (115200 baud) and keep physical writing disabled;
- `Existing NDEF needs override`: data does not match the existing
  MakerVault identity, so it is **intentionally not overwritten**.

The same UID being registered in MakerVault does not establish any fact
about the chip's current NDEF content. Never bypass inspection failures.

## 3. Write only after read-only acceptance

On a disposable empty NTAG215, verify UID and URL again, press Confirm,
remove and re-present the tag when prompted, and inspect the completion
state. Independently read with a phone and verify the staging record.
Abort and capture logs on any mismatch.

## 4. Overwrite cases

Same MakerTag NDEF URL may be updated after explicit browser request and
physical confirmation. Foreign content or a different identity must not
be overwritten silently. Additional authorization/override UI must be
reviewed before claiming broad overwrite support.

## Acceptance criteria

No writes occur during scanning, lookup, registration, initial inspection,
cancel, wrong UID, expired request, locked tag, or failed TLS. The terminal
must not report completion until its own fresh read-back matches the exact
authorized UID and URL and the staging server acknowledges completion.
