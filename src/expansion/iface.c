#include "common.h"
#include "apdata.h"

// External-client interface block. Pinned by the linker at the very start of
// the expansion segment (0x80400000) so the addresses are a stable ABI for an
// external client (e.g. Archipelago) to read/write. Layout is fixed; only ever
// append new fields at the end.
//
//   gExpansionMagic     'SNAP' in ROM, becomes 'OKAY' once loaded
//   gMaxFilm            film cap (s32, below 60)
//   gCanUseOverride     !=0: all items usable
//   gCanUseMask         bit0=apple, 1=pester, 2=flute, 3=dash, 4=sign detector, 5=L to stop, 6=technique scoring, 7=multiple scoring
//   gCourseOverride     !=0: all courses unlocked
//   gCourseUnlockMask   bit0=Beach..6=Rainbow
//   gDialogRequestFlags bit0=Cloud, 1=24k, 2=72.5k, 3=130k, 4=6 Pokemon, 5=22 Pokemon, 6=40 Pokemon
//   gDialogPlayedFlags  bit0=Cloud, 1=24k, 2=72.5k, 3=130k, 4=6 Pokemon, 5=22 Pokemon, 6=40 Pokemon

// Everything is 32-bits for alignment reasons
u32 gExpansionMagic = 0x534E4150;
s32 gMaxFilm = 15;
s32 gCanUseOverride = 1;
u32 gCanUseMask = 0;
u32 gCanUsePerLevelMask[7] = {0, 0, 0, 0, 0, 0, 0};
s32 gCourseOverride = 1;
u32 gCourseUnlockMask = 0;
u32 gDialogRequestFlags = 0;
u32 gDialogPlayedFlags = 0;
u32 gCameraInversionYAMLOption = 0;
u32 gCameraInversionApplied = 0;

// AP data block, pinned right after the scalar interface (magic initializer
// forces .data placement so the client address is stable: scores at 0x80400020).
ApData gApData = { AP_MAGIC };

// Per-seed auth token patched in by the apworld; the client reads it (base64)
// as its connect name. Non-zero init keeps it in .data; sentinel = unpatched.
u8 gApAuth[16] = "PSAP-UNPATCHED!!";
