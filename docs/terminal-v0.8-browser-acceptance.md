# MakerVault Terminal — standalone MakerTag browser (v0.8 development)

**Status:** draft testing checklist, not a released firmware. Use a dedicated staging MakerVault instance with the backend in [MakerVault PR #161](https://github.com/gavrd7/MakerVault/pull/161). Do not test on valuable/locked CFS tags or a production inventory.

## Preconditions

- ESP32-S3 hardware using the existing Rev C.3 pinout and trusted HTTPS setup.
- Flash the **v0.8.0-dev** binary built from [Hardware PR #10](https://github.com/gavrd7/MakerVault-Hardware/pull/10); do not confuse its binary with stable v0.7.
- Paired, authorised terminal advertises `ntag215_ndef_write` and has `inventory:read`, `tags:read`, `tags:write` scopes. Account has relevant record view, MakerTag add/change permissions.
- A disposable blank NTAG215, a previously assigned NTAG215, and a generic MIFARE Classic/CFS test tag, plus test inventory records in all supported categories.
- The server HTTPS certificate must validate against the saved terminal endpoint (hostname/IP and CA). The NFC URL uses this **same terminal-approved HTTPS origin, including its port**, as there is no browser in a standalone flow.

## Interface acceptance

- [ ] **Home → MakerTags → Add MakerTag** is visible only when paired with adequate scopes.
- [ ] Categories appear alphabetically: Inventory, Location, Printed Part, Printer, Project, Reusable Spool, Spool.
- [ ] Every category lists only records owned by the account and permitted to view.
- [ ] Item names appear alphabetically (case-insensitive), with public MakerVault IDs where available; other records show a short reference instead.
- [ ] Slow rotation moves one record, automatic next/previous page fetch works without noticeable blocking, and no entire catalogue is cached.
- [ ] Hold encoder opens **JUMP TO LETTER**, displaying a *single large changing character* (no alphabet grid or keyboard). Rotate cycles through # and A–Z. Confirm jumps; Back returns to original position.
- [ ] Test empty categories, duplicate names, records with quotes/accented characters, and lists crossing multiple pages (including 100+ rows).
- [ ] Record preview shows the exact chosen item before starting the registration session.
- [ ] Back during page loading and registration-start returns safely, including when HTTPS is slow or offline.

## MakerTag acceptance

- [ ] **Blank NTAG215:** choose record, confirm, present physical tag, confirm assignment once, keep tag in place, verify NFC URL and green completion icon.
- [ ] Scan the new tag with an NFC-enabled phone; it resolves to the correct MakerVault record under normal permissions.
- [ ] **Existing MakerTag, same record:** do not duplicate the MakerTag; matching permanent URL is not rewritten unnecessarily.
- [ ] **Existing MakerTag, different record:** display current and proposed records and require explicit confirmation; preserve `MV-XXXXXXXX` permanent identity while updating assignment.
- [ ] **Existing tag with missing/outdated MakerVault URL:** request explicit overwrite; verify fresh read-back, with accurate partial-failure messaging if it cannot complete.
- [ ] Foreign or another owner's MakerTag, unsupported/locked tags, mismatched UID, cancelled and expired operations fail closed.
- [ ] Generic MIFARE Classic/CFS tags register and resolve via UID **without writing any physical memory**. No printer material/database sync or push occurs.
- [ ] A failed write never displays the success icon; a successful generic UID assignment can complete without an NDEF grant.
- [ ] Denied `inventory:read`/`tags:write` or MakerTag permissions cannot create a session, including direct API attempts.

## Release gates

- [ ] MakerVault backend CI, including terminal browse and device-initiated session tests.
- [ ] MakerVault-Hardware ESP32-S3 build and portable logic tests.
- [ ] Staging and physical test evidence recorded without leaking device keys or credentials.
- [ ] Review large-list latency, connectivity recovery, cancellation, record naming, and access isolation.
- [ ] Update published user guide, firmware/source release notes, and version before merging. **Do not ship this draft artifact as stable v0.7.**
