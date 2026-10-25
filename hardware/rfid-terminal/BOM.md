# Bill of materials — Rev3 handheld

Items and counts below describe the validated Rev C.3 terminal, not the abandoned angled-console prototype. Choose actual board variants compatible with the enclosure geometry and [wiring](WIRING.md). Supplier SKUs and current purchase prices are **not** validated.

| Qty | Item | Specification / notes |
|---:|---|---|
| 1 | ESP32-S3 SuperMini | USB-C; tested board type; GPIOs must match the firmware |
| 1 | MFRC522/RC522 | SPI, 3.3 V, antenna PCB matching the Rev3 cavity |
| 1 | ST7735S TFT | 1.44-inch, 128×128 SPI, matching mounting carrier |
| 1 | KY-040 rotary encoder | With push switch and front locking nut |
| 2 | Tactile buttons | 12×12 mm, normally open; four-pin packages |
| 1 set | 3D-printed Rev3 enclosure | See printed parts below |
| As required | M2 machine screws | Internal display/reader mounting and ESP32 stop bridge; check measured pilot engagement |
| As required | M3 machine screws | Main front/rear enclosure closure; match insert and wall depth |
| As required | M3 heat-set inserts | Prototype used 4.5 mm OD × 5.6 mm long; 4.2 mm bore validated in **PLA only** |
| As required | Stranded hookup wire | Insulated, suitable for 3.3 V logic |
| As required | Heat-shrink / insulation | Cover soldered joints |
| 1 | USB-C **data** cable | For flashing; suitable USB power source |
| Optional | Thin removable adhesive | Hold cosmetic bezel away from active TFT glass |

**Fastener counts and exact screw lengths:** not exhaustively validated for every part/material; inspect the CAD pilot holes and dry-fit the actual hardware rather than treating a suggested nominal size as exact. The **ESP32 USB-stop bridge uses two M2 screws into plastic without inserts**; the enclosure uses **M3 heat-set inserts**.

## Required printable items

| Qty | Path (relative to this folder) |
|---:|---|
| 1 | `STLs/front_shell_fastening_revC.stl` |
| 1 | `STLs/rear_shell_esp32_cradle_revC_m2.stl` |
| 1 | `STLs/usb_stop_bridge_revC_m2.stl` |
| 1 | `STLs/display_carrier_revC.stl` |
| 1 | `3MF/MakerVault_flush_wordmark_3color_revC.3mf` |
| 1 | `STLs/open_switch_carrier_revE.stl` |
| 2 | `STLs/button_cap_revD.stl` |
| 1 | `STLs/encoder_knob_RevD.stl` |

This is **nine** installed printed pieces in total, counting the multi-colour bezel as one assembly. Optional 0.4/0.8 mm switch shims are in `STLs/Optional/`. Single-colour users can substitute the aligned components from `STLs/Bezel_3_Color_Components/`. Editable STEP geometry is under `STEPs/`. Do not substitute superseded geometries stored in `CAD_Sources/`.

## Print guidance

**Actually tested:** fitment of the Rev3 assembly (including revised ESP32 cradle/USB bridge), PLA heat-set insert coupon with 4.2 mm bore and 4.5×5.6 mm M3 insert, PETG in earlier enclosure prototypes. The owner confirms successful printing/assembly of the final fitment. Exact final material, layer height, infill, support orientation, temperatures and slicer settings were **not recorded as one universally verified profile**.

**Suggestions only:** use a dimensionally reliable PLA/PETG/ASA profile appropriate to your printer; print an insert/clearance coupon when changing material; check overhangs at the USB opening and internal carriers; slice the 3MF bezel with its three components aligned. Do not assert a universal infill, support, nozzle or print orientation as proven.

For mounting order, power checks and validation, follow [BUILD_GUIDE.md](BUILD_GUIDE.md).
