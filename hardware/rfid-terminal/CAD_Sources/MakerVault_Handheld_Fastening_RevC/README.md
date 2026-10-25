# MakerVault Handheld — Fastening Rev C (PLA-validated M3 insert pocket)

## Purpose
Matching front and rear shells for a **mechanical fastening fit test**. This revision modifies the Rev B heat-set insert receiving pockets only; no adjustment has been made to the tested screen, controls, RC522, ESP32 layout, USB access, or mating lip geometry.

## Prints
- `front_fastening_revC.stl` (one)
- `rear_fastening_revC.stl` (one)

Use 0.2 mm layers and at least 3 perimeters. The printed insert-coupon fit was obtained in **PLA**; the 4.2 mm bore has not yet been calibrated in ASA or PETG. If printing this assembly in PETG, treat the insert fit as unverified and first print a new insert calibration coupon in that material. For heat-setting, follow the insert manufacturer's temperature guidance; test temperature and insertion pressure away from electronics.

## Parameters
- External closed envelope: 70 × 103 × 34 mm
- Front and rear nominal shell sections: 12 and 22 mm (features such as bosses extend beyond split plane)
- Front receiver bosses: 9.6 mm diameter
- Blind insert holes: **4.2 mm nominal diameter**, **6.1 mm depth**, opening on the inner boss face (Z=18)
- Insert hardware: M3, measured outer diameter 4.5 mm, length 5.6 mm
- Rear shank passage: 3.35 mm diameter
- Rear head recess: 6.5 mm diameter
- Initial candidate: M3 × 6 mm flat-ended machine screws, **only after verifying actual under-head length and screw-head diameter**
- Four positions: (7,9), (63,9), (9,95), (63,94), measured from front left/top
- Peripheral locating joint retained from Rev B

## Assembly check
1. Dry-fit both *empty* shells before installing inserts. Confirm the alignment lip engages without force and four screw axes line up.
2. Insert M3 heat-set inserts into the four front receivers, aligned with their axes and seated flush. Stop if the bosses soften excessively, split, or deform.
3. Let the plastic cool. Check M3 threads with a screw before joining the halves.
4. Join the empty shells and start all screws by hand. Do not power-drive or overtighten. Check for head clearance, bottoming out, wall distortion, and even closure.
5. Reopen and check that inserts do not spin, pull out, or disturb the front wall. Repeat gentle disassembly/reassembly a few times before declaring the joint serviceable.
6. Later test with electronics; do not assume electronic clearance from an empty-shell fit alone.

## Not finished in Rev C
- ESP32 load-bearing bracket/retainer
- Final RC522 / TFT machine-screw retention
- Final USB connector / plug strain-relief assessment
- Final material-specific tolerance calibration for ASA or matte PLA
- Mechanical force/durability testing

## Files
- `design.py`: editable CadQuery 2.x source, shared shell stations, insert dimensions, and output exports
- `.step` files: editable CAD exchange models
- `.stl` files: printable parts

## CAD validation
The two exported meshes have been checked as individually watertight; assembled CAD solids have zero overlapping volume. This is *not* evidence that M3 inserts, physical screws, mating tolerances or connected electronics have been physically validated.
