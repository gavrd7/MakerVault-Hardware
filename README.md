# MakerVault Hardware

**Build, flash and use connected hardware for [MakerVault](https://github.com/gavrd7/MakerVault).** This repository contains open-source firmware, 3D-printable enclosures, hardware assembly instructions, wiring diagrams and practical testing documentation.

The first supported project is the **MakerVault Terminal**: a handheld RFID/NFC reader and MakerTag writer that connects securely to your self-hosted MakerVault installation.

**Compatible release sources:** MakerVault **v1.5.1** + MakerVault Terminal **v1.0.1**. Confirm published release assets in [Releases](https://github.com/gavrd7/MakerVault-Hardware/releases).

## MakerVault Terminal

A compact, 3D-printable handheld with a 128×128 colour display, rotary encoder, Back and Confirm buttons, and an RFID/NFC reader housed behind the enclosure. It can look up physical tags, browse MakerVault records, and register or reassign MakerTags without needing to start the workflow on a computer.

### Current capabilities

| Area | What it does |
| --- | --- |
| **Wi-Fi and setup** | Connects to 2.4 GHz Wi-Fi, remembers multiple networks, and offers a temporary access point / QR-assisted phone setup |
| **Secure connection** | Certificate-verified HTTPS using the native ESP-IDF HTTP client, persistent keep-alive sessions, scoped credentials, custom/public CA trust, manual **Reconnect server**, connection diagnostics and controlled recovery |
| **Pairing and management** | Pairs through MakerVault's eight-digit Connected Devices wizard; uses scoped, revocable credentials and reports device heartbeat |
| **Read and look up tags** | Scans compatible RFID/NFC UIDs and resolves registered MakerTags through MakerVault |
| **Create MakerTags on the terminal** | Browse **Inventory, Location, Printed Part, Printer, Project, Reusable Spool and Spool** directly on the handheld |
| **Browse large inventories** | Uses alphabetically sorted, paged server results, record IDs and a hold-to-open **A–Z letter jump**; no on-screen keyboard or full inventory download |
| **Assign and reassign** | Assigns a new tag, recognises an existing MakerTag, and preserves its stable `MV-XXXXXXXX` identity on reassignment |
| **NTAG215 NFC writing** | Writes and verifies the permanent MakerVault HTTPS record URL on supported writable NTAG215 tags |
| **Clean / Remove MakerTag — #159 feature branch** | **Clean** clears and verifies physical NTAG215 NFC data while retaining the MakerVault record; **Remove** deletes the server registration without altering NFC memory; **Remove + clean** wipes first, then removes the registration. Requires the matching MakerVault v1.5.1 app; included in the matching v1.0.1 release-candidate source; availability in published assets depends on the final tagged release |
| **Protected URL replacement** | Shows an additional **Back / Confirm** prompt before replacing a recognised unrelated HTTP(S) NFC URL; Back leaves it unchanged |
| **Safety and recovery** | Enforces physical UID matching, tag compatibility/lock checks, server authorisation and expiry; supports cancellation and retry after incomplete writes |
| **Other RFID tags** | Supports UID-only lookup/assignment for compatible MIFARE Classic 1K tags without modifying their sectors |

The browser-led **Use MakerVault Terminal** workflow is also supported. Terminal-first registration and browser-led registration are complementary options.

### Filament RFID reading

The terminal reads compatible **Creality CFS** material tags, shows the material ID and the tag's actual HEX colour/swatch, and uses a paired MakerVault server to resolve the manufacturer's name and material product from a synchronised read-only printer material database. A physical spool is identified **only by its separately linked RFID UID**; matching a product name or colour never silently assigns a spool. From the MakerVault spool's **Physical Identity** window, choose **Scan using terminal** to capture its UID, then explicitly save the link.

**Offline:** the tag's UID, decoded material ID and colour remain available without Wi-Fi; complete product names and linked spool details require MakerVault.

**Other manufacturers:** Bambu Lab and Elegoo have experimental read-only reader paths, based on documented MIFARE Classic and NTAG data. Both are software-validated only; neither has been tested with a genuine manufacturer tag. Protected, unknown or unsupported tags fall back to generic UID reading. No third-party filament tag rewriting is supported. See [third-party RFID limitations](docs/terminal-third-party-rfid.md).

### Clean or remove a MakerTag (draft #159)

The matching **Terminal v1.0.1 and MakerVault v1.5.1 releases** provide two distinct options to **Home → MakerTags**:

1. **Clean MakerTag:** present a **registered writable NTAG215**, review the linked record, confirm, and present the *same* tag for erasure and verification. The physical user data is cleared; the MakerVault MakerTag registration, UID association and short URL **remain** so the tag can be programmed again later. An unregistered/blank tag is rejected as **Tag not registered**.
2. **Remove MakerTag:** present a registered MakerTag, then answer **Clean NFC data too? Yes / No / Back**. **No** removes the server registration only, leaving any old NFC URL physically present (but no longer resolvable). **Yes** requires the original NTAG215 to be physically wiped and independently verified before the registration is removed. **Back** cancels.

The **web interface** offers **Clean NFC data** with paired-terminal selection and **Remove MakerTag** with an optional physical clean. An incomplete or failed wipe must not delete the MakerTag record, and a reported partial result must not be mistaken for full completion.

Physical cleaning is supported only for writable, unprotected **NTAG215 MakerTags**. UID/manufacturer, capability/lock, password/configuration and reserved pages remain unchanged. Generic RFID and Creality CFS/Bambu/Elegoo manufacturer tags are **never wiped**.

**Connection troubleshooting (v1.0.1 release candidate):** Use **Connections → MakerVault → Reconnect server** to refresh the Wi-Fi/TCP transport and reverify the saved HTTPS pairing without forgetting the server, CA or credentials. The terminal now shows **Joining saved Wi-Fi**, **Verifying HTTPS**, then **Connected** or an actionable **Failed** state; **OK** retries a failed attempt. Wi-Fi may briefly appear offline **during the deliberate transport reset**. **Test HTTPS** is diagnostic only and cannot reset the transport. Connection status shows free internal heap and largest available block to help investigate allocation failures. Idle verification no longer repeatedly reconfigures SNTP and is less frequent to reduce allocation churn. The previously reproducible ~96-byte-per-request idle-polling memory leak is **not reproduced in the reported two-hour-plus ESP-IDF endurance run**; request buffers now unwind before FreeRTOS worker deletion. Wi-Fi, certificate and MakerTag workflows have also been reported passing the release test matrix. Continue monitoring on released hardware; this does not guarantee every environment is leak-free. The pending-work checks for browser registration, NFC writing, RFID lookup and Clean/Remove now share **one authenticated five-second poll** rather than four HTTPS polls; heartbeat and live safety checks remain separate. This requires the matching MakerVault app feature branch with the new `/api/device/v1/pending-work/` endpoint.

The full step-by-step instructions are being updated in the [MakerVault user guide](https://gavrd7.github.io/MakerVault-docs/using/maker-tags/). **Release boundary:** these features are present in the matching Terminal v1.0.1 firmware and MakerVault v1.5.1 app. Use the compatible published release assets; older app versions may not offer the consolidated pending-work and Clean/Remove endpoints.

### A typical on-device workflow

1. Pair the terminal to MakerVault over **trusted HTTPS** and grant the required device scopes and account permissions.
2. Open **Home → MakerTags → Add MakerTag**.
3. Choose a record category, browse the list or hold the encoder for the large **A–Z jump**, and review the selected record.
4. Present the NFC/RFID tag and approve the assignment as prompted.
5. For a supported writable NTAG215, approve replacement separately if it already holds an unrelated URL, then keep the same tag near the reader until **MakerTag ready** appears.
6. Scan the completed NTAG215 with a phone to open its stable MakerVault URL. The phone must be able to reach and trust your MakerVault HTTPS endpoint, including any custom port.

For terminal-initiated registration, the paired device requires `inventory:read` and `tags:write` scopes, and the MakerVault user needs permission to view the target record and create/change MakerTags. Other operations require their corresponding scopes.

## Release validation: v1.0.1

The maintainer reports that the complete connectivity, security, MakerTag, RFID, usability, build and release acceptance matrix passed on physical hardware. The native ESP-IDF persistent HTTPS transport and corrected C++/FreeRTOS worker cleanup were additionally observed over a two-hour-plus idle soak: stable idle heap, one reused secure session, successful 5-second pending-work polling and no memory-recovery reboot. This is an observed test outcome, **not a guarantee of leak-free operation in every deployment**. See [release notes](docs/TERMINAL_v1.0.1_RELEASE_NOTES.md), [flashing and recovery](firmware/rfid-terminal/FLASHING.md) and [NFC safety requirements](docs/ntag215-ndef-writer-safety.md).

## Build your own

The current handheld is built around an **ESP32-S3 SuperMini**, **MFRC522/RC522 SPI reader**, **1.44-inch ST7735S 128×128 TFT**, **KY-040 rotary encoder** and **two independent buttons**. The printable enclosure holds the display and controls on the front and the RFID antenna toward the back.

| Start here | Resources |
| --- | --- |
| Firmware source | [Arduino sketch folder](firmware/rfid-terminal/MakerVault_Terminal_Firmware/) |
| Software installation | [Arduino build instructions](firmware/rfid-terminal/BUILD.md) · [Flashing and troubleshooting](firmware/rfid-terminal/FLASHING.md) |
| Electronics | [Rev C.3 wiring / pin map](hardware/rfid-terminal/WIRING.md) · [Bill of materials](hardware/rfid-terminal/BOM.md) |
| Printable hardware | [Enclosure CAD and assembly guide](hardware/rfid-terminal/BUILD_GUIDE.md) · [Hardware files](hardware/rfid-terminal/) |
| Connected setup | [Pairing compatibility](firmware/rfid-terminal/PAIRING_COMPATIBILITY.md) · [MakerVault user guide](https://gavrd7.github.io/MakerVault-docs/) |
| Tests and safeguards | [terminal acceptance checklist](docs/terminal-v0.8-browser-acceptance.md) · [NFC writer safety](docs/ntag215-ndef-writer-safety.md) · [NFC physical acceptance](docs/ntag215-esp32s3-acceptance.md) · [TLS testing](firmware/rfid-terminal/TLS_SECURITY_ACCEPTANCE_v0.5.1.7.md) |

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
- [x] **Read-only Creality CFS tag material decoding** with online material-name and physical spool UID lookup
- [x] Experimental Bambu Lab and Elegoo read-only decoding paths with synthetic software fixtures (not hardware-validated)
- [ ] Further terminal usability and diagnostics improvements as needed
- [ ] OTA firmware updates (not available in v0.8)

### Other connected hardware

- [ ] **MakerVault Weigh Station:** ESP32-S3, load cell/HX711, round display, RFID reader and removable 3D-printed weighing platform (planned)
- [ ] Share reusable device/pairing/record lookup components between terminal and future hardware where practical

**Deliberately excluded from the Terminal v1.0 scope:** writing Creality CFS material-sector data, changing the printer's material database, or automatic printer sync/push. Filament RFID scanning is **read-only**.

## Automatic release downloads

Publishing a GitHub Release with a terminal version tag such as `v1.0.1` starts the [release-asset publishing workflow](.github/workflows/publish-terminal-release.yml). The action compiles the tagged sketch using Arduino-ESP32 3.3.12, packages the application binary and Arduino source, and copies the existing STL and STEP ZIP archives **unchanged** from a previous release containing one of each. It verifies the CAD archives and publishes an SHA-256 checksum list.

Expected assets: `MakerVault-Terminal-v1.0.1-Firmware.zip`, `MakerVault-Terminal-v1.0.1-Arduino-Sketch.zip`, `MakerVault-Terminal-v1.0.1-App.bin`, `MakerVault-Terminal-v1.0.1-STL.zip`, `MakerVault-Terminal-v1.0.1-STEP.zip`, `SHA256SUMS.txt` and `CAD-SOURCE.txt`. No manual attachment is normally needed. **Check the publishing action succeeds and the files appear in the release assets after clicking Publish release**; the assets are uploaded by the workflow, not instantly when you click the button. If the previous CAD assets are missing/ambiguous, the action fails instead of inventing replacement files. The workflow does not silently modify old release assets.

The **App.bin** is intended for application-only updates that preserve NVS, when flashed to the verified board-specific application offset; the firmware ZIP contains separate app, bootloader and partition binaries for appropriately configured installations. **Never erase flash/NVS on an already paired device** unless deliberately resetting it. The source ZIP contains the complete sketch folder and links to external Arduino Library Manager dependencies.

## Releases, documentation and licensing

- **[Releases and downloads](https://github.com/gavrd7/MakerVault-Hardware/releases)** — published assets and version-specific notes. The README describes the current source code; a previously published binary may implement fewer features.
- **[MakerVault application](https://github.com/gavrd7/MakerVault)** — the self-hosted backend/frontend, device management and MakerTag records.
- **[MakerVault user guide](https://gavrd7.github.io/MakerVault-docs/)** ([source](https://github.com/gavrd7/MakerVault-docs)) — guided setup, pairing and end-user workflows.
- **[Public release checklist](PUBLIC_RELEASE_CHECKLIST.md)** — distribution/security review notes and historical checks.

MakerVault-authored code is licensed under the **GNU Affero General Public License v3**; see [LICENSE](LICENSE). Bundled third-party components retain their stated licences; see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Do not commit credentials, local CA private keys, personal network details or unredacted provisioning logs. Historical firmware acceptance records remain in their own files rather than being mixed into this project overview.
