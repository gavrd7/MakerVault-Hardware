# RFID/NFC handheld terminal

**Hardware:** ESP32-S3 SuperMini + MFRC522 (SPI) + ST7735S 128×128 TFT + KY-040 encoder + two buttons.

## Validated Rev C.3 GPIO
| Function | GPIO |
|---|---|
| Shared SPI SCK / MOSI / MISO | 12 / 11 / 13 |
| TFT CS / DC / RST | 1 / 2 / 43 |
| RC522 CS / RST | 10 / 9 |
| Encoder CLK / DT / push | 4 / 5 / 6 |
| Back / Confirm | 7 / 8 |
| TFT BLK | 3V3 directly |

Preserve single hardware SPI bus and deselect both chip-select lines before initialisation.

## Firmware status
- v0.3.3: proven reader and display SPI integration.
- v0.3.10: UI baseline (requires validation).
- v0.4.0: **experimental provisioning**, has not passed Arduino compilation yet.

### v0.4.0 QR-code dependency
The provisioning code uses **Richard Moore (ricmoo)'s** [QRCode library](https://github.com/ricmoo/QRCode), with its C API `QRCode`, `qrcode_getBufferSize`, `qrcode_initText`, `ECC_LOW`, and `qrcode_getModule`. A different installed `qrcode.h` that exposes only `esp_qrcode_*` is **incompatible**.

Install the correct library and resolve header collisions before compiling. Do not remove the QR functionality just to get a green build.

No device secrets should be committed to this repository.
