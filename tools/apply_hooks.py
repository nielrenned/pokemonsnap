#!/usr/bin/env python3
"""Redirect existing game functions into expansion code, size-neutrally.

For each `target = replacement` entry in the hook file, overwrites the target
function's first two instructions in the linked ELF with `j replacement; nop`.
The target keeps its exact footprint, so nothing downstream shifts. Patching by
ELF file offset means objcopy then emits the bytes at the correct ROM position.
"""
import re
import subprocess
import sys

CROSS = "mips-linux-gnu-"


def get_sections(elf):
    # section index -> (addr, file offset)
    out = subprocess.check_output([CROSS + "readelf", "-S", elf]).decode()
    secs = {}
    for line in out.splitlines():
        m = re.search(
            r"\[\s*(\d+)\]\s+\S+\s+\S+\s+([0-9a-f]+)\s+([0-9a-f]+)\s+", line
        )
        if m:
            secs[int(m.group(1))] = (int(m.group(2), 16), int(m.group(3), 16))
    return secs


def get_symbols(elf):
    # name -> (value, section index)
    out = subprocess.check_output([CROSS + "readelf", "-sW", elf]).decode()
    syms = {}
    for line in out.splitlines():
        m = re.match(
            r"\s*\d+:\s+([0-9a-f]+)\s+\S+\s+\S+\s+\S+\s+\S+\s+(\S+)\s+(\S+)\s*$", line
        )
        if m and m.group(2).isdigit():
            syms[m.group(3)] = (int(m.group(1), 16), int(m.group(2)))
    return syms


def main():
    in_elf, hooks_file, out_elf = sys.argv[1], sys.argv[2], sys.argv[3]
    syms = get_symbols(in_elf)
    secs = get_sections(in_elf)
    data = bytearray(open(in_elf, "rb").read())

    with open(hooks_file) as f:
        for line in f:
            line = line.split("#")[0].strip()
            if not line:
                continue
            tgt, repl = [s.strip() for s in line.split("=")]
            # "call:" prefix retargets a single jal at the site (keeps delay slot);
            # otherwise the target's first two words become `j replacement; nop`.
            call_site = tgt.startswith("call:")
            if call_site:
                tgt = tgt[len("call:"):].strip()
            # target may be "symbol" or "symbol+0xOFFSET" (mid-function detour)
            tgt_off = 0
            if "+" in tgt:
                tgt, off_str = [s.strip() for s in tgt.split("+")]
                tgt_off = int(off_str, 0)
            if tgt not in syms:
                sys.exit(f"hook: target symbol '{tgt}' not found")
            if repl not in syms:
                sys.exit(f"hook: replacement symbol '{repl}' not found")
            tgt_vma, tgt_ndx = syms[tgt]
            tgt_vma += tgt_off
            repl_vma = syms[repl][0]
            if tgt_ndx not in secs:
                sys.exit(f"hook: {tgt} has no resolvable section (ndx {tgt_ndx})")
            sec_addr, sec_off = secs[tgt_ndx]
            foff = sec_off + (tgt_vma - sec_addr)
            if call_site:
                instr = 0x0C000000 | ((repl_vma >> 2) & 0x03FFFFFF)
                data[foff : foff + 4] = instr.to_bytes(4, "big")
                kind = "jal"
            else:
                instr = 0x08000000 | ((repl_vma >> 2) & 0x03FFFFFF)
                data[foff : foff + 4] = instr.to_bytes(4, "big")
                data[foff + 4 : foff + 8] = (0).to_bytes(4, "big")
                kind = "j"
            print(
                f"hook {tgt} @ {tgt_vma:#010x} -> {repl} @ {repl_vma:#010x}"
                f"  ({kind} 0x{instr:08x} @ file 0x{foff:x})"
            )

    open(out_elf, "wb").write(data)


if __name__ == "__main__":
    main()
