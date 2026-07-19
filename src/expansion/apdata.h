#ifndef _APDATA_H_
#define _APDATA_H_

#include "common.h"

// Validity marker for the AP flash block.
#define AP_MAGIC 0x53414431 // 'SAD1'

// AP data block, stored in expansion RAM (pinned in iface.c) and persisted to a
// dedicated FLASH region after the main save (see expansion.c). Page-aligned
// (0x300 = 6 flash pages) so flash read/write never overruns the buffer.
typedef struct ApData {
    /* 0x000 */ u32 magic;
    /* 0x004 */ u32 checksum; // sum over speciesScores
    /* 0x008 */ u16 speciesScores[69][5]; // [species slot][0=special,1=pose,2=size,3=technique,4=samePkmn]
    /* 0x2BA */ u8 pad[0x300 - 0x2BA];
} ApData; // size = 0x300

extern ApData gApData;

#endif
