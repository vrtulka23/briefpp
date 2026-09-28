"""Generate the example figure with Python's standard library."""

import math
import struct
import zlib
from pathlib import Path

WIDTH, HEIGHT = 480, 280
pixels = bytearray([255, 255, 255] * WIDTH * HEIGHT)


def point(x, y, color):
    if 0 <= x < WIDTH and 0 <= y < HEIGHT:
        start = 3 * (y * WIDTH + x)
        pixels[start:start + 3] = bytes(color)


for x in range(55, 445):
    point(x, 235, (60, 60, 60))
for y in range(25, 236):
    point(55, y, (60, 60, 60))
for x in range(56, 444):
    y = 235 - round(205 * math.exp(-(x - 56) / 140))
    for offset in (-1, 0, 1):
        point(x, y + offset, (35, 95, 170))


def chunk(kind, data):
    return (struct.pack(">I", len(data)) + kind + data
            + struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff))


raw = b"".join(b"\0" + bytes(pixels[y * WIDTH * 3:(y + 1) * WIDTH * 3])
               for y in range(HEIGHT))
png = (b"\x89PNG\r\n\x1a\n"
       + chunk(b"IHDR", struct.pack(">2I5B", WIDTH, HEIGHT, 8, 2, 0, 0, 0))
       + chunk(b"IDAT", zlib.compress(raw))
       + chunk(b"IEND", b""))
Path(__file__).with_name("output").mkdir(exist_ok=True)
Path(__file__).with_name("output").joinpath("density.png").write_bytes(png)
