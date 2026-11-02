# Experimental third-party filament RFID

MakerVault Terminal v1.0 reads supported tags without writing or changing their contents. Decoding capability depends on tag technology, documentation, and accessible data; it is **not** a claim of universal brand compatibility.

| Format | Hardware / access | Status | Offline information |
| --- | --- | --- | --- |
| Creality CFS | Compatible MIFARE Classic tags; read-only sector decoder | Hardware-tested with known CFS tags | UID, material code, HEX colour and swatch, and any other validated decoded fields |
| Bambu Lab | MIFARE Classic 1K; protected sector authentication and community-described keys | Experimental research; physical decode not yet validated | UID/basic tag classification unless authenticated data is actually read |
| Elegoo | Published NTAG213-based formats; variants differ | Experimental research; physical decode not yet validated | UID/basic tag classification unless a supported payload is actually decoded |

Bambu Lab community format research: https://github.com/NickWaterton/Bambu-Lab-RFID-Tag-Guide and https://github.com/cwiegert/rfid/blob/main/docs/BambuLabRfid.md.

Elegoo manufacturer documentation: https://github.com/elegooofficial/ELEGOO-RFID-Tag-Guide. Real-world examples differ in their layout; verify the actual tag before decoding colour, material or weight.

**Online enhancement:** A paired terminal can request Creality material names from MakerVault's stored, read-only printer material database using its existing HTTPS connection. Physical spool records are matched by the tag's **RFID UID**, never merely by material or colour. A scan can fill a spool's Physical Identity field in the web UI, but only explicit Save associates the spool.

**Offline behaviour:** All information decoded directly from a supported tag remains available; server-side names and spool identity are unavailable and must not be guessed.

**Safety:** No third-party filament tag-writing support, printer database push, automated inventory edits, or automatic spool association. Unknown or locked formats fall back to generic tag UID display. Compatible physical tag samples and repeatable hardware tests are required before claiming additional decoding support.

## v1.0 acceptance gates

- [x] Creality tag material and colour display verified with eSUN and SUNLU examples
- [x] Online K2 material database lookup, real spool UID matching and browser-assisted scan tested
- [x] Offline CFS tag decoding and generic MakerTag/RFID regression tested
- [ ] Document and physically validate Bambu Lab decoding, if readable on available hardware
- [ ] Document and physically validate Elegoo decoding, if readable on available hardware
- [ ] Final cross-repository CI, security, release asset and hardware regression review
