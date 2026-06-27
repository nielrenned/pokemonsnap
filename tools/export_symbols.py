#!/usr/bin/env python3
"""Export the memory addresses an external client (Archipelago) needs, as JSON.

Reads the linked ELF symbol table and emits the client interface block plus the
AP data block (with computed sub-field addresses). Run as part of the build so
the addresses stay in sync with the ROM.
"""
import json
import subprocess
import sys

CROSS = "mips-linux-gnu-"


def get_symbols(elf):
    out = subprocess.check_output([CROSS + "nm", elf]).decode()
    syms = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3:
            syms[parts[2]] = int(parts[0], 16)
    return syms


def main():
    in_elf, out_json = sys.argv[1], sys.argv[2]
    s = get_symbols(in_elf)
    ap = s["gApData"]

    # Main save struct: D_800C21B0_5F050 = (&D_800C21B8_5F058 + 0xF) & ~0xF.
    save_base = (s["D_800C21B8_5F058"] + 0xF) & ~0xF
    save = {
        "savePtr": (s["D_800C21B0_5F050"], "ptr", "r",
                    "pointer to the save struct (deref for base; == saveBase)"),
        "saveBase": (save_base, "UnkBigBoy", "r",
                     "main save struct base; full layout in src/more_funcs/more_funcs.h"),
        "rank": (save_base + 0x64, "u32", "r", "current rank = (value >> 29) & 7"),
        "reportScores": (save_base + 0x6C, "s32[69]", "r",
                         "best PKMN Report score per species slot"),
        "registeredPhotos": (save_base + 0x180, "PhotoData[69]", "r",
                             "registered photo per species slot (stride 0x3A0); registered if s32 at +0x04 != -1"),
    }

    symbols = {
        "gExpansionMagic": (s["gExpansionMagic"], "u32", "r",
                            "0x534E4150 'SNAP' in ROM, becomes 0x4F4B4159 'OKAY' once loaded"),
        "gMaxFilm": (s["gMaxFilm"], "s32", "rw", "film cap (below 60)"),
        "gCanUseOverride": (s["gCanUseOverride"], "s32", "rw", "!=0: all items usable"),
        "gCanUseMask": (s["gCanUseMask"], "u32", "rw",
                        "bit0=apple,1=pester,2=flute,3=dash (used when gCanUseOverride==0)"),
        "gCourseOverride": (s["gCourseOverride"], "s32", "rw", "!=0: all courses unlocked"),
        "gCourseUnlockMask": (s["gCourseUnlockMask"], "u32", "rw",
                              "bit0=Beach,1=Tunnel,2=Volcano,3=Cave,4=River,5=Valley,6=Rainbow (used when gCourseOverride==0)"),
        "apMagic": (ap + 0x00, "u32", "r", "0x53414431 'SAD1' when the AP block is valid"),
        "apChecksum": (ap + 0x04, "u32", "r", "checksum over speciesScores"),
        "speciesScores": (ap + 0x08, "u16[69][5]", "r",
                          "[slot][0=special,1=pose,2=size,3=technique,4=samePkmn]; slot=func_8009BB4C(pokemonID)"),
    }

    data = {
        "meta": {
            "endian": "big",
            "addressing": "KSEG0 virtual; physical RDRAM = addr & 0x1FFFFFFF",
            "requires": "8MB Expansion Pak",
        },
        "symbols": {
            name: {"addr": "0x%08X" % addr, "type": t, "access": acc, "desc": desc}
            for name, (addr, t, acc, desc) in symbols.items()
        },
        "save": {
            name: {"addr": "0x%08X" % addr, "type": t, "access": acc, "desc": desc}
            for name, (addr, t, acc, desc) in save.items()
        },
    }

    with open(out_json, "w") as f:
        json.dump(data, f, indent=2)
        f.write("\n")
    print(f"wrote {out_json}")


if __name__ == "__main__":
    main()
