import cadquery as cq
from pathlib import Path
P=Path(__file__).parent
# All coordinates in mm, centre of panel / bezel. Z increases toward enclosure interior.
PANEL_W,PANEL_H,PANEL_T=60.,66.,2.
BEZEL_W,BEZEL_H,BEZEL_T=39.,52.,1.0
BEZEL_CLEAR=.24
PCB_W,PCB_H=29.5,43.
ACTIVE_W,ACTIVE_H=28.,29.2
ACTIVE_TOP=-16. # PCB upper edge is -22; +6
ACTIVE_BOTTOM=13.2
HOLE_W,HOLE_H=27.8,29.0
SCREEN_CY=(ACTIVE_TOP+ACTIVE_BOTTOM)/2
# Front plane z=0, inside panel z=2
panel=cq.Workplane('XY').box(PANEL_W,PANEL_H,PANEL_T,centered=(True,True,False)).edges('|Z').fillet(5)
# Pocket from outer surface for bezel, nominal .95 deep
recess=cq.Workplane('XY').box(BEZEL_W+BEZEL_CLEAR*2,BEZEL_H+BEZEL_CLEAR*2,1.0,centered=(True,True,False)).edges('|Z').fillet(3.8)
panel=panel.cut(recess)
# main opening covers display module, allows nonviewable tail to extend under bezel
# actual display module side boundaries must be verified: optical aperture controlled by bezel
module_cut=cq.Workplane('XY').center(0,1.25).box(29.4,35.7,4,centered=(True,True,False)).translate((0,0,-1))
panel=panel.cut(module_cut)
# recess behind skin for complete PCB, all eight solder pads + lead routing top (rear side)
# PCB does not pass through panel front; cavity near the rear at z=1.0
pcb_cut=cq.Workplane('XY').box(30.5,44.2,1.05,centered=(True,True,False)).translate((0,0,1.0))
panel=panel.cut(pcb_cut)
# carve route at top for wiring, exposed to the rear
wire=cq.Workplane('XY').center(0,-26).box(22,9,1.1,centered=(True,True,False)).translate((0,0,1.0))
panel=panel.cut(wire)
# Small boss stations outside the PCB footprint, integrated with panel
for x in [-23.8,23.8]:
 for y in [-15,15]:
  boss=cq.Workplane('XY').center(x,y).circle(3).extrude(4).translate((0,0,2))
  panel=panel.union(boss)
  # 2.2 mm clearance for M2 bolts, use captive nuts in later version; Rev A friction-fit M2 through hole without thread promise
  panel=panel.cut(cq.Workplane('XY').center(x,y).circle(.95).extrude(4.3).translate((0,0,2)))
# Replaceable bezel optical aperture. Recess fills front from z=0 to z=1
bezel=cq.Workplane('XY').box(BEZEL_W,BEZEL_H,BEZEL_T,centered=(True,True,False)).edges('|Z').fillet(3.6)
bezel=bezel.cut(cq.Workplane('XY').center(0,SCREEN_CY).box(HOLE_W,HOLE_H,3,centered=(True,True,False)).translate((0,0,-1)))
# Rear carrier open centre: outside PCB, four tiny corner retaining pads touch PCB underside only at outer corners
carrier=cq.Workplane('XY').box(36,49,2,centered=(True,True,False)).edges('|Z').fillet(2)
carrier=carrier.cut(cq.Workplane('XY').box(31,45,4,centered=(True,True,False)).translate((0,0,-1)))
# four lightweight retaining tabs project inward, contact near PCB corners, avoid glass and wires
for x in [-1,1]:
 for y in [-1,1]:
  tab=cq.Workplane('XY').center(x*15.3,y*20.9).box(2,3,1.0,centered=(True,True,False)).translate((0,0,0))
  carrier=carrier.union(tab)
# mounting wings aligned to panel bosses; open behind screen
for x in [-23.8,23.8]:
 for y in [-15,15]:
  wing=cq.Workplane('XY').center(x,y).circle(3.2).extrude(2)
  # short bridge from frame to wing
  bridge=cq.Workplane('XY').center(x*.85,y).box(8,5,2,centered=(True,True,False))
  carrier=carrier.union(wing).union(bridge)
  carrier=carrier.cut(cq.Workplane('XY').center(x,y).circle(1.15).extrude(3))
# avoid false claims about fully validated assembly, no nuts or M2 screw holes threaded
for name,part in [('front_coupon',panel),('flush_bezel',bezel),('display_carrier',carrier)]:
 cq.exporters.export(part,str(P/f'{name}.step'))
 cq.exporters.export(part,str(P/f'{name}.stl'),tolerance=.05,angularTolerance=.12)
 print(name,part.val().BoundingBox().xlen,part.val().BoundingBox().ylen,part.val().BoundingBox().zlen)
