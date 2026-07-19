#ifndef _APDATA_H_
#define _APDATA_H_

#include "common.h"

// Validity marker for the AP flash block.
#define AP_MAGIC 0x53414431 // 'SAD1'

struct ApData_s {
    u32 magic;
    u32 checksum; // sum over speciesScores
    s16 speciesScores[69][6]; // [species slot][0=special,1=pose,2=size,3=technique,4=samePkmn,5=specialFlags]
};

// AP data block, stored in expansion RAM (pinned in iface.c) and persisted to a
// dedicated FLASH region after the main save (see expansion.c). Page-aligned
// (0x380 = 7 flash pages) so flash read/write never overruns the buffer.
typedef union {
    struct ApData_s;
    u8 padding[0x380];
} ApData; // size = 0x380

extern ApData gApData;

#endif
