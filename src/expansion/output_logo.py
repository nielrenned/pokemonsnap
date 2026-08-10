from PIL import Image
import math

im = Image.open('icon.png')
im_bytes = im.tobytes()

def round_half_up(x):
    return math.floor(x + 0.5)

# Convert to 5-bit RGB + 1 alpha bit
new_bytes = []
for i in range(0, len(im_bytes), 4):
    r, g, b, a = im_bytes[i:i+4]
    new_r, new_g, new_b = map(lambda x: round_half_up(x / 255 * 31), (r, g, b))
    word = (new_r << 11) | (new_g << 6) | (new_b << 1) | 1
    high, low = word >> 8, word & 0xFF
    new_bytes.extend([high, low])

# Do odd-line 64-bit high/low word swap
for y in range(42):
    if y % 2 == 0: continue
    for x in range(y*88, (y+1)*88, 8):
        new_bytes[x:x+4], new_bytes[x+4:x+8] = new_bytes[x+4:x+8], new_bytes[x:x+4]

with open('icon_archipelago_logo.png.bin.c', 'w') as f:
    f.write(", ".join(map(str, new_bytes)))
