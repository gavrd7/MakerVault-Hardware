MakerVault RFID/NFC Terminal — Arduino sketch
===========================================

Open MakerVault_Terminal_Firmware.ino in Arduino IDE with every file in this
directory kept alongside it. See ../../BUILD.md and ../../FLASHING.md for the
tested board setup, libraries, wiring, HTTPS/CA setup and flashing instructions.

Hardware: ESP32-S3 SuperMini, RC522 SPI, ST7735S 128x128 TFT, encoder, Back
and Confirm controls. The published firmware baseline on main is v0.5.1.14;
this draft branch contains additional connected MakerTag features and retains
the legacy internal version string until a separately tested version bump.

Works with MakerVault app PR #150:
- Read a tag UID and look it up against the connected MakerVault instance.
- From MakerVault's Add Maker Tag screen, choose an inventory item/spool and
  request a scan on the paired terminal. Confirm assignment on the device.
- For an already registered UID assigned elsewhere, the terminal shows the
  current and proposed targets and requires a second explicit Confirm to
  reassign. The physical UID and MakerVault ID are preserved.
- Press Back to cancel a pending request. Browser cancellation also returns
  the terminal to its menu. Browser refresh can resume a pending session.
- A normal lookup does not create or alter a MakerTag.

Tag memory is NOT written. NDEF, MIFARE blocks, and Creality CFS payload
reading/writing are NOT implemented. Exact NTAG215 model verification is NOT
performed by the current RC522 family identification.

Use verified HTTPS; never use insecure TLS. Wi-Fi, CA and pairing credentials
live in NVS: do not select Erase All Flash on ordinary updates. NVS encryption
has not been verified. QRCode qrcode.c/qrcode.h retain third-party MIT notice.

Draft release gate: compile the RENAMED sketch and run final regression tests
before merging PR #6 or app PR #150.

NEXT RELEASE CANDIDATE: v0.6.0 (PR #6). The sketch reports v0.6.0; previously published firmware v0.5.1.14 remains the stable baseline until release. Retest flashing and the displayed version after this version-only change. No NDEF/CFS tag-memory writing.
