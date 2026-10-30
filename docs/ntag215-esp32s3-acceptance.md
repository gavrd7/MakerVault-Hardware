# ESP32-S3 single-scan MakerTags acceptance (experimental)

This procedure applies only to the MakerVault ESP32-S3 handheld terminal.
**This is not a production-approved writer.** Test with disposable NTAG215 tags.

## Prerequisites

- Stage the matching MakerVault PR with database migrations applied.
- Use the experimental firmware sketch from the matching hardware PR and preserve existing pairing settings.
- Require the terminal to advertise `ntag215_ndef_write`, a paired HTTPS endpoint and `tags:write` permission.
- Record the current firmware version and keep the last known working firmware available.
- Use a fresh, disposable NTAG215 with no important contents.

## 1. Existing functionality

Check Wi-Fi, HTTPS trust, pairing, heartbeat, UID lookup, Back/Confirm and regular MakerTags reading. No tag should be altered by lookup.

## 2. One-scan assignment and programming

1. In **Create MakerTag**, select RFID/NFC, an inventory item or spool and **Use MakerVault Terminal**.
2. Verify the proposed MakerVault address, including port; optionally use **Override address**.
3. Start the operation, present the disposable NTAG215 **once**, and keep it against the reader.
4. The terminal checks the UID and existing assignment, then shows **Assign & write?** or the existing-record reassignment review.
5. Press **Confirm once**, then **keep the tag in place** while the server registers/reassigns it and authorises the NFC URL.
6. Wait for the terminal to inspect the physical tag, program if necessary, reselect/read back, and report completion.
7. Remove it only after verified success. Check the NDEF URL independently with a phone and confirm it resolves to the assigned MakerTag.
8. Confirm the MakerTags home list updates without a manual browser refresh.

If the reader loses tag contact, completion must not be reported. Capture the error screen and 115200-baud serial output and stop testing that tag until its state is checked.

## 3. Reassignment

Reassign an existing MakerTag to a different record. The terminal must display the **previous** and **proposed** assignments before the **single physical confirmation**. An unchanged permanent MakerTag URL must not be rewritten. Verify that its URL resolves to the new target and the list changes immediately.

## 4. Negative and safety cases

On disposable test tags, confirm all of the following are blocked or produce clear errors: wrong UID, unsupported tag, protected memory, unrelated existing NDEF contents, cancelled/expired server request, lost HTTPS trust and physical removal before verification.

Do not disable lock/configuration/UID checks or substitute a saved UID for a freshly selected physical tag.

## Acceptance

No ordinary scan writes memory. A combined assignment may commit the database before NFC programming; the UI must explicitly distinguish that partial result from fully verified success. A valid finished state requires the correct UID, exact permanent URL, fresh memory read-back, and acknowledged server completion. Existing public releases remain unchanged until physical testing and review pass.
