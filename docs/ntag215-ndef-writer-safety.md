# NTAG215 NDEF writer — safety and acceptance gates

Status: **not hardware-ready**. The current codec and memory inspector are pure C++ helpers and must not be treated as permission to write an NFC tag. Keep the existing terminal scanner and Creality CFS code unchanged.

## Software prerequisites
1. The browser creates an expiring, owner-bound operation through MakerVault's `/api/device/ndef-write/` endpoint only after previewing the exact URL.
2. The connected terminal must have an active pairing, explicit `tags:write` scope, and a firmware capability named `ntag215_ndef_write`. **Do not advertise this capability before physical writes and verification are implemented and tested.**
3. Device must reject missing, expired, cancelled, mismatched-UID or unexpected requests and must not automatically write when a tag is scanned.
4. URL, physical UID, request ID and confirmation should be associated with a single transaction, with confirmation on the physical terminal **after** checking the exact UID and target URL.
5. Existing CFS-tag decoding remains read-only. Never attempt NDEF insertion or formatting on a tag that fails strict NTAG215 checks.

## Physical-reader implementation gates
- Read UID and verify seven-byte format. A valid seven-byte UID alone **does not prove** a tag is NTAG215.
- Read and validate capability container, static lock bytes, dynamic lock bytes, and any relevant password/protection configuration; reject unknown or partially inspected memory states.
- Read the existing NDEF area and preserve unrelated data unless an explicitly reviewed replacement procedure is implemented. Do not silently format a non-empty tag.
- Restrict writes to user-memory pages; never write UID/manufacturer pages, capability container, static/dynamic lock pages, configuration, OTP, or password/key material.
- Check exact encoded payload length and page alignment against writable user-memory capacity. Require an explicit, physical confirmation after the intended URL and target UID are shown.
- After each write, read back the physical tag, re-check UID, decode the complete NDEF TLV and compare the exact URL. Report completion to the server **only after successful physical read-back**.
- Power interruption, lost RFID field, cancellation, timeout, failed read-back or UID change must fail closed, with clear UI error and no success report.

## Hardware acceptance checklist
- [ ] Fresh Arduino IDE Verify/Compile of the complete terminal sketch.
- [ ] Existing registration, lookup, reassignment, cancellation, pairing, heartbeat and normal scanning regressions.
- [ ] Blank writable genuine NTAG215: explicit write, read-back and phone tap.
- [ ] Already-written NTAG215: inspect before any replacement; confirm explicit replacement policy.
- [ ] Read-only/locked/protected tag: no write attempted.
- [ ] Wrong UID, different tag substituted during confirmation, malformed or non-NTAG CC: no write.
- [ ] CFS/MIFARE/NTAG other types: normal read behavior preserved, never written.
- [ ] Cancel, timeout, network loss, tag removal and power loss: no false successful completion.
- [ ] Confirm a locally generated URL with IP + port and an explicit override.
- [ ] Repeat on a phone over the intended LAN/VPN; verify the current MakerVault record opens after authentication.

## Automated tests
`c++ -std=c++11 -Wall -Wextra -Werror tests/ntag_ndef_uri_test.cpp -o /tmp/makervault-ndef-test && /tmp/makervault-ndef-test`

Passing native tests cover the standalone codec and conservative CC/lock inspection **only**, not physical RFID operations.
