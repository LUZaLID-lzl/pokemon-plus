"""Prepare and import localized MobiIoT edits of actual scene ground."""
from pathlib import Path
from PIL import Image, ImageDraw
import argparse
import hashlib
import json
from scene_nest_art import build_nest, catalog, ROOT

BASE_SOURCES = [
 'source/courtyard-hud-v13.png', 'source/scenes-hud-v14/01.png',
 'source/scenes-hud-v14/02.png', 'source/waterfall-v24/03-base.png',
 'source/scenes-hud-v14/04.png', 'source/scenes-hud-v14/05.png',
 'source/scenes-v18/06.png', 'source/scenes-hud-v14/07.png',
 'source/scenes-hud-v14/08.png', 'source/scenes-hud-v14/09.png',
 'source/scenes-hud-v14/10.png', 'source/scenes-v18/11.png',
 'source/blue-sacred-tree/09-curved-cherry-reference-original.png']

DESIGNS = [
 'Morning grassy cottage yard. Make a modest tan twig nest tucked into a gently flattened patch of grass near the cottage. A few grass blades overlap the bottom edge, a few loose straw pieces lie nearby, subtle shaded soil underneath. Preserve cottage and path.',
 'Cherry blossom yard. Let dusty pink petals and gray-brown fallen twigs gather into a shallow raised woven nest at the foot of the nearby cherry tree. Small petal scatter links it to the ground, muted moss around the base. Preserve existing cherry tree.',
 'Sunny coast. Create a shallow scooped depression in DRY sand, with naturally banked sand sheltering a low driftwood and dry seagrass nest. Sand partly covers its lower edges, a few tiny shell fragments nearby. Weathered sandy colors, no gold basket. Preserve shoreline.',
 'Forest waterfall. Nest is tucked into a DRY mossy rock ledge at the left side of the waterfall basin, sheltered behind a low natural rock outcrop. Muted root fibers, moss and a few dry leaves connect it to the ledge. Preserve waterfall, pool shoreline, rocks and walkable clearing. Keep nest out of falling water and pool.',
 'Autumn woodland. Interwoven reddish twigs rise from a bed of accumulated fallen leaves at the tree base. A few scattered russet leaves overlap the lower nest wall. The hollow remains distinct from surrounding leaf litter. Preserve tree trunk.',
 'Snowy village. Sculpt a shallow wind-sheltered hollow in the snow around a warm, faded straw nest. Small snow banks partly overlap its base, blue-gray shadow presses into the snow, straw remains dry inside. Preserve cottage and snow path.',
 'Violet crystal cave. A small root-and-straw cradle sits naturally in a shallow cleft of the dry cave floor, bordered by worn purple-gray rock and one tiny crystal. Root fibers tuck under nearby rocks, raised woven front edge, clear empty cavity. Preserve existing crystal clusters.',
 'Mushroom grove. Low twig-and-moss nest sheltered by the nearby mushroom, with irregular moss, grass and two tiny fallen leaves connecting the base to the forest floor. Olive brown, low contrast, visible woven front wall. Preserve large mushrooms.',
 'Starry hill at NIGHT. Cool slate-blue grass and gray-beige fibers form a low nest in a shallow grass hollow. Short bent grass overlaps the base and casts subdued dark blue contact shadows. Very subdued night lighting, no warm yellow spotlight, no glow. Preserve hill and sky.',
 'Desert oasis. A low faded palm-fiber nest in a small sheltered sand hollow beneath the palm. Dusty sage leaf tips and a crescent of naturally piled sand wrap its lower edge. Match warm sandy local light without shiny yellow. Preserve water and palms.',
 'Rainy village. Wet walnut willow twigs and a broad dull green leaf cradle fit into a dry sheltered nook near the house wall. Damp grass, a little scattered straw and soft cool contact shadow connect the base with the ground. Keep hollow dry. Preserve walls and path.',
 'Sky island. A shallow grass-lined cradle grows naturally into the grassy soil beside the old stone ruins. Sage grass stems and a few small pale feather tufts, short grass overlapping base, soft green ground shadow. Preserve island edges and ruins.',
 'Sacred blue tree and its EXISTING STONE ALTAR. The user wants the egg cradle embedded into the LEFT PART OF THE EXISTING ALTAR, not sitting to the right at the tree foot. Carve a shallow oval recess into the altar top at the guide location; line its interior with a small amount of muted gray-beige dry grass. The existing blue-gray altar stones form its raised front rim and sidewall, seamlessly continuing the original stone slab and root edging. Preserve the altar width, stone texture and cyan inlays outside the recess; retain existing roots and moss. NO separate basket, NO new pedestal, NO round prop pasted on top. The altar itself holds the egg cavity. Preserve main tree trunk and surrounding lake.'
]

def original(scene):
    return Image.open(ROOT/'assets'/BASE_SOURCES[scene]).convert('RGB').resize((120,120),Image.Resampling.NEAREST).quantize(colors=48,dither=Image.Dither.NONE).convert('RGB')

def bounds(scene):
    x,y=catalog()['scenes'][scene]['position']
    assert x%3==0 and y%3==0
    return (x//3-8,y//3-8,x//3+32,y//3+24)

def terrain_directory():
    return catalog().get('terrainDirectory','scene-nest-terrain')

def references(scenes=range(13)):
    directory=terrain_directory()
    for scene in scenes:
        ref=ROOT/f'assets/reference/{directory}/{scene:02}.png'
        context=ROOT/f'assets/reference/{directory}/{scene:02}-context.png'
        prompt=ROOT/f'assets/prompts/{directory}/{scene:02}.txt'
        for p in (ref,context,prompt):p.parent.mkdir(parents=True,exist_ok=True)
        im=original(scene);placed=im.convert('RGBA')
        x,y=catalog()['scenes'][scene]['position']
        # The previous prop is only a position/silhouette guide for AI editing.
        placed.alpha_composite(build_nest(scene).resize((24,14),Image.Resampling.NEAREST),(x//3,y//3))
        placed.convert('RGB').crop(bounds(scene)).resize((400,320),Image.Resampling.NEAREST).save(ref)
        im.resize((360,360),Image.Resampling.NEAREST).save(context)
        prompt.write_text('Edit FIRST reference image: it is a close-up of the actual game ground with a temporary nest placement guide. SECOND reference is the full original scene for context and lighting. Output ONLY the edited first close-up, same 5:4 aspect ratio and exactly the same camera, framing and ground landmarks.\n\nReplace the pasted-looking nest AND rework the surrounding ground into one coherent piece of scenery. The nest occupies logical x8..31,y8..21 on a 40x32 pixel logical crop; keep the EMPTY cavity centered near x20,y15. Keep the outermost 4 logical pixels of the crop unchanged so this edited ground can fit back into the original scene. Keep major landmarks in place. No egg, pets, characters, text, labels, HUD or borders. Do not make an inventory icon.\n\n'+DESIGNS[scene]+'\n\nTrue coarse pixel art matching the references, 40x32 logical square pixel grid enlarged with nearest neighbor. Restrained colors sampled from the ground, no photoreal detail, no blur, no antialiasing, no dramatic black sticker outline. Raised rear rim, shadowed empty oval cavity, clearly woven raised front wall, subdued sidewall and natural contact shadows. Nest size about 24x14 logical pixels including its accents; ground changes may extend around it. The lower front wall should lie around y17..21; leave clear empty interior around x16..24,y12..17 for one egg. Seamless physical connection with the local ground, not a separate prop on top of unchanged floor.\n')
    print('Prepared selected actual ground crops, full scene references and environment-specific edit prompts')

def import_scene(scene):
    directory=terrain_directory()
    target=ROOT/f'assets/source/{directory}/{scene:02}-native.png'
    raw=ROOT/f'assets/source/{directory}/{scene:02}-edit.png'
    if not raw.exists():raise FileNotFoundError(raw)
    base=original(scene);box=bounds(scene)
    edited=Image.open(raw).convert('RGB').resize((40,32),Image.Resampling.NEAREST)
    # A compact shared ground palette keeps the new rim in lossless I4
    # alongside all151 pets within the fixed resource partition.
    edited=edited.quantize(colors=15,dither=Image.Dither.NONE).convert('RGB')
    # Reserve the terrain's own shades in the final48-color scene. A global
    # quantizer otherwise spends nearly all colors on the large background
    # and collapses the tiny nest's hollow/front wall into a brown smudge.
    background=base.quantize(colors=32,dither=Image.Dither.NONE).convert('RGB')
    colors=list(dict.fromkeys(list(background.getdata())+list(edited.getdata())))
    assert len(colors)<=47
    palette=Image.new('P',(1,1));flat=[v for rgb in colors for v in rgb]
    palette.putpalette(flat+[0]*(768-len(flat)))
    base=base.quantize(palette=palette,dither=Image.Dither.NONE).convert('RGB')
    # Preserve the unedited map outside this local ground patch. Native-pixel
    # blending is confined to the patch perimeter, never across nest details.
    mask=Image.new('L',(40,32))
    for yy in range(32):
        for xx in range(40):
            edge=min(xx,yy,39-xx,31-yy)
            mask.putpixel((xx,yy),0 if edge<3 else 85 if edge==3 else 170 if edge==4 else 255)
    patch=Image.composite(edited,base.crop(box),mask)
    base.paste(patch,box)
    base=base.quantize(palette=palette,dither=Image.Dither.NONE).convert('RGB')
    target.parent.mkdir(parents=True,exist_ok=True);base.save(target)
    return dict(scene=scene,original=BASE_SOURCES[scene],edit=str(raw.relative_to(ROOT/'assets')),source=str(target.relative_to(ROOT/'assets')),crop=list(box),sha256=hashlib.sha256(target.read_bytes()).hexdigest())

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('mode',choices=['references','import']);p.add_argument('--scene',type=int);args=p.parse_args()
    if args.mode=='references':references([args.scene] if args.scene is not None else range(13))
    else:
        scenes=[args.scene] if args.scene is not None else range(13)
        for scene in scenes:print(json.dumps(import_scene(scene),ensure_ascii=False))
