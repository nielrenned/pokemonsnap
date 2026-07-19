#!/usr/bin/env python3
"""Copy the client symbols + basepatch into the Archipelago apworld, if configured.

Usage: sync_apworld.py <config> <symbols_json> <bsdiff> <stamp>

<config> is an optional text file whose first non-empty, non-comment line is the
path to the apworld directory (e.g. ../PSAP/apworld/pokemon_snap). When it is
absent the sync is skipped silently so the normal build is unaffected.
"""
import os
import shutil
import sys


def read_apworld_dir(config: str) -> str | None:
    if not os.path.exists(config):
        return None
    with open(config) as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith("#"):
                return os.path.expanduser(line)
    return None


def main() -> None:
    config, symbols_json, bsdiff, stamp = sys.argv[1:5]

    apworld = read_apworld_dir(config)
    if apworld is None:
        print(f"sync_apworld: no {config}, skipping apworld sync")
    elif not os.path.isdir(apworld):
        print(f"sync_apworld: apworld dir not found: {apworld}", file=sys.stderr)
        sys.exit(1)
    else:
        data_dir = os.path.join(apworld, "data")
        os.makedirs(data_dir, exist_ok=True)
        shutil.copyfile(symbols_json, os.path.join(data_dir, os.path.basename(symbols_json)))
        shutil.copyfile(bsdiff, os.path.join(data_dir, "basepatch.bsdiff4"))
        print(f"sync_apworld: copied symbols + basepatch into {apworld}")

    with open(stamp, "w") as f:
        f.write("")


if __name__ == "__main__":
    main()
