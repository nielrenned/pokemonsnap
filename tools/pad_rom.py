#!/usr/bin/env python3
"""Zero-pad a ROM in place up to a multiple of 1 KiB: pad_rom.py <rom>.

BizHawk treats any file whose size is 128 or 512 bytes past a 1 KiB boundary
as having a copier header and strips that many bytes from the front, which
breaks the ROM. The expansion segment makes the ROM size arbitrary, so round
it up to a 1 KiB multiple to keep BizHawk's header heuristic from firing.
"""
import sys

ALIGN = 0x400


def main() -> None:
    path = sys.argv[1]
    with open(path, "r+b") as f:
        size = f.seek(0, 2)
        pad = -size % ALIGN
        if pad:
            f.write(b"\x00" * pad)


if __name__ == "__main__":
    main()
