# MakerVault TFT Carrier — Rev C

**Replacement carrier only.** Reuse the printed Rev A front coupon and flush bezel. No other parts need printing.

## What changed from Rev B

- Original 36 × 49 mm body, four mounting-ear locations (X ±23.8 mm, Y ±15 mm), through-holes (Ø2.3 mm), and existing PCB corner pads are **unchanged**.
- Added eight short **edge-guiding features** inside the existing frame (four side guides and four end guides). Their inner faces create a nominal **30.0 × 43.5 mm PCB locating envelope** for a 29.5 × 43.0 mm PCB: 0.25 mm clearance each side.
- The guide additions rise 1.35 mm above the Rev B ring. The original supporting surfaces and their Z heights are unchanged, but the extra material above the frame **still requires a careful physical interference check**.
- End guides are located towards the corners, leaving the middle of the top wiring edge open.

This modification attempts to reduce the approximately 2 mm of side-to-side and end-to-end motion reported in Rev B. Do **not** shrink the full carrier by 2 mm: that would break compatibility with the successful coupon.

## Print

Print `display_carrier_revC.stl` flat in PETG, 0.2 mm layers, 3 walls. STEP and editable CadQuery source (`design.py`) are included. **Only this carrier needs printing.**

## Dry-fit checks

1. Disconnect power. Reuse the Rev A coupon and bezel and position the display with wires exiting at the established top edge.
2. Try the Rev C carrier without screws. The PCB must enter the locating guides **without force**, with the guides touching only the bare **PCB edges**, not glass, ribbon, solder joints or surface components.
3. Check lateral movement in both X and Y. Slight clearance is expected; the board should no longer rattle visibly.
4. Check that the seating depth and optical alignment remain correct; guides must not compress the screen.
5. Confirm access for the eight wires at the top edge and that the carrier can be removed freely.
6. Mounting-ear fastener engagement, clamp load and final component retention remain **unvalidated**. Do not tighten screws against the TFT during this test.

If any edge guide touches the display module, mark which guide and share a photo. We can locally relieve it instead of changing the correct external fastening geometry.

**Note:** This is a PETG dimension coupon. Final matte PLA / ABS / ASA shrinkage and tolerances should be rechecked on the chosen filament.
