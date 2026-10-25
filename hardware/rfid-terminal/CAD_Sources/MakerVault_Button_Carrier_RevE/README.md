# MakerVault Button Carrier Rev E — soldered-terminal clearance

**Only the button carrier changes.** Reuse the final enclosure, button caps, encoder, and all other hardware.

## Geometry change from Rev D

- Switch centre locations: unchanged (X = 43 and 58 mm).
- M2 carrier mounting-ear centres: unchanged (X = 50.5, Y = 7.3 and 32.7 mm).
- Carrier mounting depth and envelope: unchanged.
- Existing main switch apertures: 12.8 mm wide; original vertical switch-location guide rails retained.
- At the terminal-contact layer, relief on *both* lateral sides of *both* apertures is expanded by 1 mm each side: 14.8 mm nominal width.
- Relief depth into the 1.7 mm carrier body is 0.95 mm (Z=8.5 to 9.45), leaving material at the higher Z side around the widened region.

The switch centres are only 15 mm apart. A 14.8 mm full-depth opening at each centre would leave only ~0.2 mm between them, so this relief is **stepped**, not a full-depth enlargement. This is a clearance test for the actual soldered terminals and has not yet been physically validated.

## Test

1. Print `open_switch_carrier_revE.stl` flat at 0.2 mm layer height with 3 walls, in the same material used for the existing carrier.
2. Test with both **already soldered** tactile switches; do not force pins or joints past plastic.
3. Confirm that each switch seats at its intended depth, neither terminal/solder joint is pinched, and both buttons actuate normally.
4. Reuse existing M2 fastening points to check fit to your enclosure. Do not overtighten.

If the solder joints still make contact, we can increase the *local pocket depth* based on your physical test, without disturbing the proven 15 mm centres or carrier mount holes.

Included: STL, STEP and editable CadQuery source.
