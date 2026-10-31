#ifndef _APDATA_H_
#define _APDATA_H_

#include "common.h"

// Validity marker for the AP flash block.
#define AP_MAGIC 0x53414431 // 'SAD1'

typedef struct ApSpeciesScore {
    s16 specialScore;
    s16 poseScore;
    s16 lastPoseId;
    s16 sizeScore;
    s16 isWellFramed;
    s16 samePkmnBonus;
    s16 specialPoseFlags;
    s16 totalScore; // photo total is u32, but it will fit in an s16 
} ApSpeciesScore;

struct ApData_s {
    u32 magic;
    u32 checksum; // sum over speciesScores
    ApSpeciesScore speciesScores[73];
    u8 signsFound[6];
    u8 secretExits;
};

typedef struct {
    u16 frame;
    s16 speciesSlotId;
    s16 specialPoseId;
    s16 sizeScore;
    s16 poseId;
    s16 poseScore;
    s16 goodTechnique;
    s16 multipleScore;
    s16 totalScore;
} ApRawPhotoScore;

typedef struct { 
    u16 frame;
    u16 type;
} ApRunEvent;

// AP data block, stored in expansion RAM (pinned in iface.c) and persisted to a
// dedicated FLASH region after the main save (see expansion.c). Page-aligned
// to 0x80-sized pages so flash read/write never overruns the buffer.
typedef union {
    struct ApData_s;
    u8 padding[0x500];
} ApData;

extern ApData gApData;

#endif
