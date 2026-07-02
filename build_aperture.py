from PIL import Image

SIZE = 12

def main():
    img = Image.open('resources/images/battery/aperture.png')
    img.load()
    colors = {
        #  1st bit = background, 2nd = wedge 2, 3/4 = index
        (  0, 255,   0): 0b1000,  # Background
        ( 64,   0,   0): 0b0000,  # Wedge 1a
        (128,   0,   0): 0b0001,  # Wedge 1b
        (192,   0,   0): 0b0010,  # Wedge 1c
        (255,   0,   0): 0b0011,  # Wedge 1d
        (  0,   0,  64): 0b0100,  # Wedge 2a
        (  0,   0, 128): 0b0101,  # Wedge 2b
        (  0,   0, 192): 0b0110,  # Wedge 2c
        (  0,   0, 255): 0b0111,  # Wedge 2d
    }
    pixels = [
        colors[img.getpixel((x, y))]
        for y in range(SIZE)
        for x in range(SIZE)
    ]
    for y in range(0,SIZE*SIZE, SIZE):
        print(''.join(map(str, pixels[y:y+SIZE])))
    # 12x12, 4 bits per pixel, one byte per 2 pixels.
    result = bytearray([
        pixels[off] | (pixels[off+1] << 4)
        for off in range(0, len(pixels), 2)
    ])
    print(f'APERTURE_LAYOUT[{len(result)}]=', ''.join(f'\\x{i:02X}' for i in result))

main()
