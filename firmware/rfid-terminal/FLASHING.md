# Flashing and validating the MakerVault handheld firmware

This guide covers the **MakerVault Terminal v1.0.0 release-candidate source** for the matching **MakerVault v1.5.0** app. For published downloads, verify the final release tag and assets on [MakerVault-Hardware Releases](https://github.com/gavrd7/MakerVault-Hardware/releases). Earlier v0.5/v0.6 limitations no longer describe the current source.

## Prepare Arduino IDE

1. Install Arduino IDE 2.x and Espressif's **esp32** board package using Arduino Boards Manager.
2. Select **ESP32S3 Dev Module**. The previously tested developer setup used Arduino IDE **2.3.10** and Arduino-ESP32 **3.3.12**; this is a report of the working setup, **not** a verified reproducible dependency lockfile.
3. Set **USB CDC On Boot = Enabled**, **Huge APP (3MB No OTA / 1MB SPIFFS)** partition scheme (if available for your module), and Serial Monitor **115200 baud**. Match the physical flash size to the module; do not guess other menu settings.
4. Install compatible Adafruit GFX, Adafruit ST7735/ST7789, Adafruit NeoPixel and MFRC522 libraries. The sketch also uses an ESP32 certificate bundle header/library (`esp32_cert_bundle.h`), which must be available in the chosen toolchain. **The exact installed versions of these non-core libraries have not been recovered**; resolve and record them from a successful compile before claiming a fully reproducible release. The QRCode `qrcode.c`/`qrcode.h` sources are bundled beside the sketch with separate MIT attribution.
5. Download the repository ZIP or clone it. Open `firmware/rfid-terminal/MakerVault_Terminal_Firmware/MakerVault_Terminal_Firmware.ino` in Arduino IDE; retain **all sketch source and header files** in the same folder. Confirm the display constructor and GPIO constants still match [WIRING.md](../../hardware/rfid-terminal/WIRING.md).
6. Attach the board via a real USB-C **data cable**, select its serial port, **Verify/Compile**, and then **Upload**. On an existing paired device, do **not** use an “Erase All Flash” option: it may remove Wi-Fi, Local CA and pairing credentials. If the serial port fails to appear, verify the cable/USB port and board connection; check board-specific BOOT/RESET procedures rather than assuming a fixed sequence for every SuperMini variant.

## First boot and functional acceptance

With the enclosure open, watch Serial Monitor at **115200** and confirm TFT splash/menu. For the previous v0.5.1.14 firmware, System info and heartbeat version reporting have been confirmed by the owner.

1. Confirm screen backlight, orientation, menu text, and **no uncontrolled reset/reboot**.
2. Rotate the encoder both ways, then independently press encoder-select, Back and Confirm.
3. Present a known **MIFARE Classic** tag and an **NTAG215**; confirm local UID/type detection.
4. Enter **Connections → Wi-Fi**; connect to a **2.4 GHz** network using saved-network selection or temporary phone provisioning portal/QR code. The portal is on the *local AP/LAN*, not on a public internet host.
5. In **Connections → MakerVault**, set the **HTTPS** base URL. For a self-signed/private Local CA, import its **public CA certificate** via the setup workflow; never place a CA private key on the device. Run the built-in HTTPS test. No `setInsecure()` fallback should be used.
6. On MakerVault: **Settings → User Account → Connected Devices → Add Device**. Start Pair Device on the handheld, then approve the displayed **eight-digit pairing code** in the web wizard before expiry. Verify paired status and heartbeat; reboot and check persistence. Credentials are stored in device NVS but **NVS encryption/secure boot were not verified**. Terminal-side Forget pairing is local-only; revoke from MakerVault if necessary.

**v1.0.0 capabilities with MakerVault v1.5.0:** paired HTTPS using ESP-IDF persistent client, MakerTag scanning and record browsing, on-device/browser-led assignment and reassignment, verified NTAG215 NDEF URL writing, clean and clean-on-remove with physical confirmation, read-only Creality CFS decoding and experimental read-only Bambu/Elegoo paths. **Not implemented:** OTA and third-party filament-tag writing. Clean/remove requires the matching app PR #165 or an app release containing it.

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
| Metadata says older version | Confirm you are flashing v0.6.0 from the current source, not the older v0.5.1.13 or v0.5.1.14 releases |

> **Release-candidate status:** The maintainer reports all release regression tests passed, including HTTPS reliability, certificate rejection, connectivity recovery and physical MakerTag safety. CI compiles this source. The public release binaries and sketch packages must still be checked after a final tag is published; do not assume draft-branch artifacts are release assets.

## v1.0.0 verification and endurance

1. Flash the final v1.0.0 build without erasing NVS. Check **v1.0.0** in System info, serial startup and heartbeat.
2. Confirm the terminal pairs or reconnects with validated HTTPS (never insecure TLS), resolves `/api/device/v1/me/`, and reports heartbeat HTTP 200.
3. Watch `[TLS SESSION]` for `backend=ESP-IDF` and reuse of an established connection, and `[HEAP IDLE]` for a stable baseline with normal task count. Small per-request `[TLS HEAP]` negative deltas are temporary buffers, not proof of leakage.
4. Verify Wi-Fi loss and MakerVault HTTPS restart recover without re-pairing. A different/untrusted CA and mismatched hostname must be rejected.
5. On disposable NTAG215 tags, validate creation, overwrite confirmation, wrong-tag rejection, cancellation, clean without deleting the registration and clean-on-remove with verified erase before server removal.
6. Verify UID-only RFID and supported read-only filament tag paths, record browsing and menu responsiveness.
7. Verify a complete multi-hour idle run with no sustained free-heap decline and no unplanned recovery reboot. The maintainer has reported the full release test matrix passed; always retest following any firmware changes.

Never erase a registered production tag while testing.
