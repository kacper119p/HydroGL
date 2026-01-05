import svgwrite

CANVAS_SIZE = 1024

CAMERA_POS = (270, 300)

FILE_NAME = "CdlodVisualization.svg"

LOD_RANGES = [
    150,
    300,
    600,
    1200,
]

LOD_COLORS = {
    0: "#ff5252",
    1: "#faf25a",
    2: "#b2fc68",
    3: "#000000",
    4: "#000000",
}

dwg = svgwrite.Drawing(FILE_NAME, size=(CANVAS_SIZE, CANVAS_SIZE))

def intersect_circle(x, y, size, radius):
    closest_x = max(x, min(CAMERA_POS[0], x + size))
    closest_y = max(y, min(CAMERA_POS[1], y + size))

    dist_x = closest_x - CAMERA_POS[0]
    dist_y = closest_y - CAMERA_POS[1]

    return (dist_x ** 2 + dist_y ** 2) < (radius ** 2)


def draw_recursive(x, y, size, level):
    should_split = False
    max_depth = len(LOD_RANGES)

    if level < max_depth:
        target_lod_idx = max_depth - 1 - level
        radius = LOD_RANGES[target_lod_idx]

        if intersect_circle(x, y, size, radius):
            should_split = True

    if should_split:
        half = size / 2
        draw_recursive(x, y, half, level + 1)
        draw_recursive(x + half, y, half, level + 1)
        draw_recursive(x, y + half, half, level + 1)
        draw_recursive(x + half, y + half, half, level + 1)
    else:
        lod_index = max(0, len(LOD_RANGES) - level)
        color = LOD_COLORS.get(lod_index, "gray")

        dwg.add(dwg.rect(insert=(x, y), size=(size, size),
                         fill=color, fill_opacity=0.6,
                         stroke='black', stroke_width=1))


dwg.add(dwg.rect(insert=(0, 0), size=(CANVAS_SIZE, CANVAS_SIZE), fill="white"))

draw_recursive(0, 0, CANVAS_SIZE, 0)

dwg.add(dwg.circle(center=CAMERA_POS, r=8, fill="black"))

for i in reversed(range(len(LOD_RANGES))):
    r = LOD_RANGES[i]

    dwg.add(dwg.circle(center=CAMERA_POS, r=r,
                       fill='none', stroke='black',
                       stroke_width=3, stroke_dasharray="10,10"))

    dwg.add(dwg.text(f"LOD {i}", insert=(CAMERA_POS[0], CAMERA_POS[1] + r + 25),
                     text_anchor="middle",
                     font_size=32, font_weight="bold", font_family="Arial"))

dwg.save()
print(f"Saved \"{FILE_NAME}\".")
