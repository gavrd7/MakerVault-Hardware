import cadquery as cq, math
from pathlib import Path
P=Path(__file__).parent
# Rev C verified: OD12, height12, 10.5mm D bore.
# Rev D: +2 mm skirt and +2 mm internal socket, same remaining roof thickness.
OD=12.0; HEIGHT=14.0; BORE_DEPTH=12.5; RADIUS=3.05; FLAT_X=2.1
knob=cq.Workplane('XY').circle(OD/2).extrude(HEIGHT).edges('>Z').fillet(.7)
for i in range(24):
 a=2*math.pi*i/24
 x,y=6.22*math.cos(a),6.22*math.sin(a)
 knob=knob.cut(cq.Workplane('XY').center(x,y).circle(.45).extrude(12.0))
cavity=cq.Workplane('XY').circle(RADIUS).extrude(BORE_DEPTH)
# Preserve exact XY D bore profile from Rev C
half=cq.Workplane('XY').center(-4,0).box(12.2,12,13,centered=(True,True,False))
cavity=cavity.intersect(half)
knob=knob.cut(cavity)
for ext in ('stl','step'):
 cq.exporters.export(knob,str(P/f'MakerVault_D_Shaft_Knob_RevD.{ext}'), **({'tolerance':.035,'angularTolerance':.1} if ext=='stl' else {}))
print('exported')
