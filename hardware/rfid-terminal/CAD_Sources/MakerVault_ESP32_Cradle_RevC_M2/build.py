"""MakerVault ESP32 supported cradle Rev C M2 screw mounting - derivative of Fastening Rev C.
Uses existing source for all unchanged geometry. CAD assumptions annotated in README.
"""
from pathlib import Path
import cadquery as cq
from cadquery import exporters
P=Path(__file__).parent
src=(P/'original_shell_design.py').read_text()
# Original open-top USB notch: 13 x 5.5 x 19 mm from Z12.
# Replace with local 12.4 x 6.0 x 7.0 mm access aperture, Z25.5..32.5.
# The opening is within the end wall at Y=103, not into the rear scanning wall.
src=src.replace('rear=rear.cut(box(13,5.5,19,40,101.5,12))',
                'rear=rear.cut(box(12.4,6.0,7.0,40,101.5,25.5))')
# Remove source export loop to avoid side effects, and replace guide geometry exclusively.
a=src.index('for x,y in [(29.5,81)')
b=src.index('# From-rear screw tunnels:',a)
src=src[:a]+'''# ESP32 cradle: continuous shallow rails are outside the side-pad corridor.
# At least a narrow side gap is retained for soldered leads, with an open roof.
# PCB nominal X31..49, Y77.5..101.0, thickness TBD on assembled board.
# Two cradling ledges support board edges from below on wiring-free bottom face.
for x in (30.1,49.9):
 rear=rear.union(box(2.0,22.0,2.0,x,89.0,30.25))
# End stop opposes inward USB plug force (toward Y-), not friction caps.
rear=rear.union(box(14.0,2.0,3.5,40,76.35,28.8))
# USB-side lateral stop shoulders, kept outside plug envelope at X~34..46.
for x in (29.0,51.0):
 rear=rear.union(box(2.0,3.0,3.5,x,98.5,28.8))
# Four-sided hardware access remains open above side GPIO pads.
# Two compact M2 machine-thread receiver posts support a detachable bridge, away from PCB and RF zone.
for x in (24.0,56.0):
 rear=rear.union(cyl(x,87.0,2.8,24.3,8.0))
 # Blind 1.7 mm pilot for a hand-cut M2x0.4 thread; do not force a machine screw.
 rear=rear.cut(cyl(x,87.0,0.85,24.25,6.25))
''' + src[b:src.index('for name,solid in')]
ns={'__file__':str(P/'original_shell_design.py')}
exec(compile(src,str(P/'original_shell_design.py'),'exec'),ns)
front,rear=ns['front'],ns['rear']
# Bridge top surface at z24.3, with M2 screws through bridge into hand-tapped PLA/PETG receiver posts.
# Saddle arms outside side pads, with small retention lips at board extreme short edges.
bridge=cq.Workplane('XY')
for x in (24,56): bridge=bridge.union(cq.Workplane('XY').center(x,87).circle(2.65).extrude(2.6).translate((0,0,21.7)))
# Crossbar is routed below (y smaller) the PCB and ESP antenna end.
bridge=bridge.union(cq.Workplane('XY').box(36,3.0,2.6,centered=(True,True,False)).translate((40,74.0,21.7)))
for x in (24,56):
 bridge=bridge.union(cq.Workplane('XY').box(3.0,14.0,2.6,centered=(True,True,False)).translate((x,80,21.7)))
 # Bridge anchors only at these two screws, no inboard GPIO-spanning crossbar.
 bridge=bridge.cut(cq.Workplane('XY').center(x,87).circle(1.15).extrude(3.2).translate((0,0,21.5)))
# At front edge of PCB short end, two PCB-end-corner hooks: NONCOMPRESSIVE, need real board verification.
# Design conservative: overhang ends at y=77.7, no overlap on populated board plane.
for x in (31.6,48.4):
 bridge=bridge.union(cq.Workplane('XY').box(2.2,6.8,5.0,centered=(True,True,False)).translate((x,76.0,24.0)))
# add no PCB-clamping feature until physical test establishes z offset. 2.5mm loose tolerance.
rear_shape=rear.val(); bridge_shape=bridge.val()
# output
for name,obj in [('rear_shell_esp32_cradle_revC_m2',rear),('usb_stop_bridge_revC_m2',bridge)]:
 sh=obj.val(); bb=sh.BoundingBox();print(name,'solids',len(obj.solids().vals()),'size',tuple(round(v,2) for v in (bb.xlen,bb.ylen,bb.zlen)),'volume',round(sh.Volume(),1))
 exporters.export(obj,str(P/(name+'.stl')),tolerance=.05,angularTolerance=.12)
 exporters.export(obj,str(P/(name+'.step')))
# intersections + nominal wire/component keepouts:
common=front.val().intersect(rear_shape).Volume()
print('front-rear interference mm3',round(common,5))
print('bridge-rear interference mm3',round(rear_shape.intersect(bridge_shape).Volume(),5))
