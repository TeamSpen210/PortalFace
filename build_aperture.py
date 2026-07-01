from PIL import Image

SIZE = 12

def main():
    img = Image.open('resources/images/battery/aperture.png')
    img.load()
    colors = {
        (0, 255, 0): 0,  # Background
        (255, 0, 0): 1,  # Wedge 1
        (0, 0, 255): 2,  # Wedge 2
    }
    pixels = [
        colors[img.getpixel((x, y))]
        for y in range(SIZE)
        for x in range(SIZE)
    ]
    for y in range(0,SIZE*SIZE, SIZE):
        print(''.join(map(str, pixels[y:y+SIZE])))
    # 12x12, 2 bits per pixel, one byte per 4 pixels.
    result = bytearray()
    for off in range(0, len(pixels), 4):
        byte = pixels[off]
        byte |= pixels[off+1] << 2
        byte |= pixels[off+2] << 4
        byte |= pixels[off+3] << 6
        result.append(byte)
    print(result)

main()
