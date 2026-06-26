#include "common.h"

// External-client interface block. Pinned by the linker at the very start of
// the expansion segment (0x80400000) so the addresses are a stable ABI for an
// external client (e.g. Archipelago) to read/write. Layout is fixed; only ever
// append new fields at the end.
//
//   0x80400000  gExpansionMagic    'SNAP' in ROM, becomes 'OKAY' once loaded
//   0x80400004  gMaxFilm           film cap (s32, below 60)
//   0x80400008  gCanUseOverride    !=0: all items usable
//   0x8040000C  gCanUseMask        bit0=apple,1=pester,2=flute,3=dash
//   0x80400010  gCourseOverride    !=0: all courses unlocked
//   0x80400014  gCourseUnlockMask  bit0=Beach..6=Rainbow

u32 gExpansionMagic = 0x534E4150;
s32 gMaxFilm = 15;
s32 gCanUseOverride = 1;
u32 gCanUseMask = 0;
s32 gCourseOverride = 1;
u32 gCourseUnlockMask = 0;
