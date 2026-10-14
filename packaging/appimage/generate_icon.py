#!/usr/bin/env python3
"""Generate a simple solid-colour PNG icon for the AppImage (no dependencies)."""
import struct
import sys
import zlib


def chunk(tag, data):
    return (
        struct.pack(">I", len(data))
        + tag
        + data
        + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
    )


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else "comic-reader.png"
    size = 256
    background = bytes((30, 136, 229))
    raw = b"".join(b"\x00" + background * size for _ in range(size))
    png = (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(raw, 9))
        + chunk(b"IEND", b"")
    )
    with open(out, "wb") as handle:
        handle.write(png)


if __name__ == "__main__":
    main()
