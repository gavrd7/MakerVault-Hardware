"""MakerVault Rev D, dimensional button mechanism test (mm).
Coordinates: front panel external face Z=0; enclosure interior Z positive.
Switch dummy is for assembly inspection only, not a printable item.
"""
from pathlib import Path
import cadquery as cq
P=Path(__file__).resolve().parent
W,H,T=70.,43.,2.
ENC_X,BACK_X,CONF_X,Y=18.,43.,58.,20.
BUTTON_BORE=8.6
ENC_BORE=7.4
MOUNTS=((50.5,7.3),(50.5,32.7))


def cyl(x,y,r,z,h):
    return cq.Workplane('XY').center(x,y).circle(r).extrude(h).translate((0,0,z))

def block(x,y,sx,sy,z,d):
    return cq.Workplane('XY').center(x,y).rect(sx,sy).extrude(d).translate((0,0,z))

def panel():
    p=block(W/2,H/2,W,H,0,T).edges('|Z').fillet(8)
    for x,d in [(ENC_X,ENC_BORE),(BACK_X,BUTTON_BORE),(CONF_X,BUTTON_BORE)]:
        p=p.cut(cyl(x,Y,d/2,-.2,2.4))
        p=p.cut(cq.Solid.makeCone(d/2+.3,d/2,.3,cq.Vector(x,Y,0)))
        p=p.cut(cq.Solid.makeCone(d/2,d/2+.3,.3,cq.Vector(x,Y,T-.3)))
    p=p.cut(cyl(ENC_X,Y,5.3,1.45,.55))
    # Solid blind panel bosses, M2 clearance bores; metal nut recesses accessible from backside
    for x,y in MOUNTS:
        p=p.union(cyl(x,y,3.5,T,6.5))
        p=p.cut(cyl(x,y,1.12,4.05,4.7))
        p=p.cut(cq.Workplane('XY').center(x,y).polygon(6,4.65).extrude(2.1).translate((0,0,6.45)))
    return p

def cap():
    # Cap consists of a 7.4mm shaft/head running through 8.6mm clearance.
    # It is inserted from INSIDE; 10.2mm flange cannot pass through opening.
    # Local coordinates are ASSEMBLED z positions (no offsets required).
    c=cyl(0,0,3.7,-1.05,3.65) # protrudes 1.05 beyond panel, extends to z=2.6
    c=c.union(cyl(0,0,5.1,2.6,.65))
    c=c.union(cyl(0,0,2.25,3.25,.55)) # central plunger ends at z=3.8
    try: c=c.edges('<Z').fillet(.25)
    except Exception: pass
    return c

def carrier():
    # Rear carrier has open sidewalls along X so all 4 terminals exit unobstructed.
    # Back side of each 12mm switch rests on two separate Y-edge support rails.
    # Rails DO NOT cover the terminal edges and never touch the actuator.
    c=block(50.5,Y,33.,19.,8.5,1.7)
    for x in (BACK_X,CONF_X):
        # Open underside cutout removes backing under central pins and creates side exit paths.
        c=c.cut(block(x,Y,12.8,9.4,8.4,2.0))
        # Front-projecting Y-direction positioning lips on opposite sides of each switch.
        for yy in (Y-6.35,Y+6.35):
            c=c.union(block(x,yy,11.9,.75,6.25,2.25))
        # Trim away the top/bottom ribs outside this test part should remain one piece
    for x,y in MOUNTS:
        c=c.union(cyl(x,y,3.4,8.5,1.7))
        c=c.union(block(x,(y+Y)/2,4.0,abs(y-Y)+1.0,8.5,1.7))
        c=c.cut(cyl(x,y,1.15,8.35,2.1))
    return c

def washer(h):
    # Optional shims place behind carrier ears, one per fixing, to adjust operating gap.
    return cyl(0,0,3.25,0,h).cut(cyl(0,0,1.2,-.1,h+.2))

parts={'control_panel':panel(), 'button_cap':cap(), 'open_switch_carrier':carrier(),
       'shim_0p4':washer(.4),'shim_0p8':washer(.8)}
for name,obj in parts.items():
    cq.exporters.export(obj,str(P/(name+'.step')))
    cq.exporters.export(obj,str(P/(name+'.stl')),tolerance=.035,angularTolerance=.1)

assy=cq.Assembly(name='MakerVault_RevD_reference_position')
assy.add(parts['control_panel'],name='panel')
assy.add(parts['open_switch_carrier'],name='carrier')
for x,name in [(BACK_X,'back_button'),(CONF_X,'confirm_button')]:
    assy.add(parts['button_cap'].translate((x,Y,0)),name=name)
assy.save(str(P/'reference_assembly.step'))
print('Rev D generated')
