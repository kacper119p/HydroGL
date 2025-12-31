import argparse
import sys
from PIL import Image


def pack_channels(r_path, g_path, b_path, a_path=None, output_path="packed.png"):
    try:
        print(f"Loading Red channel: {r_path}")
        r_img = Image.open(r_path).convert('L')

        print(f"Loading Green channel: {g_path}")
        g_img = Image.open(g_path).convert('L')

        print(f"Loading Blue channel: {b_path}")
        b_img = Image.open(b_path).convert('L')

        if r_img.size != g_img.size or r_img.size != b_img.size:
            raise ValueError(f"Dimension mismatch in RGB channels. R:{r_img.size}, G:{g_img.size}, B:{b_img.size}")

        if a_path:
            print(f"Loading Alpha channel: {a_path}")
            a_img = Image.open(a_path).convert('L')

            if r_img.size != a_img.size:
                raise ValueError(f"Alpha texture dimensions {a_img.size} do not match RGB dimensions {r_img.size}.")

            packed = Image.merge('RGBA', (r_img, g_img, b_img, a_img))
            mode = "RGBA"
        else:
            packed = Image.merge('RGB', (r_img, g_img, b_img))
            mode = "RGB"

        packed.save(output_path)
        print(f"Successfully saved {mode} texture to: {output_path}")

    except FileNotFoundError as e:
        print(f"Error: File not found - {e}")
        sys.exit(1)
    except ValueError as e:
        print(f"Error: Invalid data - {e}")
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        sys.exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Pack R, G, B, and optional A textures into a single image.")

    parser.add_argument("-r", "--red", required=True, help="Path to the texture for the Red channel")
    parser.add_argument("-g", "--green", required=True, help="Path to the texture for the Green channel")
    parser.add_argument("-b", "--blue", required=True, help="Path to the texture for the Blue channel")
    parser.add_argument("-a", "--alpha", required=False, help="Path to the texture for the Alpha channel (optional)")

    parser.add_argument("-o", "--output", default="packed.png", help="Output filename (default: packed.png)")

    args = parser.parse_args()

    pack_channels(args.red, args.green, args.blue, args.alpha, args.output)
