# MakerVault Hardware

**Build, flash and use connected hardware for [MakerVault](https://github.com/gavrd7/MakerVault).** This repository contains open-source firmware, 3D-printable enclosures, hardware assembly instructions, wiring diagrams and practical testing documentation.

The first supported project is the **MakerVault Terminal**: a handheld RFID/NFC reader and MakerTag writer that connects securely to your self-hosted MakerVault installation.

**Current integration:** MakerVault **v1.4.6** + MakerVault Terminal **v0.8**. The repository contains the v0.8 source; check [Releases](https://github.com/gavrd7/MakerVault-Hardware/releases) for published binaries and source archives.

## MakerVault Terminal

A compact, 3D-printable handheld with a 128×128 colour display, rotary encoder, Back and Confirm buttons, and an RFID/NFC reader housed behind the enclosure. It can look up physical tags, browse MakerVault records, and register or reassign MakerTags without needing to start the workflow on a computer.

### Current capabilities

| Area | What it does |
| --- | --- |
| **Wi-Fi and setup** | Connects to 2.4 GHz Wi-Fi, remembers multiple networks, and offers a temporary access point / QR-assisted phone setup |
| **Secure connection** | Uses certificate-verified HTTPS, including a trusted local CA where required; supports connection diagnostics and an approved server endpoint |
| **Pairing and management** | Pairs through MakerVault's eight-digit Connected Devices wizard; uses scoped, revocable credentials and reports device heartbeat |
| **Read and look up tags** | Scans compatible RFID/NFC UIDs and resolves registered MakerTags through MakerVault |
| **Create MakerTags on the terminal** | Browse **Inventory, Location, Printed Part, Printer, Project, Reusable Spool and Spool** directly on the handheld |
| **Browse large inventories** | Uses alphabetically sorted, paged server results, record IDs and a hold-to-open **A–Z letter jump**; no on-screen keyboard or full inventory download |
| **Assign and reassign** | Assigns a new tag, recognises an existing MakerTag, and preserves its stable `MV-XXXXXXXX` identity on reassignment |
| **NTAG215 NFC writing** | Writes and verifies the permanent MakerVault HTTPS record URL on supported writable NTAG215 tags |
| **Protected URL replacement** | Shows an additional **Back / Confirm** prompt before replacing a recognised unrelated HTTP(S) NFC URL; Back leaves it unchanged |
| **Safety and recovery** | Enforces physical UID matching, tag compatibility/lock checks, server authorisation and expiry; supports cancellation and retry after incomplete writes |
| **Other RFID tags** | Supports UID-only lookup/assignment for compatible MIFARE Classic 1K tags without modifying their sectors |

The browser-led **Use MakerVault Terminal** workflow is also supported. Terminal-first registration and browser-led registration are complementary options.

### A typical on-device workflow

1. Pair the terminal to MakerVault over **trusted HTTPS** and grant the required device scopes and account permissions.
2. Open **Home → MakerTags → Add MakerTag**.
3. Choose a record category, browse the list or hold the encoder for the large **A–Z jump**, and review the selected record.
4. Present the NFC/RFID tag and approve the assignment as prompted.
5. For a supported writable NTAG215, approve replacement separately if it already holds an unrelated URL, then keep the same tag near the reader until **MakerTag ready** appears.
6. Scan the completed NTAG215 with a phone to open its stable MakerVault URL. The phone must be able to reach and trust your MakerVault HTTPS endpoint, including any custom port.

For terminal-initiated registration, the paired device requires `inventory:read` and `tags:write` scopes, and the MakerVault user needs permission to view the target record and create/change MakerTags. Other operations require their corresponding scopes.

## Build your own

The current handheld is built around an **ESP32-S3 SuperMini**, **MFRC522/RC522 SPI reader**, **1.44-inch ST7735S 128×128 TFT**, **KY-040 rotary encoder** and **two independent buttons**. The printable enclosure holds the display and controls on the front and the RFID antenna toward the back.

| Start here | Resources |
| --- | --- |
| Firmware source | [Arduino sketch folder](firmware/rfid-terminal/MakerVault_Terminal_Firmware/) |
| Software installation | [Arduino build instructions](firmware/rfid-terminal/BUILD.md) · [Flashing and troubleshooting](firmware/rfid-terminal/FLASHING.md) |
| Electronics | [Rev C.3 wiring / pin map](hardware/rfid-terminal/WIRING.md) · [Bill of materials](hardware/rfid-terminal/BOM.md) |
| Printable hardware | [Enclosure CAD and assembly guide](hardware/rfid-terminal/BUILD_GUIDE.md) · [Hardware files](hardware/rfid-terminal/) |
| Connected setup | [Pairing compatibility](firmware/rfid-terminal/PAIRING_COMPATIBILITY.md) · [MakerVault user guide](https://gavrd7.github.io/MakerVault-docs/) |
| Tests and safeguards | [v0.8 acceptance checklist](docs/terminal-v0.8-browser-acceptance.md) · [NFC writer safety](docs/ntag215-ndef-writer-safety.md) · [NFC physical acceptance](docs/ntag215-esp32s3-acceptance.md) · [TLS testing](firmware/rfid-terminal/TLS_SECURITY_ACCEPTANCE_v0.5.1.7.md) |

**Important:** Use 3.3 V logic and verify wiring for your exact hardware revision. Do not expose the temporary configuration portal to the public internet or disable certificate verification to resolve a connection problem. Physical hardware is maker-built and **not independently safety certified**.

## Roadmap

This is a functional roadmap, not a claim that every future device or feature is already implemented. Completed items reflect the currently tested firmware and application integration.

### MakerVault Terminal

- [x] ESP32-S3 handheld hardware, printable enclosure and assembly references
- [x] Colour TFT interface with rotary encoder and Back / Confirm controls
- [x] Multi-network Wi-Fi, local setup portal and QR-assisted provisioning
- [x] Trusted HTTPS, local CA handling, pairing and background heartbeats
- [x] RFID/NFC UID reading and online MakerTag resolution
- [x] Browser-initiated MakerTag registration and reassignment
- [x] Verified NTAG215 MakerVault URL writing and stable tag identities
- [x] Standalone MakerTag creation, server-side record paging and A–Z jump
- [x] Explicit foreign URL overwrite confirmation and cancellation/recovery protection
- [x] Generic MIFARE Classic UID-only assignment without tag-memory writes
- [ ] **Read-only Creality CFS tag material decoding** and matching decoded data to MakerVault spool inventory
- [ ] Further terminal usability and diagnostics improvements as needed
- [ ] OTA firmware updates (not available in v0.8)

### Other connected hardware

- [ ] **MakerVault Weigh Station:** ESP32-S3, load cell/HX711, round display, RFID reader and removable 3D-printed weighing platform (planned)
- [ ] Share reusable device/pairing/record lookup components between terminal and future hardware where practical

**Deliberately excluded from the Terminal v0.8 scope:** writing Creality CFS material-sector data, changing the printer's material database, or automatic printer sync/push. The planned CFS reader integration is **read-only**.

## Automatic release downloads

Publishing a GitHub Release with a terminal version tag such as `v0.8.0` starts the [release-asset publishing workflow](.github/workflows/publish-terminal-release.yml). The action compiles the tagged sketch using Arduino-ESP32 3.3.12, packages the application binary and Arduino source, and copies the existing STL and STEP ZIP archives **unchanged** from a previous release containing one of each. It verifies the CAD archives and publishes an SHA-256 checksum list.

Expected assets: `MakerVault-Terminal-v0.8.0-Firmware.zip`, `MakerVault-Terminal-v0.8.0-Arduino-Sketch.zip`, `MakerVault-Terminal-v0.8.0-App.bin`, `MakerVault-Terminal-v0.8.0-STL.zip`, `MakerVault-Terminal-v0.8.0-STEP.zip`, `SHA256SUMS.txt` and `CAD-SOURCE.txt`. No manual attachment is normally needed. **Check the publishing action succeeds and the files appear in the release assets after clicking Publish release**; the assets are uploaded by the workflow, not instantly when you click the button. If the previous CAD assets are missing/ambiguous, the action fails instead of inventing replacement files. The workflow does not silently modify old release assets.

The **App.bin** is intended for application-only updates that preserve NVS, when flashed to the verified board-specific application offset; the firmware ZIP contains separate app, bootloader and partition binaries for appropriately configured installations. **Never erase flash/NVS on an already paired device** unless deliberately resetting it. The source ZIP contains the complete sketch folder and links to external Arduino Library Manager dependencies.

## Releases, documentation and licensing

- **[Releases and downloads](https://github.com/gavrd7/MakerVault-Hardware/releases)** — published assets and version-specific notes. The README describes the current source code; a previously published binary may implement fewer features.
- **[MakerVault application](https://github.com/gavrd7/MakerVault)** — the self-hosted backend/frontend, device management and MakerTag records.
- **[MakerVault user guide](https://gavrd7.github.io/MakerVault-docs/)** ([source](https://github.com/gavrd7/MakerVault-docs)) — guided setup, pairing and end-user workflows.
- **[Public release checklist](PUBLIC_RELEASE_CHECKLIST.md)** — distribution/security review notes and historical checks.

MakerVault-authored code is licensed under the **GNU Affero General Public License v3**; see [LICENSE](LICENSE). Bundled third-party components retain their stated licences; see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Do not commit credentials, local CA private keys, personal network details or unredacted provisioning logs. Historical firmware acceptance records remain in their own files rather than being mixed into this project overview.
