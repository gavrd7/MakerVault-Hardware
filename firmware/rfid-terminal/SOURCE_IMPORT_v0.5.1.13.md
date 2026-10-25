# Source import instructions — v0.5.1.13

The verified local firmware artifact is `MakerVault_Handheld_v0523_release_candidate.zip` supplied in the terminal development conversation.

Expected project folder:

```text
firmware/rfid-terminal/MakerVault_v0523/
  MakerVault_v0523.ino
  brand_images.h
  maker_backend.h
  provisioning.h
  ui_icons.h
  qrcode.c
  qrcode.h
  README.txt
  TESTING.md
```

Import **exactly** the source files from the tested archive without changing runtime logic. QRCode C files must come from `ricmoo/QRCode` with the original MIT copyright and licence retained at `firmware/rfid-terminal/third_party/QRCode-LICENSE.txt`.

**Security:** Never commit real Wi-Fi passwords, device API keys, `claim_secret`, production CA private keys or diagnostic dumps containing credentials.

After import: compile for ESP32S3 Dev Module using the documented core and Huge APP layout; hardware regression test any source edits. Update root and firmware README to current status. Review PR diff before merge.

This document does not imply the firmware source has been uploaded.
