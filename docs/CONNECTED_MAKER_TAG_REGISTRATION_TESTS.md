# Connected Maker Tag registration — hardware acceptance

This draft implements browser-initiated UID registration on the existing Rev C.3
ESP32-S3 / RC522 handheld. It does **not** write tag memory or decode Creality
CFS sector data.

## Intended tag families

- NTAG215 for Maker Tags. RC522 currently recognizes the broader MIFARE
  Ultralight / ISO 14443A Type 2 family, **not the exact NTAG215 model**;
  GET_VERSION/model verification remains outstanding.
- MIFARE Classic 1K for Creality CFS spools. Recognition of the Classic 1K
  family does not by itself validate Creality-specific data.

Other card families are rejected by the new connected-registration flow. The
existing standalone scan path is unchanged.

## Hardware verification checklist

1. Build on Arduino-ESP32 version documented in repository; confirm the
   header `connected_registration.h` is present beside the sketch.
2. Connect to staging over verified HTTPS; pair terminal with `tags:write`.
3. Create an inventory item and start **Use RFID terminal** in staging.
4. Verify prompt shows the correct record. Back should dismiss; it must not
   submit an identity.
5. Start a new request. Present NTAG215; verify UID display, require physical
   Confirm, and verify browser completion and one recorded tag.
6. Repeat with MIFARE Classic 1K on a separate test record; confirm family and
   exact normalized UID.
7. Rescan/retry an existing UID across NFC/RFID legacy kinds. Reassignment to a **different** record must show the original and proposed target on the terminal, require a second explicit Confirm, and keep the UID and MakerVault ID. Scanning against the same record must not modify the tag. Back cancels without reassignment.
8. Test cancellation from **either** handheld or browser, browser refresh and resuming a pending request, expiry, wrong device, offline Wi-Fi and untrusted CA.
   Registration must not bypass TLS or disturb existing pairing/heartbeat.
9. Verify normal standalone tag scan and button navigation still work.

9. Verify unregistered tag icon and text wrapping, and repeat lookups without alternating an offline result.
10. Confirm tag history contains the previous and new target and that the original MakerVault ID remains unchanged.

**Do not merge or release firmware before a clean Arduino IDE compile after the sketch-folder rename and final regression acceptance.**
