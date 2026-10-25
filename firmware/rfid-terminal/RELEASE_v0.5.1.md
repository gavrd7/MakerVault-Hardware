# MakerVault RFID terminal — v0.5.1 integration candidate

Status: **experimental, physical testing required**. The locally prepared `MakerVault_Handheld_v0510.zip` is the candidate Arduino source package; do not publish a stable firmware release from this branch before the source package has been committed and Arduino compilation/hardware tests have passed.

## Baseline

Based on tested v0.5.0 firmware and the v0.5.0.1 heartbeat adjustment. ESP32-S3 SuperMini; RC522 and 1.44-inch ST7735S 128×128 TFT; common hardware SPI SCK 12, MOSI 11, MISO 13; TFT CS 1/DC 2/RST 43; RC522 CS 10/RST 9; encoder 4/5/6; Back 7; Confirm 8. Preserve multi-network Wi-Fi and the backend pairing protocol from MakerVault PR #146.

## Planned v0.5.1 UI changes

- On confirmed authenticated `/api/device/v1/me/` HTTP 200, pairing screen shows **Connected! / MakerVault ready** and automatically returns to Connections after brief confirmation. A paired device whose API verification fails must not show the success message or discard its saved credential.
- Main menu: **MakerTags**, **RFID Tags**, **Connections**, **System info**. Remove the recent-tag menu option.
- MakerTags submenu: **Scan MakerTag** and **Create MakerTag**. RFID submenu: **Read RFID tag** and **Write RFID tag**.
- Create MakerTag and Write RFID tag are **disabled placeholders** in this candidate — no tag writes are performed.
- Scrollable detail pages with word-wrap, character fallback for long URLs, scrollbar and encoder-driven scrolling while preserving navigation semantics for menus.
- Keep Arduino IDE flash erasure disabled to retain saved SSIDs, CA trust and device credentials.

## Security, documentation and acceptance gates

- Use the canonical [MakerVault pairing contract](https://github.com/gavrd7/MakerVault/blob/feature/143-connected-device-pairing/docs/CONNECTED_DEVICES_PAIRING.md): code issued by backend, one-time proof and credential, verified HTTPS only, no secrets in logs.
- Verify code-based pairing, reboot persistence, heartbeat, permission changes, revocation, Wi-Fi loss/recovery, and RC522 scanning on real hardware.
- Create MakerTag requires its own backed, permission-scoped record registration workflow; physical writing is a separate explicit-confirm action and must not overwrite Creality CFS material data.
- Add finished enclosure STLs/source, assembly guide and real device photos before public publication.

## Pending

Upload the exact tested Arduino source files from `MakerVault_Handheld_v0510.zip` to `firmware/rfid-terminal/MakerVault_v0510/` after laptop compilation and validation. Do not treat this documentation PR as a completed firmware release.

## Bundled QR generator (third party)

The firmware folder includes `qrcode.c` and `qrcode.h` from [ricmoo/QRCode](https://github.com/ricmoo/QRCode), licensed under the MIT License (copyright Richard Moore and Project Nayuki). The full original copyright and licence notice is retained at `firmware/rfid-terminal/third_party/QRCode-LICENSE.txt` and in the sources. These files are vendored so Arduino compiles the QR implementation beside the `.ino`, without manually copying source files into each sketch. Keep the copyright and licence in any redistribution. Other Arduino/ESP32 dependencies still need their normal installation.

Note: At present this review branch includes the QR source and metadata; the complete v0.5.1.1 sketch has not yet been committed to GitHub. Do not mark the firmware release complete until the full sketch is present and verified.
