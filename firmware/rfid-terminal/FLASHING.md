# Flashing and validating the MakerVault handheld firmware

This guide refers to the physically tested **v0.5.1.13** source at [MakerVault_v0523](MakerVault_v0523/). A later metadata-only **v0.5.1.14** correction is in a **draft PR #3** and has **not** been confirmed hardware-tested. Do not present it as a released or validated build.

## Prepare Arduino IDE

1. Install Arduino IDE 2.x and Espressif's **esp32** board package using Arduino Boards Manager.
2. Select **ESP32S3 Dev Module**. The previously tested developer setup used Arduino IDE **2.3.10** and Arduino-ESP32 **3.3.12**; this is a report of the working setup, **not** a verified reproducible dependency lockfile.
3. Set **USB CDC On Boot = Enabled**, **Huge APP (3MB No OTA / 1MB SPIFFS)** partition scheme (if available for your module), and Serial Monitor **115200 baud**. Match the physical flash size to the module; do not guess other menu settings.
4. Install compatible Adafruit GFX, Adafruit ST7735/ST7789, Adafruit NeoPixel and MFRC522 libraries. The sketch also uses an ESP32 certificate bundle header/library (`esp32_cert_bundle.h`), which must be available in the chosen toolchain. **The exact installed versions of these non-core libraries have not been recovered**; resolve and record them from a successful compile before claiming a fully reproducible release. The QRCode `qrcode.c`/`qrcode.h` sources are bundled beside the sketch with separate MIT attribution.
5. Download the repository ZIP or clone it. Open `firmware/rfid-terminal/MakerVault_v0523/MakerVault_v0523.ino` in Arduino IDE; retain **all seven** source files in the same folder. Confirm the display constructor and GPIO constants still match [WIRING.md](../../hardware/rfid-terminal/WIRING.md).
6. Attach the board via a real USB-C **data cable**, select its serial port, **Verify/Compile**, and then **Upload**. On an existing paired device, do **not** use an “Erase All Flash” option: it may remove Wi-Fi, Local CA and pairing credentials. If the serial port fails to appear, verify the cable/USB port and board connection; check board-specific BOOT/RESET procedures rather than assuming a fixed sequence for every SuperMini variant.

## First boot and functional acceptance

With the enclosure open, watch Serial Monitor at **115200** and confirm TFT splash/menu. In the physically tested v0.5.1.13 baseline, Serial startup states v0.5.1.13 but System info incorrectly shows an older firmware string; heartbeat metadata is also stale. This **does not establish that an older firmware binary is installed**.

1. Confirm screen backlight, orientation, menu text, and **no uncontrolled reset/reboot**.
2. Rotate the encoder both ways, then independently press encoder-select, Back and Confirm.
3. Present a known **MIFARE Classic** tag and an **NTAG215**; confirm local UID/type detection.
4. Enter **Connections → Wi-Fi**; connect to a **2.4 GHz** network using saved-network selection or temporary phone provisioning portal/QR code. The portal is on the *local AP/LAN*, not on a public internet host.
5. In **Connections → MakerVault**, set the **HTTPS** base URL. For a self-signed/private Local CA, import its **public CA certificate** via the setup workflow; never place a CA private key on the device. Run the built-in HTTPS test. No `setInsecure()` fallback should be used.
6. On MakerVault: **Settings → User Account → Connected Devices → Add Device**. Start Pair Device on the handheld, then approve the displayed **eight-digit pairing code** in the web wizard before expiry. Verify paired status and heartbeat; reboot and check persistence. Credentials are stored in device NVS but **NVS encryption/secure boot were not verified**. Terminal-side Forget pairing is local-only; revoke from MakerVault if necessary.

**Current features:** read-only RFID UID detection, secure Wi-Fi/HTTPS/CA setup, pairing, heartbeat. **Not implemented:** tag-to-inventory resolve, tag assignment/writing, Creality CFS payload decode, OTA updates.

## Troubleshooting

| Symptom | Check |
|---|---|
| No port / upload fails | USB data cable, device selection, port permissions, connection seating; board-specific BOOT/RESET only where needed |
| Firmware too large | Confirm selected **Huge APP** partition; verify board flash capacity rather than erasing blindly |
| Blank/dim TFT | Display VCC/GND, reset GPIO43, CS GPIO1, DC GPIO2, shared SPI GPIO12/11, ST7735S controller variant; inspect BLK module circuitry before changing power wiring |
| Reader powered but no UID | RC522 3.3 V/GND, GPIO10 SS, GPIO9 RST, SPI GPIO12/11/13; antenna clearance; check serial reader diagnostic |
| Encoder reversed/bouncing | CLK GPIO4 / DT GPIO5, ground and module contacts; check for loose soldering |
| Back/Confirm don't work | Correct GPIO7/GPIO8, opposite tactile switch contacts to common GND; verify using continuity meter |
| Wi-Fi setup unavailable | Confirm 2.4 GHz, local AP/portal status, power, and local network reachability |
| HTTPS rejected | Validate host/IP against certificate SAN, correct trusted Local CA/public roots, valid device date/time; do **not** disable certificate validation |
| Pairing never gets code | Confirm backend HTTPS reachability and server wizard; inspect TLS diagnostics before retrying |
| Metadata says older version | Known v0.5.1.13 display/heartbeat label bug; v0.5.1.14 correction awaits physical validation |

> **Build reproducibility outstanding:** A hardware user has already confirmed the firmware features work, but an independent clean Arduino install and pinned third-party dependency versions have not yet been verified. No precompiled binary or OTA package is claimed.
