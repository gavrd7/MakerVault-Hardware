"""MakerVault paired-shell fastening prototype Rev C. Requires CadQuery 2.x.
All features in common coordinates: X 0..70, Y 0..103, Z front 0..34.
PLA test coupon verified nominal 4.2 mm bore with 4.5 x 5.6 mm M3 heat-set inserts; other materials require recalibration.
"""
from pathlib import Path
import cadquery as cq
P=Path(__file__).resolve().parent
STATIONS=[(7.,9.),(63.,9.),(9.,95.),(63.,94.)]
# Tested in PLA only: M3 heat-set insert nominal OD 4.5 mm, length 5.6 mm.
INSERT_BORE_D=4.2
INSERT_DEPTH=6.1
M3_CLEARANCE_D=3.35
M3_HEAD_BORE_D=6.5
FASTENER_BOSS_R=4.8
def box(w,h,d,x,y,z,r=0):
 s=cq.Workplane('XY').box(w,h,d,centered=(True,True,False)).translate((x,y,z))
 return s.edges('|Z').fillet(r) if r else s
def cyl(x,y,r,z,h):return cq.Workplane('XY').center(x,y).circle(r).extrude(h).translate((0,0,z))
# Preserve physically tested front geometry, including 1.6 mm display support posts
front=box(70,103,12,35,51.5,0,12).cut(box(66,99,13,35,51.5,2,10))
front=front.cut(box(39.48,52.48,1,35,31.5,0,3.8))
front=front.cut(box(29.4,35.7,3,35,32.75,-.2))
front=front.cut(box(30.5,44.2,1.2,35,31.5,1))
front=front.cut(box(22,9,1.2,35,5.5,1))
for x in [35-23.8,35+23.8]:
 for y in [31.5-15,31.5+15]:
  front=front.union(cyl(x,y,3,2,1.6)).cut(cyl(x,y,1.1,1.9,2))
for x,d in [(18,7.4),(43,8.2),(58,8.2)]:
 front=front.cut(cyl(x,72,d/2,-.2,2.5))
 front=front.cut(cq.Solid.makeCone(d/2+.3,d/2,.3,cq.Vector(x,72,0)))
 front=front.cut(cq.Solid.makeCone(d/2,d/2+.3,.3,cq.Vector(x,72,1.7)))
front=front.cut(cyl(18,72,5.3,1.45,.65))
for yy in (7.3,32.7):
 x,y=50.5,72-20+yy
 front=front.union(cyl(x,y,3.5,2,6.5))
 front=front.cut(cyl(x,y,1.12,4.05,4.7))
 front=front.cut(cq.Workplane('XY').center(x,y).polygon(6,4.65).extrude(2.1).translate((0,0,6.45)))
# Mating lip: a narrow 0.75-high band retained at the inner side of the front rim.
# Projected into rear recess with per-side ~0.25 clearance.
lip=box(67.2,100.2,.75,35,51.5,12,10.6).cut(box(65.0,98.0,1,35,51.5,11.9,9.5))
front=front.union(lip)
# Main fastener interface: M3 heat-set receivers in the FRONT shell.
# Insert from internal boss face Z=18 towards front; pocket extends to Z=11.9.
for x,y in STATIONS:
 front=front.union(cyl(x,y,FASTENER_BOSS_R,2,16.0)) # end Z18
 # Blind insert bore; leave material below the insert to protect outer front surface.
 front=front.cut(cyl(x,y,INSERT_BORE_D/2,18-INSERT_DEPTH,INSERT_DEPTH+0.15))
# Rear shell: retain RF-facing floor, reader bosses and ESP guides exactly.
rear=box(70,103,22,35,51.5,12,12).cut(box(66,99,20.3,35,51.5,12,10))
rear=rear.cut(box(13,5.5,19,40,101.5,12))
# Corresponding shallow lip receiver in rear wall; position matches front shell lip.
rear=rear.cut(box(67.8,100.8,1.15,35,51.5,11.95,10.9).cut(box(64.5,97.5,1.5,35,51.5,11.8,9.25)))
for x,y in [(18,23),(52,23),(22.8,60.9),(47.2,60.9)]:
 rear=rear.union(cyl(x,y,2.2,28.5,3.8)).cut(cyl(x,y,1.1,28.45,3.95))
for x,y in [(29.5,81),(50.5,81),(29.5,94),(50.5,94)]:
 rear=rear.union(box(2,5,6.1,x,y,26.2))
rear=rear.union(box(14,1.2,3.2,40,76.7,29.1))
# From-rear screw tunnels: M3 screw head counterbore and shank clearance.
# Screw-head underside is nominally Z=20.4. An M3 x 6 mm screw is the candidate: reach approx Z=14.4,
# provided the actual head and insert geometry agree. Measure before tightening.
for x,y in STATIONS:
 rear=rear.union(cyl(x,y,FASTENER_BOSS_R,17.9,14.4))
 rear=rear.cut(cyl(x,y,M3_HEAD_BORE_D/2,20.4,14.0))
 rear=rear.cut(cyl(x,y,M3_CLEARANCE_D/2,17.7,2.9))
# Relief for the front receivers: the rounded rear inner cavity does not
# fully clear corner bosses. Cut a true shared interface (0.25 radial gap).
for x,y in STATIONS:
 rear=rear.cut(cyl(x,y,FASTENER_BOSS_R+0.25,11.95,6.3))
# Model for ESP retainer is deliberately omitted: existing wired board + USB need measured height.
for name,solid in [('front_fastening_revC',front),('rear_fastening_revC',rear)]:
 shape=solid.val(); b=shape.BoundingBox()
 print(name,'solids',len(solid.solids().vals()),'bbox',b.xlen,b.ylen,b.zlen,'volume',shape.Volume())
 cq.exporters.export(solid,str(P/(name+'.stl')),tolerance=.055,angularTolerance=.14)
 cq.exporters.export(solid,str(P/(name+'.step')))
