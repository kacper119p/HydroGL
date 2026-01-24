import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.path import Path
import numpy as np
import os

# --- KONFIGURACJA PLIKÓW (TWOJE NAZWY) ---
# Klucze odpowiadają strukturze, którą podałeś.
# Wartości to ścieżki do plików.
FILES = {
    # Stage 1
    "gauss": "textures/gauss.png",

    # Stage 2 (Stack)
    "h_0_1": "textures/h_0_1.png",  # Top
    "h_0_2": "textures/h_0_2.png",  # Mid
    "h_0_3": "textures/h_0_3.png",  # Bottom

    # Stage 3 - Upper Branch (Stack)
    "h_1_1": "textures/h_1_1.png",  # Top
    "h_1_2": "textures/h_1_2.png",
    "h_1_3": "textures/h_1_3.png",

    # Stage 3 - Lower Branch (Stack)
    "h_2_1": "textures/h_2_1.png",  # Top
    "h_2_2": "textures/h_2_2.png",
    "h_2_3": "textures/h_2_3.png",

    # Stage 4 - Upper Branch (Stack)
    "result_1_1": "textures/result_1_1.png",  # Top
    "result_1_2": "textures/result_1_2.png",
    "result_1_3": "textures/result_1_3.png",

    # Stage 4 - Lower Branch (Stack)
    "result_2_1": "textures/result_2_1.png",  # Top
    "result_2_2": "textures/result_2_2.png",
    "result_2_3": "textures/result_2_3.png",
}


def load_texture(path, seed=0, pattern_type='noise', size=64):
    """
    Loads texture or generates unique dummy data based on seed/type if file missing.
    """
    if os.path.exists(path):
        return plt.imread(path)

    # Generate deterministic dummy data based on name/seed
    np.random.seed(seed)
    x = np.linspace(0, 4 * np.pi, size)
    y = np.linspace(0, 4 * np.pi, size)
    X, Y = np.meshgrid(x, y)

    if pattern_type == 'plasma':
        return np.sin(X + seed) + np.cos(Y + seed)
    elif pattern_type == 'lines':
        return np.sin(X * 2 + Y + seed)
    elif pattern_type == 'grid':
        return (np.sin(X * 4 + seed) * np.cos(Y * 4))
    elif pattern_type == 'noise':
        return np.random.normal(0, 1, (size, size))
    else:
        return np.random.rand(size, size)


def draw_stacked_texture_content(ax, textures_list, cmap, is_stacked=True):
    """
    Renders list of textures: [Top, Middle, Bottom]
    """
    ax.set_box_aspect(1)

    # Offsets: Bottom layer is drawn first (deepest offset)
    # But input list is [Top, Mid, Bottom]
    # So we need to reverse logical mapping.

    if is_stacked:
        offsets = [(0.08, -0.08), (0.04, -0.04), (0, 0)]  # [Bottom_Pos, Mid_Pos, Top_Pos]
    else:
        offsets = [(0, 0)]

    # We iterate offsets from Bottom to Top (Painter's algo)
    for i, (ox, oy) in enumerate(offsets):
        # Calculate which texture index corresponds to this layer.
        # textures_list[0] is Top -> should go to last offset (0,0)
        # textures_list[2] is Bottom -> should go to first offset (0.08, -0.08)

        # If we have 3 offsets:
        # i=0 (Bottom pos) -> want index 2
        # i=1 (Mid pos)    -> want index 1
        # i=2 (Top pos)    -> want index 0

        target_idx = (len(offsets) - 1) - i

        # Safety check if list is shorter than stack depth
        if target_idx < len(textures_list):
            data = textures_list[target_idx]
        else:
            data = textures_list[-1]

        inset = ax.inset_axes([ox, oy, 0.85, 0.85])
        inset.imshow(data, cmap=cmap, alpha=1.0)

        for spine in inset.spines.values():
            spine.set_visible(True)
            spine.set_edgecolor('black')
            spine.set_linewidth(2.0)

        inset.set_xticks([])
        inset.set_yticks([])

    ax.axis('off')


def add_labels_relative(ax, title, bottom_label=None):
    ax.text(0.5, 1.05, title, transform=ax.transAxes,
            ha='center', va='bottom', fontsize=14, fontweight='bold', color='black')
    if bottom_label:
        ax.text(0.5, -0.12, bottom_label, transform=ax.transAxes,
                ha='center', va='top', fontsize=12, fontweight='medium', color='#222222')


def add_straight_arrow(fig, ax_start, ax_end, label=None):
    arrow = patches.ConnectionPatch(
        xyA=(1.0, 0.5), xyB=(0.0, 0.5),
        coordsA="axes fraction", coordsB="axes fraction",
        axesA=ax_start, axesB=ax_end,
        arrowstyle="-|>", color="black", lw=2.5, mutation_scale=25, zorder=1
    )
    fig.add_artist(arrow)
    if label:
        ax_start.text(1.3, 0.55, label, transform=ax_start.transAxes,
                      ha='center', va='bottom', fontsize=12, fontweight='bold',
                      bbox=dict(facecolor='white', edgecolor='none', alpha=0.9, pad=3), zorder=2)


def add_stepped_arrow(fig, ax_start, ax_end):
    transA = ax_start.transAxes
    transB = ax_end.transAxes
    start = fig.transFigure.inverted().transform(transA.transform((1.0, 0.5)))
    end = fig.transFigure.inverted().transform(transB.transform((0.0, 0.5)))
    mid_x = (start[0] + end[0]) / 2

    verts = [(start[0], start[1]), (mid_x, start[1]), (mid_x, end[1]), (end[0], end[1])]
    codes = [Path.MOVETO, Path.LINETO, Path.LINETO, Path.LINETO]
    fig.add_artist(patches.FancyArrowPatch(
        path=Path(verts, codes), arrowstyle="-|>", color="black", lw=2.5, mutation_scale=25, zorder=1
    ))


def visualize_fft_pipeline_names():
    fig = plt.figure(figsize=(18, 10))
    plt.subplots_adjust(left=0.05, right=0.95, top=0.9, bottom=0.15)
    gs = fig.add_gridspec(2, 4, hspace=0.4, wspace=0.6)

    # --- LOADING DATA (With distinct fallback patterns) ---

    # 1. Gauss
    t_gauss = [load_texture(FILES["gauss"], 1, 'noise')]

    # 2. h_0 (Stack)
    t_h0 = [
        load_texture(FILES["h_0_1"], 10, 'plasma'),  # Top
        load_texture(FILES["h_0_2"], 11, 'lines'),  # Mid
        load_texture(FILES["h_0_3"], 12, 'grid')  # Bottom
    ]

    # 3. h_1 (Upper Stack)
    t_h1 = [
        load_texture(FILES["h_1_1"], 20, 'plasma'),
        load_texture(FILES["h_1_2"], 21, 'lines'),
        load_texture(FILES["h_1_3"], 22, 'grid')
    ]

    # 3. h_2 (Lower Stack)
    t_h2 = [
        load_texture(FILES["h_2_1"], 30, 'plasma'),
        load_texture(FILES["h_2_2"], 31, 'lines'),
        load_texture(FILES["h_2_3"], 32, 'grid')
    ]

    # 4. result_1 (Upper Stack)
    t_res1 = [
        load_texture(FILES["result_1_1"], 40, 'plasma'),
        load_texture(FILES["result_1_2"], 41, 'lines'),
        load_texture(FILES["result_1_3"], 42, 'grid')
    ]

    # 4. result_2 (Lower Stack)
    t_res2 = [
        load_texture(FILES["result_2_1"], 50, 'plasma'),
        load_texture(FILES["result_2_2"], 51, 'lines'),
        load_texture(FILES["result_2_3"], 52, 'grid')
    ]

    # --- DRAWING ---

    # Stage 1: Gauss
    ax1 = fig.add_subplot(gs[:, 0])
    draw_stacked_texture_content(ax1, t_gauss, 'gray', is_stacked=False)
    add_labels_relative(ax1, "Stage 1", "gauss")

    # Stage 2: h_0
    ax2 = fig.add_subplot(gs[:, 1])
    draw_stacked_texture_content(ax2, t_h0, 'hot', is_stacked=True)
    add_labels_relative(ax2, "Stage 2", "h_0_{1..3}")

    # Stage 3: Upper (h_1)
    ax3_1 = fig.add_subplot(gs[0, 2])
    draw_stacked_texture_content(ax3_1, t_h1, 'viridis', is_stacked=True)
    add_labels_relative(ax3_1, "Stage 3 (Upper)", "h_1_{1..3}")

    # Stage 3: Lower (h_2)
    ax3_2 = fig.add_subplot(gs[1, 2])
    draw_stacked_texture_content(ax3_2, t_h2, 'viridis', is_stacked=True)
    add_labels_relative(ax3_2, "", "h_2_{1..3}")

    # Stage 4: Upper (result_1)
    ax4_1 = fig.add_subplot(gs[0, 3])
    draw_stacked_texture_content(ax4_1, t_res1, 'Blues_r', is_stacked=True)
    add_labels_relative(ax4_1, "Stage 4 (Upper)", "result_1_{1..3}")

    # Stage 4: Lower (result_2)
    ax4_2 = fig.add_subplot(gs[1, 3])
    draw_stacked_texture_content(ax4_2, t_res2, 'Blues_r', is_stacked=True)
    add_labels_relative(ax4_2, "", "result_2_{1..3}")

    # --- CONNECTIONS ---
    add_straight_arrow(fig, ax1, ax2)
    add_stepped_arrow(fig, ax2, ax3_1)
    add_stepped_arrow(fig, ax2, ax3_2)
    add_straight_arrow(fig, ax3_1, ax4_1, label="IFFT")
    add_straight_arrow(fig, ax3_2, ax4_2, label="IFFT")

    plt.suptitle("FFT Ocean Pipeline: Named Textures Flow", fontsize=24, fontweight='bold', y=0.98)
    plt.savefig("fft_pipeline_named.png")
    plt.show()


if __name__ == "__main__":
    visualize_fft_pipeline_names()