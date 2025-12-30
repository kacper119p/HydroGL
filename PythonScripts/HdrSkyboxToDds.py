import cv2
import numpy as np
import os
import shutil
import subprocess
import argparse


def normalize(v):
    return v / np.linalg.norm(v, axis=-1, keepdims=True)


def generate_cubemap_faces(hdr_image, face_size):
    H, W, _ = hdr_image.shape
    faces = {}

    directions = {
        "posx": lambda u, v: np.stack([np.ones_like(u), -v, -u], axis=-1),
        "negx": lambda u, v: np.stack([-np.ones_like(u), -v, u], axis=-1),
        "posy": lambda u, v: np.stack([u, np.ones_like(u), v], axis=-1),
        "negy": lambda u, v: np.stack([u, -np.ones_like(u), -v], axis=-1),
        "posz": lambda u, v: np.stack([u, -v, np.ones_like(u)], axis=-1),
        "negz": lambda u, v: np.stack([-u, -v, -np.ones_like(u)], axis=-1),
    }

    uv = np.linspace(-1, 1, face_size)
    u, v = np.meshgrid(uv, uv)

    for name, func in directions.items():
        dir_vec = normalize(func(u, v))
        x, y, z = dir_vec[..., 0], dir_vec[..., 1], dir_vec[..., 2]
        theta = np.arctan2(z, x)
        phi = np.arcsin(y)
        px = ((theta + np.pi) / (2 * np.pi) * W).astype(np.float32)
        py = ((np.pi / 2 - phi) / np.pi * H).astype(np.float32)

        x0 = np.floor(px).astype(int) % W
        x1 = (x0 + 1) % W
        y0 = np.floor(py).astype(int)
        y1 = np.clip(y0 + 1, 0, H - 1)
        dx = px - x0
        dy = py - y0

        top = (1 - dx[..., None]) * hdr_image[y0, x0] + dx[..., None] * hdr_image[y0, x1]
        bottom = (1 - dx[..., None]) * hdr_image[y1, x0] + dx[..., None] * hdr_image[y1, x1]
        face = (1 - dy[..., None]) * top + dy[..., None] * bottom

        faces[name] = face

    return faces


def main():
    parser = argparse.ArgumentParser(description='Convert HDR image to DDS cubemap')
    parser.add_argument('input_hdr', help='Path to input HDR image')
    parser.add_argument('output_dds', help='Path to output DDS cubemap')
    parser.add_argument('-s', '--face-size', type=int, default=1024,
                        help='Size of each cubemap face in pixels (default: 1024)')
    parser.add_argument('-t', '--temp-dir', default='cube_faces',
                        help='Temporary directory for intermediate files (default: cube_faces)')
    parser.add_argument('-c', '--texconv', default='texconv.exe',
                        help='Path to texconv.exe tool (default: texconv.exe)')
    parser.add_argument('-a', '--texassemble', default='texassemble.exe',
                        help='Path to texassemble.exe tool (default: texassemble.exe)')
    parser.add_argument('-r', '--cleanup', action='store_true', default=True,
                        help='Clean up temporary files after conversion (default: True)')
    parser.add_argument('-f', '--dds-format', default='BC6H_UF16',
                        help='DDS format for the final cubemap (default: BC6H_UF16)')
    args = parser.parse_args()

    hdr_path = args.input_hdr
    cubemap_output = args.output_dds
    face_size = args.face_size
    output_dir = args.temp_dir
    texconv_path = args.texconv
    texassemble_path = args.texassemble
    dds_format = args.dds_format

    if os.path.exists(output_dir):
        raise RuntimeError(f'{output_dir} already exists.')

    os.makedirs(output_dir, exist_ok=True)

    print(f'Creating temporary folder at {output_dir}')

    hdr_img = cv2.imread(hdr_path, cv2.IMREAD_ANYDEPTH | cv2.IMREAD_COLOR)
    if hdr_img is None:
        if args.cleanup and os.path.exists(output_dir):
            shutil.rmtree(output_dir)

        print(f"Removing temporary folder at {output_dir}")
        raise RuntimeError(f"Failed to load HDR image: {hdr_path}")

    faces = generate_cubemap_faces(hdr_img, face_size)
    dds_faces = []

    for name, face in faces.items():
        hdr_face_path = os.path.join(output_dir, f"{name}.hdr")
        cv2.imwrite(hdr_face_path, face.astype(np.float32))

        dds_face_path = os.path.join(output_dir, f"{name}.dds")
        subprocess.run([
            texconv_path,
            "-f", "R32G32B32A32_FLOAT",
            "-m", "0",
            "-y",
            "-o", output_dir,
            hdr_face_path
        ], check=True)

        dds_faces.append(dds_face_path)

    assemble_cmd = [
        texassemble_path,
        "cube",
        *dds_faces,
        "-stripmips",
        "-y",
        "-o",
        cubemap_output
    ]

    subprocess.run(assemble_cmd, check=True)

    output_dir_path = os.path.dirname(cubemap_output)
    if not output_dir_path:
        output_dir_path = "."

    subprocess.run([
        texconv_path,
        "-f", dds_format,
        "-m", "0",
        "-y",
        "-o", output_dir_path,
        cubemap_output
    ], check=True)

    print(f"Created cubemap was written to: {cubemap_output}")

    if args.cleanup and os.path.exists(output_dir):
        shutil.rmtree(output_dir)

    print(f"Removing temporary folder at {output_dir}")


if __name__ == "__main__":
    main()
