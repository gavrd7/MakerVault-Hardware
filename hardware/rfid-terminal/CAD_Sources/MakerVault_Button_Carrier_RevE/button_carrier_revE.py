"""MakerVault button carrier Rev E: wiring relief for soldered switch legs.
Derived from the tested Rev D open switch carrier.
Dimensions in mm; retain exact mounting ear positions and overall Z.
"""
from pathlib import Path
import cadquery as cq
P=Path(__file__).resolve().parent
BACK_X,CONF_X,Y=43.,58.,20.
MOUNTS=((50.5,7.3),(50.5,32.7))

def block(x,y,sx,sy,z,d):
    return cq.Workplane('XY').center(x,y).rect(sx,sy).extrude(d).translate((0,0,z))
def cyl(x,y,r,z,h):
    return cq.Workplane('XY').center(x,y).circle(r).extrude(h).translate((0,0,z))

def carrier():
    c=block(50.5,Y,33.,19.,8.5,1.7)
    for x in (BACK_X,CONF_X):
        # Original central open-backed switch cavities, retained at 12.8 x 9.4.
        c=c.cut(block(x,Y,12.8,9.4,8.4,2.0))
        # Revised soldered-terminal relief: +1 mm per side in X. Cut locally
        # into the *rear-facing* half only. This keeps more material elsewhere.
        # Relief opens at the switch cutout and accommodates side solder joints.
        c=c.cut(block(x,Y,14.8,9.4,8.4,1.05))  # to z=9.45
        for yy in (Y-6.35,Y+6.35):
            c=c.union(block(x,yy,11.9,.75,6.25,2.25))
    for x,y in MOUNTS:
        c=c.union(cyl(x,y,3.4,8.5,1.7))
        c=c.union(block(x,(y+Y)/2,4.0,abs(y-Y)+1.0,8.5,1.7))
        c=c.cut(cyl(x,y,1.15,8.35,2.1))
    return c

part=carrier()
cq.exporters.export(part,str(P/'open_switch_carrier_revE.stl'),tolerance=.035,angularTolerance=.1)
cq.exporters.export(part,str(P/'open_switch_carrier_revE.step'))
print('volume',part.val().Volume(),'bbox',part.val().BoundingBox().xlen,part.val().BoundingBox().ylen,part.val().BoundingBox().zlen,'solids',len(part.solids().vals()))
