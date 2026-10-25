"""MakerVault handheld TFT carrier Rev C. Units millimetres.

Rev B original geometry is retained verbatim as base(); added edge guide segments
locate the real 29.5 x 43 mm PCB with 0.25 mm clearance per side.
This is a clearance-fit trial, NOT a clamping mechanism or finished enclosure.
"""
from pathlib import Path
import cadquery as cq
P=Path(__file__).resolve().parent
PCB_W, PCB_H =29.5,43.0
CLEARANCE_PER_SIDE=0.25
LOCATE_W=PCB_W+2*CLEARANCE_PER_SIDE  # 30.0 mm
LOCATE_H=PCB_H+2*CLEARANCE_PER_SIDE  # 43.5 mm
EAR_X, EAR_Y=23.8,15.0
THICKNESS=2.0
GUIDE_HEIGHT=1.35

def base():
    part=cq.Workplane('XY').box(36,49,THICKNESS,centered=(True,True,False)).edges('|Z').fillet(2)
    part=part.cut(cq.Workplane('XY').box(31,45,4,centered=(True,True,False)).translate((0,0,-1)))
    for sx in (-1,1):
        for sy in (-1,1):
            tab=cq.Workplane('XY').center(sx*15.15,sy*21.0).box(3,4,1,centered=(True,True,False))
            part=part.union(tab)
    for x in (-EAR_X, EAR_X):
        for y in (-EAR_Y,EAR_Y):
            part=part.union(cq.Workplane('XY').center(x,y).circle(3.2).extrude(2))
            part=part.union(cq.Workplane('XY').center(x*.85,y).box(8,5,2,centered=(True,True,False)))
            part=part.cut(cq.Workplane('XY').center(x,y).circle(1.15).extrude(3))
    return part

part=base()
# The *inside* faces of these four side segments lie at X +/- 15.0 mm.
# They stand outside the PCB outline and only contact its substrate side edges.
for side in (-1,1):
    for y in (-17.8,17.8):
        guide=cq.Workplane('XY').center(side*15.9,y).box(1.8,3.6,GUIDE_HEIGHT,centered=(True,True,False)).translate((0,0,THICKNESS))
        part=part.union(guide)
# Short end-edge lands: inner faces Y +/- 21.75 mm.  Keep the middle
# of the wiring edge fully open.  Position outside the TFT module footprint.
for end in (-1,1):
    for x in (-11.8,11.8):
        guide=cq.Workplane('XY').center(x,end*22.70).box(3.0,1.9,GUIDE_HEIGHT,centered=(True,True,False)).translate((0,0,THICKNESS))
        part=part.union(guide)

# Constraints are deliberately explicit so the printable guide opening
# cannot become smaller than the nominal PCB.
assert abs(2*(15.9-1.8/2)-LOCATE_W)<1e-7
assert abs(2*(22.70-1.9/2)-LOCATE_H)<1e-7
assert len(part.solids().vals())==1
cq.exporters.export(part,str(P/'display_carrier_revC.step'))
cq.exporters.export(part,str(P/'display_carrier_revC.stl'),tolerance=0.035,angularTolerance=0.1)
print('Carrier dimensions (mm):',part.val().BoundingBox().xlen,part.val().BoundingBox().ylen,part.val().BoundingBox().zlen)
print('PCB locating opening:',LOCATE_W,LOCATE_H,'| ear stations:',EAR_X,EAR_Y)
