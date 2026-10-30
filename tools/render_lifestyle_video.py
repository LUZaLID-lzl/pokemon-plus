"""Blender 4.5: render real LVGL animation into a photo-referenced device.

Run with Blender --background --python this-file -- --shot 0 --preview.
The enclosure is an appearance approximation, not mechanical CAD.
"""
import argparse
import math
import sys
from pathlib import Path
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build/lifestyle-v03328'
parser = argparse.ArgumentParser()
parser.add_argument('--shot', type=int, default=0)
parser.add_argument('--preview', action='store_true')
parser.add_argument('--start', type=int, default=1)
parser.add_argument('--end', type=int, default=120)
args = parser.parse_args(sys.argv[sys.argv.index('--')+1:])
SHOT = args.shot
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.render.engine = 'BLENDER_EEVEE_NEXT'
scene.eevee.taa_render_samples = 24
scene.render.resolution_x = 720
scene.render.resolution_y = 1280
scene.render.resolution_percentage = 100
scene.render.fps = 24
scene.render.image_settings.file_format = 'PNG'
scene.render.film_transparent = False
scene.frame_start, scene.frame_end = args.start, args.end
scene.world.color = (.16,.16,.16)
scene.view_settings.view_transform = 'AgX'
scene.render.use_file_extension = True

def material(name, color, rough=.4, metal=0):
    m=bpy.data.materials.new(name); m.diffuse_color=(*color,1); m.use_nodes=True
    p=m.node_tree.nodes.get('Principled BSDF')
    p.inputs['Base Color'].default_value=(*color,1)
    p.inputs['Roughness'].default_value=rough
    p.inputs['Metallic'].default_value=metal
    return m

metal=material('Champagne anodized aluminium',(.68,.62,.48),.24,.82)
body=material('Graphite sidewall',(.035,.041,.043),.34,.6)
black=material('Polished black bezel',(.009,.012,.014),.17,.25)
cream=material('Ivory ceramic',(.72,.68,.59),.25)
paper=material('Warm paper',(.73,.70,.60),.8)
green=material('Deep green bookcloth',(.035,.10,.078),.85)
clay=material('Terracotta',(.30,.12,.065),.88)
leafmat=material('Living green leaves',(.055,.19,.075),.52)
brass=material('Brushed warm brass',(.43,.25,.085),.28,.8)
coffee=material('Coffee',(.035,.015,.007),.12)

def assign(obj,mat):
    obj.data.materials.append(mat)
    return obj

def bevel(obj,width=.001,segments=3):
    mod=obj.modifiers.new('Soft manufactured edges','BEVEL');mod.width=width;mod.segments=segments
    mod=obj.modifiers.new('Weighted corner normals','WEIGHTED_NORMAL')
    return obj

def cube(name,loc,scale,mat,edge=.001):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc)
    o=bpy.context.object;o.name=name;o.dimensions=scale
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    assign(o,mat)
    if edge:bevel(o,edge)
    return o

def cylinder(name,r,depth,loc,mat,vertices=96):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=r,depth=depth,location=loc)
    o=bpy.context.object;o.name=name;assign(o,mat);bevel(o,min(.0006,depth/5))
    for p in o.data.polygons:p.use_smooth=True
    return o

def lathe(name,profile,mat,loc=(0,0,0),n=128):
    verts=[];faces=[]
    for r,z in profile:
        verts.extend((r*math.cos(j*2*math.pi/n),r*math.sin(j*2*math.pi/n),z) for j in range(n))
    for k in range(len(profile)-1):
        for j in range(n):
            a=k*n+j;b=k*n+(j+1)%n
            faces.append((a,b,b+n,a+n))
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update()
    o=bpy.data.objects.new(name,mesh);scene.collection.objects.link(o);o.location=loc;assign(o,mat)
    for p in mesh.polygons:p.use_smooth=True
    return o

def torus(name,major,minor,loc,mat,rot=(0,0,0)):
    bpy.ops.mesh.primitive_torus_add(major_radius=major,minor_radius=minor,major_segments=64,minor_segments=12,location=loc,rotation=rot)
    o=bpy.context.object;o.name=name;assign(o,mat)
    for p in o.data.polygons:p.use_smooth=True
    return o

def curve(name,coords,r,mat):
    c=bpy.data.curves.new(name,'CURVE');c.dimensions='3D';c.bevel_depth=r;c.bevel_resolution=3
    sp=c.splines.new('BEZIER');sp.bezier_points.add(len(coords)-1)
    for p,co in zip(sp.bezier_points,coords):p.co=co;p.handle_left_type='AUTO';p.handle_right_type='AUTO'
    o=bpy.data.objects.new(name,c);scene.collection.objects.link(o);assign(o,mat);return o

def aim(o,target):o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()

def area(name,loc,energy,color,size,target=(0,0,.02)):
    d=bpy.data.lights.new(name,'AREA');d.energy=energy;d.color=color;d.shape='DISK';d.size=size
    o=bpy.data.objects.new(name,d);scene.collection.objects.link(o);o.location=loc;aim(o,target);return o

# Real oak texture with physical-scale grain and subtle pore normal.
wood=material('Pale oak / MobiIoT generated albedo',(.5,.35,.2),.58)
nodes=wood.node_tree.nodes;links=wood.node_tree.links;p=nodes.get('Principled BSDF')
tex=nodes.new('ShaderNodeTexImage');tex.image=bpy.data.images.load(str(ROOT/'assets/source/lifestyle-v03328/oak.png'))
coord=nodes.new('ShaderNodeTexCoord');mapping=nodes.new('ShaderNodeMapping');mapping.inputs['Scale'].default_value=(4,4,4)
links.new(coord.outputs['Generated'],mapping.inputs[0]);links.new(mapping.outputs[0],tex.inputs[0]);links.new(tex.outputs['Color'],p.inputs['Base Color'])
bump=nodes.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.12;bump.inputs['Distance'].default_value=.0002
links.new(tex.outputs['Color'],bump.inputs['Height']);links.new(bump.outputs[0],p.inputs['Normal'])
if SHOT==1:
    tint=nodes.new('ShaderNodeMixRGB');tint.blend_type='MULTIPLY';tint.inputs[0].default_value=1;tint.inputs[2].default_value=(.24,.16,.10,1)
    links.new(tex.outputs['Color'],tint.inputs[1]);links.new(tint.outputs[0],p.inputs['Base Color'])
cube('Solid oak tabletop',(0,0,-.012),(.9,.8,.024),wood,.006)
wallmat=material('Plaster wall',[(.55,.58,.52),(.38,.45,.43),(.11,.13,.16)][SHOT],.95)
cube('Room wall',(0,.30,.25),(1,.018,.55),wallmat,.002)

# Device: no strap or lugs; proportions and finish based on the saved real photo.
root=bpy.data.objects.new('Photo referenced round device',None);scene.collection.objects.link(root)
root.location=(0,0,.0262);root.rotation_euler=(math.radians(62),0,math.radians(-4))
def local(o):o.parent=root;return o
local(cylinder('Graphite housing',.0278,.008,(0,0,-.004),body))
local(lathe('Champagne rim',[(.027,-.0075),(.028,-.0068),(.028,.0001),(.0277,.0012),(.0264,.0016),(.0259,.0011),(.0259,-.001)],metal))
local(lathe('Gloss black inset bezel',[(.0259,.001),(.0258,.00155),(.0240,.00155),(.0235,.0011)],black))
button=local(cylinder('Small right hand metal button',.002,.002,(.028,.004,-.003),metal,48));button.rotation_euler[1]=math.pi/2
for y in [-.008,.008]:
    notch=local(cube('Case seam detail',(.0277,y,-.004),(.0003,.0018,.0025),black,.0001))

screenmat=bpy.data.materials.new('Actual current LVGL framebuffer');screenmat.use_nodes=True
nt=screenmat.node_tree;nt.nodes.clear()
out=nt.nodes.new('ShaderNodeOutputMaterial');em=nt.nodes.new('ShaderNodeEmission');em.inputs['Strength'].default_value=1.12
tx=nt.nodes.new('ShaderNodeTexImage');kind=['scene','battle','warehouse'][SHOT]
tx.image=bpy.data.images.load(str(BUILD/('ui-'+kind)/'000.png'));tx.image.source='SEQUENCE';tx.interpolation='Closest'
tx.image_user.frame_duration=120;tx.image_user.frame_start=1;tx.image_user.use_auto_refresh=True
nt.links.new(tx.outputs['Color'],em.inputs[0]);nt.links.new(em.outputs[0],out.inputs['Surface'])
verts=[(0,0,.00165)]+[(.0237*math.cos(i*2*math.pi/192),.0237*math.sin(i*2*math.pi/192),.00165) for i in range(192)]
faces=[(0,i+1,(i+1)%192+1) for i in range(192)]
mesh=bpy.data.meshes.new('Circular display with full framebuffer UV');mesh.from_pydata(verts,[],faces);mesh.uv_layers.new()
for poly in mesh.polygons:
    for li in poly.loop_indices:
        v=mesh.vertices[mesh.loops[li].vertex_index].co
        mesh.uv_layers.active.data[li].uv=(v.x/.0474+.5,v.y/.0474+.5)
o=bpy.data.objects.new('Live 360 pixel display',mesh);scene.collection.objects.link(o);assign(o,screenmat);local(o)

def book(name,xy,angle,covermat,width=.09,depth=.12,height=.012):
    x,y=xy
    parts=[cube(name+' pages',(x,y,height/2+.002),(width-.003,depth-.003,height),paper,.0005),
           cube(name+' bottom cover',(x,y,.001),(width,depth,.002),covermat,.001),
           cube(name+' top cover',(x,y,height+.003),(width,depth,.002),covermat,.001)]
    for o in parts:o.rotation_euler[2]=angle
    # Embossed simple line on the cover, no generated labels.
    return height+.004

def mug(loc,mat=cream):
    x,y,z=loc
    lathe('Ceramic mug',[(.018,0),(.022,.004),(.023,.044),(.022,.046),(.020,.046),(.019,.006),(.001,.005)],mat,loc)
    torus('Mug handle',.013,.003,(x+.027,y,z+.025),mat,(math.pi/2,0,0))
    cylinder('Coffee surface',.0195,.001,(x,y,z+.038),coffee)
    lathe('Saucer',[(.001,.001),(.027,.001),(.032,.004),(.032,.006),(.025,.004),(.001,.003)],mat,(x,y,z-.001))
    steam=material('Soft coffee vapour',(.58,.56,.51),1)
    sp=steam.node_tree.nodes.get('Principled BSDF');sp.inputs['Alpha'].default_value=.09
    steam.surface_render_method='DITHERED'
    for j in range(3):
        co=[(x+(j-1)*.007+math.sin(k*1.7+j)*.003,y+math.cos(k+j)*.003,z+.041+k*.011) for k in range(5)]
        o=curve('Drifting coffee steam',co,.00065,steam)
        for f,s in [(1,-1),(60,1),(120,-1)]:
            o.rotation_euler[2]=s*.13;o.location.x=s*.002;o.keyframe_insert('rotation_euler',frame=f);o.keyframe_insert('location',frame=f)

def plant(x,y):
    lathe('Plant pot',[(.014,0),(.018,.03),(.019,.031),(.019,.034),(.016,.034),(.013,.005)],clay,(x,y,0))
    soil=material('Potting soil',(.035,.022,.012),.99);cylinder('Soil',.016,.001,(x,y,.029),soil)
    for j in range(9):
        ang=j*2.399;h=.048+(j%3)*.013;r=.021+(j%2)*.01
        end=Vector((x+math.cos(ang)*r,y+math.sin(ang)*r,h))
        curve('Plant stem',[(x,y,.027),(x+(end.x-x)*.3,y+(end.y-y)*.3,h*.8),end],.0006,leafmat)
        mid=end+Vector((math.cos(ang)*.012,math.sin(ang)*.012,.006));tip=end+Vector((math.cos(ang)*.030,math.sin(ang)*.030,.009))
        side=Vector((-math.sin(ang)*.009,math.cos(ang)*.009,0))
        vs=[end,mid+side,tip,mid-side,mid+Vector((0,0,.003))]
        me=bpy.data.meshes.new('Leaf');me.from_pydata(vs,[],[(0,1,4),(1,2,4),(2,3,4),(3,0,4)]);me.update()
        ob=bpy.data.objects.new('Swaying leaf',me);scene.collection.objects.link(ob);assign(ob,leafmat)
        sol=ob.modifiers.new('Leaf thickness','SOLIDIFY');sol.thickness=.00015
        for f,a in [(1,-.016),(60,.016),(120,-.016)]:ob.rotation_euler[1]=a;ob.keyframe_insert('rotation_euler',frame=f)

if SHOT==0:
    book('Linen notebook',(-.073,.025),-.22,green)
    mug((.065,.065,0));plant(-.087,.16)
    pencil=material('Graphite pencil wood',(.17,.22,.15),.6)
    o=cylinder('Desk pencil',.0024,.11,(-.075,-.012,.017),pencil,6);o.rotation_euler=(0,math.pi/2,.32)
    area('Morning window',(-.20,-.08,.30),9,(1,.86,.66),.21)
    area('Cool sky bounce',(.16,.08,.20),4,(.69,.83,1),.18)
elif SHOT==1:
    mug((-.075,.07,0),material('Cafe sage cup',(.20,.30,.25),.25))
    book('Cafe pocket journal',(.075,.075),.2,material('Cognac leather',(.19,.075,.032),.5),.085,.115,.008)
    # A napkin and spoon anchor the device in an everyday cafe table setting.
    linen=material('Natural linen',(.53,.48,.39),.95)
    cube('Folded linen napkin',(.076,-.016,.003),(.069,.075,.005),linen,.002)
    o=curve('Teaspoon handle',[(.065,-.04,.008),(.067,-.014,.008),(.071,.005,.008)],.0013,metal)
    bpy.ops.mesh.primitive_uv_sphere_add(segments=24,ring_count=12,location=(.073,.012,.008))
    o=bpy.context.object;o.name='Teaspoon bowl';o.scale=(.0045,.008,.0013);assign(o,metal)
    area('Cafe window',(.04,-.12,.3),10,(1,.91,.75),.24)
    area('Cafe cool fill',(-.18,.1,.19),3,(.59,.77,1),.18)
    plant(.16,.22)
else:
    mat=material('Linen desk mat',(.23,.24,.22),.98)
    cube('Soft desk mat',(.018,-.005,.001),(.17,.14,.002),mat,.006)
    book('Evening reading',(-.075,.087),-.14,green,.10,.14,.018)
    # Brass bedside lamp, visible at upper right and casting warm light.
    cylinder('Lamp base',.035,.005,(.092,.102,.003),brass)
    cylinder('Lamp stem',.0025,.12,(.092,.102,.065),brass)
    shade=material('Linen lamp shade',(.78,.61,.38),.82)
    lathe('Warm lampshade',[(.048,.10),(.027,.17),(.025,.17),(.046,.10)],shade,(.092,.102,0))
    area('Warm lamp pool',(.092,.10,.11),3,(1,.60,.26),.045)
    area('Blue hour window',(-.16,-.12,.21),2.8,(.43,.63,1),.19)
    area('Soft face fill',(.02,-.20,.12),1.2,(.86,.87,1),.13)

# Window mullions and their diagonal shadows provide a room beyond the desk.
for x in [-.22,-.06,.10,.26]:
    cube('Window frame',(x,.285,.20),(.007,.012,.38),cream,.0006)

camdata=bpy.data.cameras.new('Portrait product camera');cam=bpy.data.objects.new('Portrait product camera',camdata);scene.collection.objects.link(cam);scene.camera=cam
camdata.lens=66;camdata.sensor_width=36;camdata.sensor_fit='HORIZONTAL'
focus=bpy.data.objects.new('Screen focus',None);scene.collection.objects.link(focus);focus.location=(0,0,.028)
camdata.dof.use_dof=True;camdata.dof.focus_object=focus;camdata.dof.aperture_fstop=7.1
for f in [1,60,120]:
    t=(f-1)/119
    angle=math.radians((-6+12*t) if SHOT==0 else (7-14*t) if SHOT==1 else (10-15*t))
    distance=.173-.009*math.sin(t*math.pi/2)
    cam.location=(math.sin(angle)*distance,-math.cos(angle)*distance,.133-.006*t)
    aim(cam,(0,.004,.0285))
    cam.keyframe_insert('location',frame=f);cam.keyframe_insert('rotation_euler',frame=f)
for action in bpy.data.actions:
    try:
        for fc in action.fcurves:
            for kp in fc.keyframe_points:kp.interpolation='BEZIER'
    except AttributeError:pass

target=BUILD/('render-'+kind);target.mkdir(exist_ok=True)
scene.render.filepath=str(target/'frame_')
scene.frame_set(60 if args.preview else args.start)
bpy.ops.wm.save_as_mainfile(filepath=str(BUILD/('shot-'+kind+'.blend')))
if args.preview:
    scene.render.filepath=str(BUILD/('preview-'+kind+'.png'));bpy.ops.render.render(write_still=True)
else:bpy.ops.render.render(animation=True)
