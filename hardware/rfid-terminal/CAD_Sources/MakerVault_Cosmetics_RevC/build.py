import cadquery as cq, trimesh, numpy as np, zipfile, xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image
P=Path(__file__).parent
# Original geometry, unchanged: 39x52x1, corner radius 3.6; optical hole 27.8x29 at y=-1.4
bezel=cq.Workplane('XY').box(39,52,1,centered=(True,True,False)).edges('|Z').fillet(3.6)
bezel=bezel.cut(cq.Workplane('XY').center(0,-1.4).box(27.8,29,3,centered=(True,True,False)).translate((0,0,-1)))
# coloured inlaid letters, printed flush on the LARGE band at +Y (installed bottom).
# Rotate wordmark 180 in the XY plane from Rev B, preserving reading direction when installed.
font='/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'
parts=[]
for s,dx in [('Maker',-1),('Vault',1)]:
    glyph=cq.Workplane('XY').text(s,4.5,.6,fontPath=font,halign='left',valign='bottom',combine=True)
    parts.append(glyph)
# position both words with small gap as wordmark centred; y=-22.9 original
# Final 180-degree rotation gives lettering on +Y, i.e. the LARGER bottom band.
bb=[a.val().BoundingBox() for a in parts]
widths=[b.xlen for b in bb]; gap=.25; left=-(sum(widths)+gap)/2
logos=[]
for i,(word,_) in enumerate([('Maker',0),('Vault',0)]):
    b=bb[i]; shiftx=left-b.xmin; shifty=-22.9-b.ymin
    logos.append(parts[i].translate((shiftx,shifty,.4)).rotate((0,0,0),(0,0,1),180))
    left+=widths[i]+gap
# subtract exactly same solids from bezel; the color inserts fill flush to top z=1
base=bezel
for g in logos:base=base.cut(g)
# 12mm diameter knob, 12mm tall. Existing D-fit unchanged in XY.
# D-socket extended 3mm for lower seating over exposed shaft.
knob=cq.Workplane('XY').circle(6).extrude(12).edges('>Z').fillet(.7)
# Flutes across grip circumference. Small side scallops, 24 around circumference.
import math
for i in range(24):
    a=2*math.pi*i/24
    x,y=6.22*math.cos(a),6.22*math.sin(a)
    knob=knob.cut(cq.Workplane('XY').center(x,y).circle(.45).extrude(10.0))
# D shape = circular socket and clipping against flat at x=2.1
cavity=cq.Workplane('XY').circle(3.05).extrude(10.5)
half=cq.Workplane('XY').center(-4,0).box(12.2,12,11,centered=(True,True,False))
cavity=cavity.intersect(half)
knob=knob.cut(cavity)
for name,part in [('flush_bezel_black',base),('wordmark_white_Maker',logos[0]),('wordmark_blue_Vault',logos[1]),('knurled_D_shaft_knob_12mm_tall',knob)]:
    cq.exporters.export(part,str(P/(name+'.stl')),tolerance=.035,angularTolerance=.1)
    cq.exporters.export(part,str(P/(name+'.step')))
    print(name,part.val().BoundingBox().xlen,part.val().BoundingBox().ylen,part.val().BoundingBox().zlen)
# Spec-compliant multiobject 3mf mesh with resources + basematerials; distinct printable parts
ns='http://schemas.microsoft.com/3dmanufacturing/core/2015/02'
ET.register_namespace('',ns)
model=ET.Element('{%s}model'%ns,{'unit':'millimeter','xml:lang':'en-US'})
resources=ET.SubElement(model,'{%s}resources'%ns)
materials=ET.SubElement(resources,'{%s}basematerials'%ns,{'id':'1'})
for n,col in [('Bezel black','#15191FFF'),('Maker white','#F7F7F7FF'),('Vault cyan','#00B4EFFF')]:ET.SubElement(materials,'{%s}base'%ns,{'name':n,'displaycolor':col})
for oid,name in enumerate(['flush_bezel_black','wordmark_white_Maker','wordmark_blue_Vault'],2):
    m=trimesh.load_mesh(str(P/(name+'.stl')))
    print('3mf part',name,'watertight',m.is_watertight,'bounds',m.bounds)
    obj=ET.SubElement(resources,'{%s}object'%ns,{'id':str(oid),'type':'model','pid':'1','pindex':str(oid-2),'name':name})
    mesh=ET.SubElement(obj,'{%s}mesh'%ns)
    vs=ET.SubElement(mesh,'{%s}vertices'%ns)
    for xyz in m.vertices:ET.SubElement(vs,'{%s}vertex'%ns,dict(zip('xyz',map(lambda f:format(float(f),'.5f'),xyz))))
    ts=ET.SubElement(mesh,'{%s}triangles'%ns)
    for tri in m.faces:ET.SubElement(ts,'{%s}triangle'%ns,dict(zip(('v1','v2','v3'),map(str,tri))))
build=ET.SubElement(model,'{%s}build'%ns)
for oid in (2,3,4):ET.SubElement(build,'{%s}item'%ns,{'objectid':str(oid)})
content=b'''<?xml version="1.0" encoding="UTF-8"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="model" ContentType="application/vnd.ms-package.3dmanufacturing-3dmodel+xml"/></Types>'''
rels=b'''<?xml version="1.0" encoding="UTF-8"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Target="/3D/3dmodel.model" Id="rel0" Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"/></Relationships>'''
with zipfile.ZipFile(P/'MakerVault_flush_wordmark_3color.3mf','w',zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml',content); z.writestr('_rels/.rels',rels)
    z.writestr('3D/3dmodel.model',ET.tostring(model,encoding='utf-8',xml_declaration=True))
