# MakerVault handheld front controls — Rev D

Rev D corrects the Rev C button geometry and switch orientation. This is a **functional test coupon**, not a final, validated enclosure assembly.

## Included

- `control_panel.stl` — same nominal 70 × 43 × 2 mm panel, encoder opening Ø7.4 mm and button openings Ø8.6 mm.
- `button_cap.stl` — print **two** identical caps. Ø7.4 mm moving guide, Ø10.2 mm internal captive flange, Ø4.5 mm switch-contact plunger. **The wider flange does NOT pass through the hole**; insert from the back of the panel.
- `open_switch_carrier.stl` — print **one**. Two front-facing switch positions with open sides to allow terminal access. Switches are individual 12 × 12 mm tactile switches, with actuators facing **towards the front**.
- `shim_0p4.stl` and `shim_0p8.stl` — optional 0.4 and 0.8 mm washer shims, print two of a chosen thickness when testing alternate switch mounting depth.
- STEP models, `reference_assembly.step`, and editable CadQuery `front_controls_revD.py`.

## Printing

PETG fit test, ~0.2 mm layers, three walls. Lay the panel on the flat face; place small parts on stable flat sides in the slicer. The cap is authored in assembled Z coordinates and its bottom extends below Z=0, so use slicer auto-drop to plate or reorient as required. Print without supports where feasible. Inspect thin tabs/rails in slicer preview.

## Intended assembly sequence

1. Insert each cap **from the rear** so its narrow guide passes forward through the front-panel aperture and the larger flange stays inside the panel. Confirm smooth free movement. The cap cannot be inserted head-first from the outside.
2. Orient each tactile switch with its circular actuator toward the front and its four terminals accessible at the left and right sides. Prewire switches before mounting if possible; do not bend pins against printed walls.
3. Fit the switches to the carrier. The switch body should rest on its outer Y-edge rails; terminal sides remain open. Confirm no print touches the actuator or electrical pins. **Do not permanently fasten or glue.**
4. Present the loaded carrier to the panel from behind, matching two carrier clearance holes with the panel bosses. The indicated geometry nominally assumes the back face of the switch body is at Z~8.5 mm, but actual switch travel and moulding tolerances have NOT been verified.
5. Before fully securing anything, press the two caps gently. If the switches are preloaded, don't tighten. Try two matching 0.4 or 0.8 mm shim washers between panel bosses and carrier tabs for increased separation; if the cap does not reach the switch, return to the unshimmed arrangement. These shims are experimental travel adjustments, not finalized over-travel stops.
6. Test solder access and ability to remove the switch/carrier and caps without damage.

## Fastener caution

Carrier fixing holes are Ø2.3 mm nominal M2 clearance; captive nut pockets in panel bosses are only provisional. The present pockets are not a proven screw-and-nut installation method. Verify real M2 nut dimensions, entry/access, and screw reach *before* attempting to tighten. Do not force machine screws into plastic or drive screws through the panel face. You can hold the carrier against the bosses for an initial fit inspection.

## Pass/fail criteria

- Both Ø7.4 mm caps freely enter Ø8.6 mm openings from behind, retained by flange.
- Individual switches orient unambiguously with actuators forward and terminals free.
- Carrier seats without switch deformation; both buttons return after a gentle press and click normally.
- No significant switch preload and no overtravel pressing into the switch body.
- Fixing hardware can be installed/removed without damage (not yet proven).

**Warning:** Moulded switch operating travel has not been measured. This coupon is for mechanism fit and click-height evaluation. Final positive travel stop, enclosure integration, and button retention cannot be signed off until physically tested.
