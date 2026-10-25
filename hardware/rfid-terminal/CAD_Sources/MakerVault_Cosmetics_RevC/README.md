# MakerVault Handheld — Cosmetics Rev C

This is a targeted change to the **branded bezel** and **encoder knob only**. All enclosure, display opening, and control mounting geometry remains unchanged.

## Flush 3-colour display bezel
- Exterior: 39 × 52 × 1 mm; unchanged from proven bezel.
- Optical opening: 27.8 × 29 mm at the same location as before.
- `MakerVault_flush_wordmark_3color.3mf` contains independent aligned solid regions: black bezel, white Maker, cyan Vault.
- Wordmark has been rotated/transferred to the **larger** solid band (+Y in CAD), *below* the display in the installed orientation, with text upright as installed.
- Flush letters occupy z=0.4–1.0mm; no raised text. Use the THREE constituent meshes, keeping their original shared placement; Creality Print may require manual assignment of the filament slots.
- 3MF is a generic multibody 3MF (not a proprietary Creality slicer project). Check body alignment, palette and slicer preview before printing.

## Encoder knob
- `knurled_D_shaft_knob_12mm_tall.stl`: 12 mm diameter × 12 mm height (previously 9 mm height).
- D-shaft cutout has **the same cross-section** as Rev B, but is deeper: 10.5 mm (previously 7.5 mm).
- The additional socket depth is intended to let the knob sit lower over the exposed shaft, not merely project farther outward.
- Confirm the knob does not rub on the retaining nut or prevent the KY-040 push switch from clicking and returning. Do not force it onto the shaft.

## Printing
Use the established PLA test settings. The bezel has tiny text: inspect at sliced layer level to ensure small strokes are preserved with the selected nozzle and line widths. The knob can be printed separately in a single colour.

## Editable source
`build.py` contains all relevant dimensions and builds STEP and STL parts and a combined 3MF. The original mechanical bezel is unchanged; only text placement changes.
