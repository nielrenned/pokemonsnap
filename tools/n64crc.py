#!/usr/bin/env python3
"""Recompute and write the N64 IPL3 (CIC) checksum into a ROM header.

Usage: n64crc.py <rom>   (patches the rom in place)
"""
import sys

CIC_SEEDS = {
    6101: 0xF8CA4DDC,
    6102: 0xF8CA4DDC,
    6103: 0xA3886759,
    6105: 0xDF26F436,
    6106: 0x1FEA617A,
}

M32 = 0xFFFFFFFF
START = 0x1000
LENGTH = 0x100000


def _u32(x):
    return x & M32


def _rol(value, bits):
    bits &= 0x1F
    return _u32((value << bits) | (value >> (32 - bits)))


def _r32(data, off):
    return int.from_bytes(data[off:off + 4], "big")


def calculate_crc(rom, cic):
    seed = CIC_SEEDS[cic]
    t1 = t2 = t3 = t4 = t5 = t6 = seed
    i = START
    while i < START + LENGTH:
        d = _r32(rom, i)
        if _u32(t6 + d) < t6:
            t4 = _u32(t4 + 1)
        t6 = _u32(t6 + d)
        t3 = _u32(t3 ^ d)
        r = _rol(d, d & 0x1F)
        t5 = _u32(t5 + r)
        if t2 > d:
            t2 = _u32(t2 ^ r)
        else:
            t2 = _u32(t2 ^ t6 ^ d)
        if cic == 6105:
            t1 = _u32(t1 + (_r32(rom, 0x40 + 0x0710 + (i & 0xFF)) ^ d))
        else:
            t1 = _u32(t1 + (t5 ^ d))
        i += 4

    if cic == 6103:
        crc1 = _u32(_u32(t6 ^ t4) + t3)
        crc2 = _u32(_u32(t5 ^ t2) + t1)
    elif cic == 6106:
        crc1 = _u32(_u32(t6 * t4) + t3)
        crc2 = _u32(_u32(t5 * t2) + t1)
    else:
        crc1 = _u32(t6 ^ t4 ^ t3)
        crc2 = _u32(t5 ^ t2 ^ t1)
    return crc1, crc2


def detect_cic(rom):
    """Pick the CIC whose computed CRC matches the header (assumes a valid ROM)."""
    have = (_r32(rom, 0x10), _r32(rom, 0x14))
    for cic in (6102, 6101, 6103, 6105, 6106):
        if calculate_crc(rom, cic) == have:
            return cic
    return None


def main():
    path = sys.argv[1]
    cic = int(sys.argv[2]) if len(sys.argv) > 2 else 6102
    with open(path, "rb") as f:
        rom = bytearray(f.read())
    crc1, crc2 = calculate_crc(rom, cic)
    rom[0x10:0x14] = crc1.to_bytes(4, "big")
    rom[0x14:0x18] = crc2.to_bytes(4, "big")
    with open(path, "wb") as f:
        f.write(rom)


if __name__ == "__main__":
    main()
