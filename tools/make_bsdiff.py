#!/usr/bin/env python3
"""Write a bsdiff4 patch transforming src into dst: make_bsdiff.py <src> <dst> <patch>."""
import sys

import bsdiff4


def main() -> None:
    src, dst, patch = sys.argv[1], sys.argv[2], sys.argv[3]
    bsdiff4.file_diff(src, dst, patch)


if __name__ == "__main__":
    main()
