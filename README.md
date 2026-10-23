# MakerVault Hardware

**Private development repository — not production ready.**

Firmware, hardware files and shared device integrations for [MakerVault](https://github.com/gavrd7/MakerVault).

## Devices
- **RFID/NFC handheld terminal** — ESP32-S3 SuperMini, MFRC522, ST7735S TFT, KY-040 encoder, Back and Confirm buttons.
- **Filament weigh station** — planned; reuses provisioning and MakerVault device authentication.

## Development status
- **Verified hardware baseline:** Rev C.3 shared SPI, encoder/buttons, TFT and RC522 tag reads.
- **Firmware UI:** v0.3.10 contactless icon build; hardware validation of that specific version pending.
- **Experimental:** v0.4.0 Wi-Fi provisioning AP and QR setup portal. **Compile issue:** requires Richard Moore's `QRCode` library providing `qrcode_initText`. Do not deploy until compiled and security-reviewed.
- **Backend:** [MakerVault device pairing wizard issue #143](https://github.com/gavrd7/MakerVault/issues/143).

## Planned repository layout
```text
firmware/rfid-terminal/
firmware/filament-weigh-station/
shared/provisioning/
shared/device-api/
shared/credentials/
shared/device-protocol/
hardware/rfid-terminal/
hardware/filament-weigh-station/
docs/
tests/
.github/workflows/
```

Keep working releases and experiments separate. Never commit Wi-Fi credentials, MakerVault API keys, production certificates or other secrets.

## Release
Keep this repository private until integrated, tested and documented. Select a license before publication.
