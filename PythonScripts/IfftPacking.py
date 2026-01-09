import graphviz

# Initialize Digraph
# rankdir='TB' means Top-to-Bottom flow
# splines='ortho' makes the connecting lines orthogonal (right angles), which looks cleaner for technical diagrams
dot = graphviz.Digraph('IFFT_Packing_Architecture', comment='8-Signal IFFT Packing Architecture')
dot.attr(rankdir='TB', splines='ortho', nodesep='0.6', ranksep='0.5', compound='true', fontname="Helvetica")

# Define standard node styles
dot.attr('node', shape='box', style='filled', fillcolor='white', fontname='Helvetica', fontsize='10')
dot.attr('edge', fontname='Helvetica', fontsize='9')

# --- Define Color Schemes ---
colors = {
    'freq_bg': '#E1ECF4', 'freq_border': '#7A92a3',
    'pack_bg': '#FFF3CD', 'pack_border': '#D4A017',
    'spatial_bg': '#D4EDDA', 'spatial_border': '#28A745',
    'tex_bg': '#f0f0f0', 'tex_border': '#666666',
    'ifft_op': '#007bff', 'ifft_text': 'white',
    # Channel colors (Red, Green, Blue, Alpha-Yellow)
    'ch_r': '#ffcccb', 'ch_g': '#ccffcc', 'ch_b': '#ccccff', 'ch_a': '#ffffcc'
}

# =========================================
# 1. Frequency Domain Inputs Cluster
# =========================================
with dot.subgraph(name='cluster_0_inputs') as c:
    c.attr(label='FREQUENCY DOMAIN INPUTS (vec2 amplitudes)', style='filled', fillcolor=colors['freq_bg'],
           color=colors['freq_border'])

    input_sigs = [
        ('Dx', 'Disp X\n(Dx)'), ('Dz', 'Disp Z\n(Dz)'),
        ('Dy', 'Disp Y\n(Dy)'), ('J', 'Jacobian\n(J)'),
        ('Sx', 'Slope X\n(Sx)'), ('Sz', 'Slope Z\n(Sz)'),
        ('Vx', 'Vel X\n(Vx)'), ('Vz', 'Vel Z\n(Vz)')
    ]
    for sig_id, label in input_sigs:
        c.node(sig_id, label, shape='ellipse')

# =========================================
# 2. Packing Logic (Invisible grouping for layout)
# =========================================
# We use "component" shapes to represent the abstract packing step
pack_nodes = ['Pack1', 'Pack2', 'Pack3', 'Pack4']
pack_labels = ['Pack Z = Dx + iDz', 'Pack Z = Dy + iJ', 'Pack Z = Sx + iSz', 'Pack Z = Vx + iVz']

for p_id, p_label in zip(pack_nodes, pack_labels):
    dot.node(p_id, p_label, shape='component', style='filled', fillcolor=colors['pack_bg'], color=colors['pack_border'],
             fontsize='9')

# Connect inputs to packing nodes
dot.edge('Dx', 'Pack1');
dot.edge('Dz', 'Pack1')
dot.edge('Dy', 'Pack2');
dot.edge('J', 'Pack2')
dot.edge('Sx', 'Pack3');
dot.edge('Sz', 'Pack3')
dot.edge('Vx', 'Pack4');
dot.edge('Vz', 'Pack4')


# =========================================
# 3. Texture Resources Clusters (The Core)
# =========================================

# Helper function to create HTML table labels for texture channels
def make_tex_label(title, subtitle, r_txt, g_txt, b_txt, a_txt, bg_color, ports_suffix=""):
    # ports_suffix helps distinguish input ports from output ports (e.g., "r_in" vs "r_out")
    return f'''<<TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" BGCOLOR="{bg_color}">
    <TR><TD COLSPAN="4"><B>{title}</B><BR/>{subtitle}</TD></TR>
    <TR>
        <TD BGCOLOR="{colors['ch_r']}" PORT="r{ports_suffix}" WIDTH="70">R:<BR/>{r_txt}</TD>
        <TD BGCOLOR="{colors['ch_g']}" PORT="g{ports_suffix}" WIDTH="70">G:<BR/>{g_txt}</TD>
        <TD BGCOLOR="{colors['ch_b']}" PORT="b{ports_suffix}" WIDTH="70">B:<BR/>{b_txt}</TD>
        <TD BGCOLOR="{colors['ch_a']}" PORT="a{ports_suffix}" WIDTH="70">A:<BR/>{a_txt}</TD>
    </TR>
    </TABLE>>'''


# --- Texture 0 Cluster ---
with dot.subgraph(name='cluster_tex0') as c:
    c.attr(label='TEXTURE 0 (RGBA Resource)', style='dashed', color=colors['tex_border'], bgcolor=colors['tex_bg'])

    # Node representing the texture state BEFORE IFFT
    t0_in_lbl = make_tex_label("IFFT INPUT STATE", "(Packed Frequency Data)",
                               "Dx_pack<BR/><FONT POINT-SIZE='8'>(Dx.r - Dz.i)</FONT>",
                               "Dz_pack<BR/><FONT POINT-SIZE='8'>(Dx.i + Dz.r)</FONT>",
                               "Dy_pack<BR/><FONT POINT-SIZE='8'>(Dy.r - J.i)</FONT>",
                               "J_pack<BR/><FONT POINT-SIZE='8'>(Dy.i + J.r)</FONT>",
                               'white', "_in")
    c.node('T0_IN', t0_in_lbl, shape='plaintext', style='')

    # The IFFT Operation
    c.node('IFFT0', 'COMPUTE SHADER\nIFFT Operation', shape='oval', style='filled', fillcolor=colors['ifft_op'],
           fontcolor=colors['ifft_text'])

    # Node representing the texture state AFTER IFFT
    t0_out_lbl = make_tex_label("SPATIAL OUTPUT STATE", "(Unpacked Spatial Data)",
                                "Spatial Dx", "Spatial Dz", "Spatial Dy", "Spatial J",
                                colors['spatial_bg'], "_out")
    c.node('T0_OUT', t0_out_lbl, shape='plaintext', style='')

    # Flow within the texture resource
    c.edge('T0_IN', 'IFFT0', style='bold')
    c.edge('IFFT0', 'T0_OUT', style='bold')

# --- Texture 1 Cluster ---
with dot.subgraph(name='cluster_tex1') as c:
    c.attr(label='TEXTURE 1 (RGBA Resource)', style='dashed', color=colors['tex_border'], bgcolor=colors['tex_bg'])

    t1_in_lbl = make_tex_label("IFFT INPUT STATE", "(Packed Frequency Data)",
                               "Sx_pack<BR/><FONT POINT-SIZE='8'>(Sx.r - Sz.i)</FONT>",
                               "Sz_pack<BR/><FONT POINT-SIZE='8'>(Sx.i + Sz.r)</FONT>",
                               "Vx_pack<BR/><FONT POINT-SIZE='8'>(Vx.r - Vz.i)</FONT>",
                               "Vz_pack<BR/><FONT POINT-SIZE='8'>(Vx.i + Vz.r)</FONT>",
                               'white', "_in")
    c.node('T1_IN', t1_in_lbl, shape='plaintext', style='')

    c.node('IFFT1', 'COMPUTE SHADER\nIFFT Operation', shape='oval', style='filled', fillcolor=colors['ifft_op'],
           fontcolor=colors['ifft_text'])

    t1_out_lbl = make_tex_label("SPATIAL OUTPUT STATE", "(Unpacked Spatial Data)",
                                "Spatial Sx", "Spatial Sz", "Spatial Vx", "Spatial Vz",
                                colors['spatial_bg'], "_out")
    c.node('T1_OUT', t1_out_lbl, shape='plaintext', style='')

    c.edge('T1_IN', 'IFFT1', style='bold')
    c.edge('IFFT1', 'T1_OUT', style='bold')

# =========================================
# 4. Connecting Packers to Texture Inputs
# =========================================
# We connect to specific PORTS (r_in, g_in, etc.) defined in the HTML labels above
dot.edge('Pack1', 'T0_IN:r_in:n', label='Real part to R')
dot.edge('Pack1', 'T0_IN:g_in:n', label='Imag part to G')
dot.edge('Pack2', 'T0_IN:b_in:n', label='Real part to B')
dot.edge('Pack2', 'T0_IN:a_in:n', label='Imag part to A')

dot.edge('Pack3', 'T1_IN:r_in:n')
dot.edge('Pack3', 'T1_IN:g_in:n')
dot.edge('Pack4', 'T1_IN:b_in:n')
dot.edge('Pack4', 'T1_IN:a_in:n')

# =========================================
# 5. Final Spatial Outputs Cluster
# =========================================
with dot.subgraph(name='cluster_final_outputs') as c:
    c.attr(label='FINAL SPATIAL OUTPUTS (single float)', style='filled', fillcolor=colors['spatial_bg'],
           color=colors['spatial_border'])

    out_sigs = [
        ('Dx_out', 'Spatial Dx'), ('Dz_out', 'Spatial Dz'),
        ('Dy_out', 'Spatial Dy'), ('J_out', 'Spatial J'),
        ('Sx_out', 'Spatial Sx'), ('Sz_out', 'Spatial Sz'),
        ('Vx_out', 'Spatial Vx'), ('Vz_out', 'Spatial Vz')
    ]
    for sig_id, label in out_sigs:
        c.node(sig_id, label, shape='ellipse', style='filled', fillcolor='white')

# =========================================
# 6. Connecting Texture Outputs to Final Outputs
# =========================================
# Connecting from specific PORTS (r_out, g_out...) to the final nodes
dot.edge('T0_OUT:r_out:s', 'Dx_out', color=colors['ch_r'])
dot.edge('T0_OUT:g_out:s', 'Dz_out', color=colors['ch_g'])
dot.edge('T0_OUT:b_out:s', 'Dy_out', color=colors['ch_b'])
dot.edge('T0_OUT:a_out:s', 'J_out', color=colors['ch_a'])

dot.edge('T1_OUT:r_out:s', 'Sx_out', color=colors['ch_r'])
dot.edge('T1_OUT:g_out:s', 'Sz_out', color=colors['ch_g'])
dot.edge('T1_OUT:b_out:s', 'Vx_out', color=colors['ch_b'])
dot.edge('T1_OUT:a_out:s', 'Vz_out', color=colors['ch_a'])

# Render the graph
# view=True will open the generated PNG automatically
filename = 'ifft_packing_architecture'
dot.render(filename, view=True, format='png')
print(f"Diagram generated: {filename}.png")