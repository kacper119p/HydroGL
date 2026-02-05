import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.path import Path
import numpy as np
import matplotlib.colors as mcolors


def draw_gradient_box(ax, xy, width, height, color_top, color_bot, label, sub_label):
    r1, g1, b1 = mcolors.to_rgb(color_top)
    r2, g2, b2 = mcolors.to_rgb(color_bot)

    cdict = {
        'red': [(0.0, r1, r1), (1.0, r2, r2)],
        'green': [(0.0, g1, g1), (1.0, g2, g2)],
        'blue': [(0.0, b1, b1), (1.0, b2, b2)]
    }
    cmap = mcolors.LinearSegmentedColormap('custom_grad', cdict)

    gradient = np.linspace(0, 1, 256).reshape(-1, 1)

    ax.imshow(gradient, aspect='auto', extent=(xy[0], xy[0] + width, xy[1], xy[1] + height),
              cmap=cmap, zorder=1, origin='upper')

    rect = patches.Rectangle(xy, width, height, linewidth=2, edgecolor='black', facecolor='none', zorder=2)
    ax.add_patch(rect)

    cx = xy[0] + width / 2
    cy = xy[1] + height / 2

    fs_main = 24

    ax.text(cx, cy + height * 0.12, label, fontsize=fs_main, ha='center', va='center', fontweight='bold', zorder=3)

    if sub_label:
        ax.text(cx, cy - height * 0.22, sub_label, fontsize=16, ha='center', va='top', style='italic', zorder=3)

    return (xy[0] + width, cy), (xy[0], cy)


def draw_stack(ax, xy, width, height, labels, colors, sub_labels):
    anchors = []
    current_y = xy[1] + height * 3

    for i in range(4):
        rect = patches.Rectangle((xy[0], current_y), width, height,
                                 linewidth=2, edgecolor='black', facecolor=colors[i], zorder=1)
        ax.add_patch(rect)

        cx = xy[0] + width / 2
        cy = current_y + height / 2

        fs_main = 24

        ax.text(cx, cy + height * 0.15, labels[i], fontsize=fs_main, ha='center', va='center', fontweight='bold',
                zorder=3)
        if sub_labels:
            ax.text(cx, cy - height * 0.25, sub_labels[i], fontsize=16, ha='center', va='top', style='italic',
                    zorder=3)

        anchors.append(((xy[0] + width, cy), (xy[0], cy)))
        current_y -= height

    return anchors


def draw_manual_z_arrow(ax, start, end, color, fraction):
    """
    Draws a Z-shaped orthogonal arrow from start to end.
    """
    x1, y1 = start
    x2, y2 = end

    # Calculate horizontal turn point
    x_turn = x1 + (x2 - x1) * fraction

    verts = [
        (x1, y1),
        (x_turn, y1),
        (x_turn, y2),
        (x2, y2),
    ]
    codes = [Path.MOVETO, Path.LINETO, Path.LINETO, Path.LINETO]
    path = Path(verts, codes)

    # Draw Background (White Outline for separation)
    patch_bg = patches.FancyArrowPatch(path=path, arrowstyle='-|>', color='black',
                                       linewidth=6, zorder=4, mutation_scale=20)
    ax.add_patch(patch_bg)

    # Draw Foreground (Color)
    patch_fg = patches.FancyArrowPatch(path=path, arrowstyle='-|>', color=color,
                                       linewidth=4, zorder=5, mutation_scale=20)
    ax.add_patch(patch_fg)


def draw_iff_connections(ax, anchors_in, anchors_mid):
    # IFFT Connections (Left -> Right)
    for i in range(4):
        in_idx = 0 if i < 2 else 1

        start = anchors_in[in_idx][0]  # Right of In
        end = anchors_mid[i][1]  # Left of Mid

        draw_manual_z_arrow(ax, start, end, "#555", 0.5)


def draw_cross_swizzle_connections(ax, all_anchors_mid, all_anchors_out, mid_cols, swizzle_config, graph_index):
    current_out_anchors = all_anchors_out[graph_index]

    for dest_idx, (src_graph_idx, src_channel_idx) in enumerate(swizzle_config):
        # Start: Right side of Mid (Source)
        start_anchor = all_anchors_mid[src_graph_idx][src_channel_idx][0]
        # End: Left side of Out (Dest)
        end_anchor = current_out_anchors[dest_idx][1]

        # --- Apply Y-Scatter ---
        # We increase the scatter factor significantly to make it visible
        # Previous was 0.4, increasing to 1.2

        # The height of a single box is 'h' (5.5).
        # A shift of 1.2 is about 20% of the box height, which should be clearly visible.

        # We define an "offset index" relative to the center (0,1,2,3 -> -1.5, -0.5, 0.5, 1.5)
        # We reverse it or mix it up so lines don't look parallel

        offset_start = (src_channel_idx - 1.5) * 1.5
        offset_end = (dest_idx - 0.75) * 0.75

        start = (start_anchor[0], start_anchor[1] + offset_start)
        end = (end_anchor[0], end_anchor[1] + offset_end)

        col = mid_cols[src_channel_idx]

        # Calculate fraction to spread lines horizontally
        frac = 0.2 + (dest_idx * 0.15)

        draw_manual_z_arrow(ax, start, end, col, frac)


def generate_final_pipeline(swizzle_top, swizzle_bot):
    fig, ax = plt.subplots(figsize=(14, 12))
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 75)
    ax.axis('off')

    w = 16
    h = 5.5
    x_in, x_mid, x_out = 2, 38, 76

    y_top_base = 40
    y_bot_base = 10

    c_red = '#ff6666'
    c_green = '#66ff66'
    c_blue = '#75d9ff'
    c_gray = '#b0b0b0'

    mid_cols = [c_red, c_green, c_blue, c_gray]
    out_cols = [c_red, c_green, c_blue, c_gray]

    labels_top_mid = [r"$h$", r"$D_z$", r"$D_x$", r"$\frac{\partial h}{\partial x}$"]
    labels_bot_mid = [r"$\frac{\partial h}{\partial z}$", r"$\frac{\partial D_x}{\partial x}$",
                      r"$\frac{\partial D_z}{\partial x}$", r"$\frac{\partial D_z}{\partial z}$"]

    all_mid_labels = [labels_top_mid, labels_bot_mid]

    # ==========================
    # 1. TOP ROW DRAWING
    # ==========================
    anchors_in_top = []
    anchors_in_top.append(draw_gradient_box(ax, (x_in, y_top_base + h * 2), w, h * 2, c_red, c_green,
                                            r"$h + i \cdot D_z$", "R + G"))
    anchors_in_top.append(draw_gradient_box(ax, (x_in, y_top_base), w, h * 2, c_blue, c_gray,
                                            r"$D_x + i \cdot \frac{\partial h}{\partial x}$", "B + A"))

    anchors_mid_top = draw_stack(ax, (x_mid, y_top_base), w, h, labels_top_mid, mid_cols,
                                 ["R", "G", "B", "A"])

    labels_out_top = []
    for src_graph, src_chan in swizzle_top:
        labels_out_top.append(all_mid_labels[src_graph][src_chan])

    anchors_out_top = draw_stack(ax, (x_out, y_top_base), w, h, labels_out_top, out_cols,
                                 ["R", "G", "B", "A"])

    draw_iff_connections(ax, anchors_in_top, anchors_mid_top)

    # ==========================
    # 2. BOTTOM ROW DRAWING
    # ==========================
    anchors_in_bot = []
    anchors_in_bot.append(draw_gradient_box(ax, (x_in, y_bot_base + h * 2), w, h * 2, c_red, c_green,
                                            r"$\frac{\partial h}{\partial z} + i \cdot \frac{\partial D_x}{\partial x}$",
                                            "R + G"))
    anchors_in_bot.append(draw_gradient_box(ax, (x_in, y_bot_base), w, h * 2, c_blue, c_gray,
                                            r"$\frac{\partial D_z}{\partial x} + i \cdot \frac{\partial D_z}{\partial z}$",
                                            "B + A"))

    anchors_mid_bot = draw_stack(ax, (x_mid, y_bot_base), w, h, labels_bot_mid, mid_cols,
                                 ["R", "G", "B", "A"])

    labels_out_bot = []
    for src_graph, src_chan in swizzle_bot:
        labels_out_bot.append(all_mid_labels[src_graph][src_chan])

    anchors_out_bot = draw_stack(ax, (x_out, y_bot_base), w, h, labels_out_bot, out_cols,
                                 ["R", "G", "B", "A"])

    draw_iff_connections(ax, anchors_in_bot, anchors_mid_bot)

    # ==========================
    # 3. CROSS-CONNECTIONS
    # ==========================
    all_anchors_mid = [anchors_mid_top, anchors_mid_bot]
    all_anchors_out = [anchors_out_top, anchors_out_bot]

    draw_cross_swizzle_connections(ax, all_anchors_mid, all_anchors_out, mid_cols, swizzle_top, 0)
    draw_cross_swizzle_connections(ax, all_anchors_mid, all_anchors_out, mid_cols, swizzle_bot, 1)

    # ==========================
    # 4. LABELS (ON TOP)
    # ==========================
    header_y = 68
    ax.text(x_in + w / 2, header_y, "1. Spectrum\n(Frequency)", ha='center', va='bottom', fontsize=24, fontweight='bold')
    ax.text(x_mid + w / 2, header_y, "2. Spectrum\n(Spatial)", ha='center', va='bottom', fontsize=24, fontweight='bold')
    ax.text(x_out + w / 2, header_y, "3. Final\n(Swizzled)", ha='center', va='bottom', fontsize=24, fontweight='bold')

    op_label_y = header_y
    ax.text((x_in + w + x_mid) / 2, op_label_y, "IFFT", ha='center', va='bottom', fontsize=28, fontweight='bold',
            bbox=dict(facecolor='#b0b0b0', edgecolor='#ccc', boxstyle='round,pad=0.5'))
    ax.text((x_mid + w + x_out) / 2, op_label_y, "Assemble", ha='center', va='bottom', fontsize=28, fontweight='bold',
            bbox=dict(facecolor='#b0b0b0', edgecolor='#ccc', boxstyle='round,pad=0.5'))

    plt.tight_layout()
    plt.show()


# Config
SWIZZLE_TOP = [(0, 2), (0, 0), (0, 1), (1, 2 )]
SWIZZLE_BOT = [(0, 3), (1, 0), (1, 1), (1, 3)]

if __name__ == "__main__":
    generate_final_pipeline(SWIZZLE_TOP, SWIZZLE_BOT)