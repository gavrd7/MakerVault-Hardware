# MakerVault handheld ESP32 cradle — Rev C (M2 compact posts)

This revision is derived from the validated Fastening Rev C shell geometry and the prior Rev B enclosed USB-C aperture. **Only** the ESP32 bridge receiver dimensions and matching bridge mounting pads/holes have changed. The front enclosure, TFT, controls, RFID position and overall 70 × 103 × 34 mm envelope remain unchanged.

## Printable parts

- `rear_shell_esp32_cradle_revC_m2.stl` — replacement rear shell (one)
- `usb_stop_bridge_revC_m2.stl` — matching removable retaining bridge (one)

Print **both** of these Rev C parts; the older Rev A/B bridge holes are sized for M3 and should not be substituted.

## M2 fastening

- Two compact rear-shell receiver posts: Ø5.6 mm outer diameter, 8 mm high; centers x=24 mm and 56 mm, y=87 mm.
- Blind M2 **thread-cutting pilot** Ø1.7 mm × ~6.2 mm, accessible from the cavity.
- Bridge M2 *clearance* holes Ø2.3 mm.
- Bridge thickness at mounts 2.6 mm.
- Candidate screws **M2 × 6 mm** (about 3.4 mm engagement) or **M2 × 8 mm** (about 5.4 mm engagement), conditional on checking actual depth, head access and whether the PCB is pinched. Screws are measured under the head.

**Important:** The owner's machine screws are blunt, constant-diameter screws, not self-tapping screws. Do not force them into an untapped Ø1.7 mm hole. Use an M2×0.4 hand tap (or verify a compatible proven thread-forming method on a coupon), clear debris and engage gently by hand. Smaller posts have lower torque/pull-out margins than M3 inserts; avoid overtightening or repeated cycling until validated.

## Assembly/checklist

1. Test the rear-shell mating outline against the unchanged front shell.
2. Confirm that the ESP32 with its actual side-GPIO wiring seats on the rear shell rails and against the USB inward stop.
3. Hand-cut and check threads in the M2 posts *before* adding electronics; keep chips out of the PCB and RFID cavity.
4. Install board and bridge; check screw head/access clearance, GPIO solder joints, ESP32 components, USB socket and cable. Do not tighten until the bridge contacts/supports the PCB at appropriate unpopulated edges; if it doesn't, stop rather than using screw pressure to bend the PCB.
5. Test repeated USB insertion/removal with the enclosure open and watch for movement, cracked posts or pinched wiring.

CAD geometry/mesh checks are not a physical validation. Nominal USB aperture is 12.4 × 7.0 mm. The exact wired board height and bridge/PCB load path remain to be validated with hardware.

`build.py` contains the editable CadQuery source; `original_shell_design.py` is bundled as its required imported baseline. STEP files supplied for both parts.
