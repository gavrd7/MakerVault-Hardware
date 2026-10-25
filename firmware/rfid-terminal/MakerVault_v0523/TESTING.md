# v0.5.1.13 physical acceptance

1. Flash over USB with the existing partition scheme, without erasing NVS.
2. Confirm previous pairing, Wi-Fi, CA and RFID scan remain functional.
3. For negative test, use an unpaired device or explicitly remove local pairing.
   Set the existing MakerVault private-CA URL with an empty CA field.
   Choose Pair Device. Expect a bounded failure screen (certificate rejected
   when native TLS verifies that diagnosis), not an indefinite "Requesting code"
   display; Back must work. Do not try to approve a code that was never issued.
4. Restore the correct Local CA through phone setup. Pair successfully and
   check heartbeat + reboot persistence.
5. Validate URL/CA credential retention and blocked reuse at another origin.
6. Ensure no 'MVSHOT' screenshot command or giant framebuffer remains.
7. Check the HTTPS browser wizard separately if it displays 'HTTPS required';
   that error refers to browser/backend request security, not ESP32 CA trust.
