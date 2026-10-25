# Public release checklist

Status: **PRE-PUBLICATION — keep repository private until reviewed**.

- [x] Firmware v0.5.1.13 physically tested for pairing, trusted HTTPS, UID scan and recovery
- [x] Tested source and bundled QRCode MIT sources committed on `main`
- [x] AGPLv3 licence matches MakerVault's actual licence
- [x] QRCode source and its MIT copyright notice preserved
- [x] Wiring table and Arduino build instructions documented
- [x] Initial review of current committed source and `main` commit-tree filenames: no obvious secret/key files found
- [x] Maintainer confirmed logo, UI graphics and enclosure designs were created for this project, including AI assistance; third-party asset attribution still applies if discovered
- [ ] Review actual historical file content and any PR branches for tokens, URLs with embedded secrets, private CA keys and personal data (filename scans alone are insufficient)
- [ ] Correct stale System info/heartbeat firmware labels and physically test updated candidate before tagging
- [ ] Confirm reproducible Arduino compile with documented exact dependency versions, or state explicitly that build is manually verified only
- [ ] Optional for first publication: release enclosure models, assembly instructions and verified printable files
- [ ] Review repository visibility, forks and licence notices before making public

Do not claim safe public exposure solely on the basis of the current source keyword scan. If secrets ever appeared in commits, revoke them; deleting from the tip does not erase historical exposure.

**No automated flash encryption or secure boot verification** was performed. Stored ESP32 NVS credentials are persistent but must not be described as encrypted at rest without specific configuration/validation.
