# Handheld terminal v0.5.1.13 — physical acceptance

**Status: tested release candidate, not yet a published source release.**

The owner reported passing real ESP32-S3 tests for:

- Normal boot and splash; ST7735S 128×128 UI, encoder, buttons and scrolling
- RC522 MIFARE Classic and NTAG215 UID reading
- Wi-Fi connect/disconnect and restoration
- Pairing-v1 eight-digit code through MakerVault staging PR #146
- HTTPS verification using MakerVault Local CA
- Rejection of unrelated CA with native mbedTLS `-0x2700`
- Restoring correct CA without re-pairing
- Changing and restoring the server URL without losing saved credentials
- Guarding stored credentials from automatic use at a changed URL
- Local custom CA removal and import
- Reboot persistence, automatic authenticated reconnect and background heartbeat
- Failure reporting when HTTPS trust is not configured
- Removal of temporary screenshot functionality from the release candidate

**Caveats:** No full Arduino CI compilation is documented; firmware source has not yet been committed here. Online tag-to-record resolution, MakerTag creation, physical RFID writing and Creality CFS payload decoding are not implemented in the inspected v0.5.1.13 scan routine. Local-only forgetting of pairing does not revoke the server credential. Secure boot/flash encryption are not demonstrated.

**Version metadata inconsistency:** The 0.5.1.13 startup version and displayed System info / heartbeat version differ in the tested source. Correct in a follow-up build and retest before marking the shipped firmware stable; do not silently alter the tested binary baseline.

**Merge gate:** Commit the exact tested `MakerVault_v0523.ino` and accompanying headers before merging this PR; bundle compatible QRCode sources, keep full MIT notice, ensure Arduino project folder is self-contained. Verify secret-free diff.
