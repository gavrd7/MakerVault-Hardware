# MakerVault handheld — printable enclosure & assembly guide (Rev3)

## Compatibility and validation

This pack is from **Master 3D Files Rev3 (23 October 2026)**. Approximate outer size from the master guide: **70 × 103 × 34 mm**. The user subsequently printed and confirmed fitment of the revised ESP32 rear cradle and USB-stop bridge. The bridge is held using **M2 screws threaded directly into printed plastic**, without heat-set inserts. Do not confuse these small internal fasteners with the enclosure's M3 heat-set inserts.

## Printable parts

| Source file | Qty | Description |
|---|---:|---|
| `STLs/front_shell_fastening_revC.stl` | 1 | Front shell; main shell insert locations |
| `STLs/rear_shell_esp32_cradle_revC_m2.stl` | 1 | Current rear shell, antenna cavity and ESP32 cradle |
| `STLs/usb_stop_bridge_revC_m2.stl` | 1 | ESP32 USB retention bridge; screws directly into plastic |
| `STLs/display_carrier_revC.stl` | 1 | ST7735S screen holder |
| `3MF/MakerVault_flush_wordmark_3color_revC.3mf` | 1 | Flush three-colour front bezel; contains its component objects |
| `STLs/open_switch_carrier_revE.stl` | 1 | Two-switch support with wire relief |
| `STLs/button_cap_revD.stl` | 2 | Back and Confirm caps |
| `STLs/encoder_knob_RevD.stl` | 1 | Rotary encoder knob |

**Total:** nine physical printed pieces when the multicolour bezel is counted as one assembly. Optional switch-height shims of 0.4 mm and 0.8 mm are in `STLs/Optional/`; print only if you need to adjust switch feel.

If your printer cannot handle the 3MF multicolour workflow, `STLs/Bezel_3_Color_Components/` holds the three aligned bezel solids. Import them into the same build plate without individually centring or rescaling, assign black, white and blue to the correct objects, and verify positioning in the slicer.

## Electronics

- ESP32-S3 SuperMini, USB-C accessible at the bottom of the enclosure
- MFRC522 RC522 SPI reader with antenna against the upper rear shell
- ST7735S 1.44-inch 128×128 TFT
- KY-040 encoder (push-to-select), separate Back and Confirm tactile switches
- Wiring, solder and heat-shrink as needed

### Firmware GPIO map (validated Rev C.3)

| Function | ESP32-S3 GPIO |
|---|---|
| Shared SPI SCK / MOSI / MISO | 12 / 11 / 13 |
| TFT CS / DC / RESET | 1 / 2 / 43 |
| RFID reader CS / RST | 10 / 9 |
| Encoder CLK / DT / SW | 4 / 5 / 6 |
| Back / Confirm | 7 / 8 |
| TFT BLK / power | 3V3 (as wired in the user's prototype) |
| All modules | Common ground |

**Use 3.3 V logic.** The TFT backlight is wired directly to 3V3 on the validated prototype, but current limiting depends on the particular module; check yours before duplicating that connection.

## Fasteners

- Main enclosure: M3 heat-set inserts, nominal Ø4.5 mm OD × 5.6 mm long; **4.2 mm printed bore was validated only in PLA**, not PETG/ASA. Select M3 screws to match the assembled housing and avoid bottoming out.
- ESP32 bridge: **two M2 screws directly into plastic pilot holes**, not M2 inserts. CAD nominal bridge clearance hole Ø2.3 mm; cradle pilot Ø1.7 mm. The original designer suggested nominal 6–8 mm screws, but choose length based on real engagement, blind-hole depth and your already-tested assembly. Do not overtighten or strip the plastic.
- TFT / RC522 carriers: internal M2 screws were used successfully in prototype fit tests; use lengths that do not penetrate external walls.
- Encoder: its bushing and nut retain the encoder through the front fascia; knob fits the encoder shaft.
- Front bezel: thin removable adhesive *after* wiring verification, attached to surrounding plastics rather than TFT glass.

## Suggested print preparation

1. Load the current STLs and the 3MF bezel in a slicer. Verify they are in **millimetres** and have not been auto-scaled.
2. Choose your tested material/profile; earlier printed calibration pieces used PLA, while some enclosure prototypes used PETG. No universal production temperature/infill/orientation is validated for all parts.
3. Inspect support requirements at USB-C opening, shell features and carrier overhangs; orient faces for finish and dimensional accuracy. Do a small tolerance test when switching materials.
4. Slice the three-colour bezel with aligned colour objects; check that the wordmark sits on the bottom, larger vertical bezel face with correct orientation.

## Assembly order

1. **Prepare fasteners.** Install M3 heat-set inserts in their main shell locations if using the validated PLA bore/insert combination; test fit carefully. Prepare the small M2 pilot holes for plastic screw engagement in the ESP32 cradle.
2. **Fit display.** Mount ST7735S in the Rev C display carrier, ensuring the visible TFT area is flush and non-viewable borders are covered by the bezel; do not apply pressure to glass.
3. **Fit controls.** Mount the KY-040 on the left; arrange Back and Confirm on the right with an obvious horizontal gap. Install the Rev E switch carrier, button caps and encoder knob. Check that all inputs move freely and wiring clears the carrier.
4. **Install reader.** Mount the RC522 in the upper rear area with antenna facing outward through the back plastic. Keep wiring and metal hardware away from the antenna area where practical.
5. **Fit ESP32.** Locate ESP32-S3 vertically in the Rev C M2 rear cradle with its USB-C connector aligned to the enclosed bottom opening. Fit the `usb_stop_bridge_revC_m2` with **two M2 screws directly into plastic**. Check insertion/removal loads do not push the controller back inside or contact GPIO solder joints.
6. **Wire and electrically test** the common SPI bus, TFT, RFID and buttons before closing the shells. Avoid shorts, pinching or pressure on LCD/antenna.
7. **Close housing.** Join enclosure halves with appropriately sized M3 machine screws engaging the inserts; do not overtighten.
8. **Fit bezel.** Attach the flush three-colour bezel with removable adhesive only after successful testing.

## Functional acceptance

- USB cable inserts without moving the ESP32; ESP32 is retained by the M2 bridge.
- TFT fits flush; bezel does not cover active pixels.
- Encoder rotates and selects independently; Back and Confirm operate without accidental presses.
- RC522 reads known MIFARE Classic and NTAG215 tag UIDs.
- Wi-Fi, pairing and HTTPS trust operate with supported terminal firmware.
- Rear shell sits flush and all fasteners secure without cracks or stripped pilot holes.

## Source revisions and known scope

Use STEP files in `STEPs/` as the authoritative exported editable geometry for the *current* required parts. `CAD_Sources/` contains originating scripts/notes and may include earlier/alternate parts such as `rear_fastening_revC` and old knobs; those **are not replacements for the Rev3 printable list**. The 3MF bezel is the recommended multicolour print setup. This release pack covers mechanical files and assembly, not firmware or the MakerVault web app.
