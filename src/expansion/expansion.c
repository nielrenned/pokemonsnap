#include "common.h"
#include "more_funcs/more_funcs.h"
#include "app_level/app_level.h"
#include "photo_check/photo_check.h"
#include "sys/render.h"
#include "PR/os_flash.h"
#include "apdata.h"

extern UnkBigBoy* D_800C21B0_5F050;
extern s32 photocheck_oaksMark(Photo* photo); // The photo scoring UI routine
extern void score_CalculateScore(ScoreData*, PhotoData*, s32);
extern s32 func_8009BB4C(s32);
extern OSMesgQueue D_800E17A8_7E648; // flash message queue
extern s32 IsInputDisabled;
extern s32 ThrowItemTimeout;
extern s32 PressPokeFluteTimeout;
extern s32 LastItemId;
extern Vec3f PlayerVelocity;

// Mirrors of icons.c-local types used by Icons_Init.
enum IconSpriteIds {
    ICON_ID_APPLE = 0,
    ICON_ID_PESTER_BALL = 1,
    ICON_ID_FLUTE = 2,
    ICON_ID_ZOOM = 3,
    ICON_ID_DASH = 4,
    ICON_ID_DASH_ZOOMED = 5,
    ICON_ID_TAKE_PHOTO = 6,
    ICON_ID_ZOOM_OFF = 7
};

typedef struct SpriteDefStruct {
    /* 0x00 */ u32 x;
    /* 0x04 */ u32 y;
    /* 0x08 */ s32 shownWhenZoomedIn;
    /* 0x0C */ s32 moveOutDirection;
    /* 0x10 */ s32 unk_10;
    /* 0x14 */ Sprite* spriteDef;
} SpriteDefStruct;

typedef struct SpriteStruct {
    /* 0x00 */ u32 x;
    /* 0x04 */ u32 y;
    /* 0x08 */ SObj* spriteObj;
    /* 0x0C */ char unused[12];
} SpriteStruct;

extern SpriteStruct Icons_IconObjects[];
extern SpriteDefStruct Icons_IconDefs[];
extern SObj* Icons_ButtonIcons[];
extern SObj* Icons_ButtonIconsCopy[];
extern s32 D_80388204_528614[];
extern GObj* Icons_MainObject;
extern u32 D_8038821C_52862C;
extern s32 Icons_MoveOutCounter[8];
extern u8 Icons_ItemFlags;
extern void Icons_UpdateDefault(GObj*);
extern void Icons_UpdateDashEngineIcon(GObj*);

extern UNK_TYPE D_80388F58_529368;
extern UNK_TYPE D_803890B8_5294C8;
extern UNK_TYPE D_80389218_529628;
extern UNK_TYPE D_8038A034_52A444[];
extern SObj* D_803B0A14_550E24;
extern SObj* D_803B0A18_550E28;
extern SObj* D_803B0A1C_550E2C;
extern s32 func_8009BC68(void);

// Client interface block (defined in iface.c, pinned at 0x80400000).
extern u32 gExpansionMagic;
extern s32 gMaxFilm;
extern s32 gCanUseOverride;
extern u32 gCanUseMask;
extern s32 gCourseOverride;
extern u32 gCourseUnlockMask;

s32 exp_canUse(s32 bit, s32 savedBit) {
    return gCanUseOverride || ((gCanUseMask >> bit) & 1) || savedBit;
}

void expansion_init(void) {
    gExpansionMagic = 0x4F4B4159;
}

// Generic no-op replacement (skips Oak's first-visit intro speech). The lab's
// dispatcher still consumes the one-shot event bit, so it just doesn't display.
void exp_noop(void) {
}

// Returns max rank (6) so the course-select text lookup indexes the full
// course list D_80195CEC_95B50C[6], making every shown course's text resolve.
s32 exp_rank6(void) {
    return 6;
}

// Drives the dash engine's availability (icon + R-trigger boost) off canUse,
// via the single checkPlayerFlag(DASH) call in initUI that builds ProgressFlags.
s32 exp_dashAvailable(void) {
    return exp_canUse(3, D_800C21B0_5F050->data.canUseDashEngine);
}

// Replaces the func_800BF864_5C704() (Pokedex count) call in initUI that gates
// the in-course tutorial: marks the tutorial finished (persisted) and returns a
// count >= 4 so IsTutorialEnabled is forced false.
s32 exp_tutorialDone(void) {
    D_800C21B0_5F050->data.hasFinishedTutorial = 1;
    return 4;
}

void exp_handleItemButtonsPress(GObj* obj) {
    if (!IsInputDisabled) {
        if ((gContInputPressedButtons & D_CBUTTONS) &&
            exp_canUse(2, D_800C21B0_5F050->data.canUseFlute) && PressPokeFluteTimeout == 0) {
            LastItemId = ITEM_ID_POKEFLUTE;
            PressPokeFluteTimeout = 45;
            Icons_ProcessButtonPress(ITEM_ID_POKEFLUTE);
            Items_PlayPokeFlute();
        } else if (ThrowItemTimeout == 0) {
            if ((gContInputPressedButtons & B_BUTTON) &&
                exp_canUse(1, D_800C21B0_5F050->data.canUsePesterBall)) {
                LastItemId = ITEM_ID_PESTER_BALL;
                ThrowItemTimeout = 45;
                Items_SpawnPesterBall(&gMainCamera->viewMtx.lookAt.eye, &PlayerVelocity);
                Icons_ProcessButtonPress(ITEM_ID_PESTER_BALL);
                Items_StopPokeFlute();
                Icons_ProcessButtonPress(-1);
                PressPokeFluteTimeout = 0;
            } else if ((gContInputPressedButtons & A_BUTTON) &&
                       exp_canUse(0, D_800C21B0_5F050->data.canUseApple)) {
                LastItemId = ITEM_ID_APPLE;
                ThrowItemTimeout = 45;
                Items_SpawnApple(&gMainCamera->viewMtx.lookAt.eye, &PlayerVelocity);
                Icons_ProcessButtonPress(ITEM_ID_APPLE);
                Items_StopPokeFlute();
                Icons_ProcessButtonPress(-1);
                PressPokeFluteTimeout = 0;
            }
        }
    }
    if (ThrowItemTimeout > 0) {
        ThrowItemTimeout--;
    }
    if (PressPokeFluteTimeout > 0) {
        PressPokeFluteTimeout--;
    }
}

s32 exp_filmGetRemaining(void) {
    return gMaxFilm - func_8009BC68();
}

s32 exp_filmUpdateCounter(void) {
    s32 ones, tens, hundreds;
    s32 value = gMaxFilm - func_8009BC68();

    ones = value % 10;
    tens = (value % 100) / 10;
    hundreds = value / 100;

    D_80388F58_529368 = D_8038A034_52A444[ones];

    if (hundreds == 0) {
        if (tens == 0) {
            spSetAttribute(&D_803B0A18_550E28->sprite, SP_HIDDEN);
        } else {
            D_803890B8_5294C8 = D_8038A034_52A444[tens];
            spClearAttribute(&D_803B0A18_550E28->sprite, SP_HIDDEN);
        }
        spSetAttribute(&D_803B0A1C_550E2C->sprite, SP_HIDDEN);
    } else {
        D_803890B8_5294C8 = D_8038A034_52A444[tens];
        spClearAttribute(&D_803B0A18_550E28->sprite, SP_HIDDEN);
        D_80389218_529628 = D_8038A034_52A444[hundreds];
        spClearAttribute(&D_803B0A1C_550E2C->sprite, SP_HIDDEN);
    }

    if (value == 10) {
        spColor(&D_803B0A14_550E24->sprite, 255, 0, 0, 255);
        spColor(&D_803B0A18_550E28->sprite, 255, 0, 0, 255);
    }

    return value;
}

void exp_Icons_Init(void) {
    GObj* gobj;
    s32 i;
    u32 progressFlags;
    SpriteStruct* spr;
    SpriteDefStruct* sprDef;
    s32 apple = exp_canUse(0, D_800C21B0_5F050->data.canUseApple);
    s32 pester = exp_canUse(1, D_800C21B0_5F050->data.canUsePesterBall);
    s32 flute = exp_canUse(2, D_800C21B0_5F050->data.canUseFlute);

    progressFlags = getProgressFlags();
    Icons_ItemFlags = 0;
    D_8038821C_52862C = 0;
    if (apple) {
        Icons_ItemFlags |= PF_HAS_APPLE;
    }
    if (pester) {
        Icons_ItemFlags |= PF_HAS_PESTER_BALL;
    }
    if (flute) {
        Icons_ItemFlags |= PF_HAS_FLUTE;
    }
    Icons_NumItemsAvailable = (apple != 0) + (pester != 0) + (flute != 0);

    if (Icons_NumItemsAvailable > 0) {
        LastItemId = D_80388204_528614[D_8038821C_52862C];
    } else {
        LastItemId = -1;
    }
    gobj = omAddGObj(26, Icons_UpdateDefault, 0, 0x80000000);
    omLinkGObjDL(gobj, &renDrawSprite, 1, 0x80000000, -1);
    Icons_MainObject = gobj;

    spr = &Icons_IconObjects[ICON_ID_ZOOM];
    sprDef = &Icons_IconDefs[ICON_ID_ZOOM];
    spr->spriteObj = omGObjAddSprite(gobj, sprDef->spriteDef);
    spMove(&spr->spriteObj->sprite, sprDef->x, sprDef->y);
    SET_SPRITE_POS_PTR(spr, sprDef->x, sprDef->y);

    if (progressFlags & PF_HAS_DASH_ENGINE) {
        spr = &Icons_IconObjects[ICON_ID_DASH];
        sprDef = &Icons_IconDefs[ICON_ID_DASH];
        spr->spriteObj = omGObjAddSprite(gobj, sprDef->spriteDef);
        spMove(&spr->spriteObj->sprite, sprDef->x, sprDef->y);
        SET_SPRITE_POS_PTR(spr, sprDef->x, sprDef->y);

        spr = &Icons_IconObjects[ICON_ID_DASH_ZOOMED];
        sprDef = &Icons_IconDefs[ICON_ID_DASH_ZOOMED];
        spr->spriteObj = omGObjAddSprite(gobj, sprDef->spriteDef);
        spMove(&spr->spriteObj->sprite, sprDef->x, sprDef->y);
        SET_SPRITE_POS_PTR(spr, sprDef->x, sprDef->y);
        spSetAttribute(&spr->spriteObj->sprite, SP_HIDDEN);
    }

    spr = &Icons_IconObjects[ICON_ID_TAKE_PHOTO];
    sprDef = &Icons_IconDefs[ICON_ID_TAKE_PHOTO];
    spr->spriteObj = omGObjAddSprite(gobj, sprDef->spriteDef);
    spMove(&spr->spriteObj->sprite, sprDef->x, sprDef->y);
    SET_SPRITE_POS_PTR(spr, sprDef->x, sprDef->y);
    spSetAttribute(&spr->spriteObj->sprite, SP_HIDDEN);

    if (progressFlags & PF_ZOOM_SWITCH) {
        spr = &Icons_IconObjects[ICON_ID_ZOOM_OFF];
        sprDef = &Icons_IconDefs[ICON_ID_ZOOM_OFF];
        spr->spriteObj = omGObjAddSprite(gobj, sprDef->spriteDef);
        spMove(&spr->spriteObj->sprite, sprDef->x, sprDef->y);
        SET_SPRITE_POS_PTR(spr, sprDef->x, sprDef->y);
        spSetAttribute(&spr->spriteObj->sprite, SP_HIDDEN);
    }

    if (flute) {
        spr = &Icons_IconObjects[ICON_ID_FLUTE];
        sprDef = &Icons_IconDefs[ICON_ID_FLUTE];
        spr->spriteObj = Icons_ButtonIcons[ICON_ID_FLUTE] = omGObjAddSprite(gobj, sprDef->spriteDef);
        spMove(&spr->spriteObj->sprite, sprDef->x, sprDef->y);
        SET_SPRITE_POS_PTR(spr, sprDef->x, sprDef->y);
    }
    if (apple) {
        spr = &Icons_IconObjects[ICON_ID_APPLE];
        sprDef = &Icons_IconDefs[ICON_ID_APPLE];
        spr->spriteObj = Icons_ButtonIcons[ICON_ID_APPLE] = omGObjAddSprite(gobj, sprDef->spriteDef);
        spMove(&spr->spriteObj->sprite, sprDef->x, sprDef->y);
        SET_SPRITE_POS_PTR(spr, sprDef->x, sprDef->y);
    }
    if (pester) {
        spr = &Icons_IconObjects[ICON_ID_PESTER_BALL];
        sprDef = &Icons_IconDefs[ICON_ID_PESTER_BALL];
        spr->spriteObj = Icons_ButtonIcons[ICON_ID_PESTER_BALL] = omGObjAddSprite(gobj, sprDef->spriteDef);
        spMove(&spr->spriteObj->sprite, sprDef->x, sprDef->y);
        SET_SPRITE_POS_PTR(spr, sprDef->x, sprDef->y);
    }

    Icons_ButtonIconsCopy[0] = Icons_ButtonIcons[0];
    Icons_ButtonIconsCopy[1] = Icons_ButtonIcons[1];
    Icons_ButtonIconsCopy[2] = Icons_ButtonIcons[2];

    omCreateProcess(gobj, Icons_UpdateDashEngineIcon, 1, 1);

    for (i = 0; i < ARRAY_COUNT(Icons_MoveOutCounter); i++) {
        Icons_MoveOutCounter[i] = 0;
    }
}

extern s32 D_801F3E28_9A3898;
extern s32 func_800BFC5C_5CAFC(void);

// Rank-up awards set the "unlocked" flags only; actual item usability is driven
// by the AP canUse mask. Each tier checks its own flag so a higher-tier item
// doesn't lock out the lower tiers.
void exp_awardItems(s32 score) {
    if (score >= 130000 && func_800BFC5C_5CAFC() >= 5 && checkPlayerFlag(PFID_HAS_FLUTE) == 0) {
        D_801F3E28_9A3898 = 0x200;
        setPlayerFlag(PFID_HAS_FLUTE, 1);
    } else if (score >= 72500 && func_800BFC5C_5CAFC() >= 3 && checkPlayerFlag(PFID_HAS_PESTER_BALL) == 0) {
        D_801F3E28_9A3898 = 0x100;
        setPlayerFlag(PFID_HAS_PESTER_BALL, 1);
    } else if (score >= 24000 && func_800BFC5C_5CAFC() > 0 && checkPlayerFlag(PFID_HAS_APPLE) == 0) {
        D_801F3E28_9A3898 = 0x80;
        setPlayerFlag(PFID_HAS_APPLE, 1);
    }
}

extern s32 func_80374F30_8486E0(void* elem, s32 flag);

static s32 sSkipOakReportBox = 0;

// The report eval shows a second Oak textbox telling the player how many more
// photos until the next course unlocks (ranks 4/2/0). Courses are AP items now,
// so that hint is meaningless: redirect those three text prints here to flag the
// box for skipping (and print nothing, avoiding a one-frame flash of the text).
void exp_skipOakBox(void) {
    sSkipOakReportBox = 1;
}

// Wraps the shared textbox wait at the end of the report eval: skip it (no box,
// no input wait) when the selected message was a suppressed next-course hint.
s32 exp_oakReportWait(void* elem, s32 flag) {
    if (sSkipOakReportBox) {
        sSkipOakReportBox = 0;
        return 0;
    }
    return func_80374F30_8486E0(elem, flag);
}

#pragma GLOBAL_ASM("src/expansion/award_detour.s")

extern UIButton* D_80195CEC_95B50C[];
extern void UILayout_CreateButtons(UIButton*);

// While set, every course is selectable regardless of its courseX flag
// (testing default-on). Clear once an external grantor drives the course flags.
static UIButton sCourseButtons[12];

static s32 exp_courseUnlocked(s32 level) {
    s32 saved;
    switch (level) {
        case 0: saved = D_800C21B0_5F050->data.courseBeach; break;
        case 1: saved = D_800C21B0_5F050->data.courseTunnel; break;
        case 2: saved = D_800C21B0_5F050->data.courseVolcano; break;
        case 3: saved = D_800C21B0_5F050->data.courseCave; break;
        case 4: saved = D_800C21B0_5F050->data.courseRiver; break;
        case 5: saved = D_800C21B0_5F050->data.courseValley; break;
        case 6: saved = D_800C21B0_5F050->data.courseRainbow; break;
        default: saved = 0; break;
    }
    return ((gCourseUnlockMask >> level) & 1) || saved;
}

// Replaces UILayout_CreateButtons at the lab course-select sites. Builds the
// button list from the full (rank-6) course list, keeping each course button
// (ids 6..12) only if its flag is set (or override) and all non-course buttons.
void exp_CreateCourseButtons(UIButton* rankList) {
    UIButton* src = D_80195CEC_95B50C[6];
    s32 n = 0;
    s32 i;

    // Clear every slot: the cursor navigation indexes this array mod 8 and only
    // skips BUTTON_NONE, so stale entries past the list would be navigable.
    for (i = 0; i < 12; i++) {
        sCourseButtons[i].id = 35 /* BUTTON_NONE */;
        sCourseButtons[i].text = NULL;
    }

    while (src->id != 35 /* BUTTON_NONE */) {
        s32 keep = 1;
        if (src->id >= 6 && src->id <= 12) {
            keep = gCourseOverride || exp_courseUnlocked(src->id - 6);
        }
        if (keep) {
            sCourseButtons[n] = *src;
            n++;
        }
        src++;
    }
    UILayout_CreateButtons(sCourseButtons);
}

extern s32 func_800E3264_8A8A84(void*, s32*);

// The course-select menu seeds/stores the cursor as a level number, but the
// filtered list is compacted, so level != list index. Map a level to its index
// in the compacted list (0 if that course isn't present).
static s32 exp_levelToIndex(s32 level) {
    s32 targetId = level + 6;
    s32 i;
    for (i = 0; sCourseButtons[i].id != 35 /* BUTTON_NONE */; i++) {
        if (sCourseButtons[i].id == targetId) {
            return i;
        }
    }
    return 0;
}

// Wraps the initial cursor placement: convert the seeded level to its index.
void exp_courseCursorInit(void* arg0, s32* cursor) {
    *cursor = exp_levelToIndex(*cursor);
    func_800E3264_8A8A84(arg0, cursor);
}

// Wraps the focus update in the course-confirmation loop, which drives the
// cursor straight from the stored level. Navigate by compacted index, then sync
// the level back so the confirmed/returned course stays correct.
void exp_courseConfirmFocus(void* input, s32* level) {
    s32 idx = exp_levelToIndex(*level);
    func_800E3264_8A8A84(input, &idx);
    if (sCourseButtons[idx].id >= 6 && sCourseButtons[idx].id <= 12) {
        *level = sCourseButtons[idx].id - 6;
    }
}

// Wraps score_CalculateScore at its call sites: after the real scoring, record
// which bonus types this species earned into the persisted save (speciesBonus).
// Bits: 0=special, 1=pose, 2=size, 3=technique, 4=samePkmn.
// Page that the AP block lives at in FLASH: right after the main save.
#define AP_FLASH_PAGE ((s32) ((sizeof(UnkBigBoy) + 0x7F) / 0x80))

static u32 exp_apChecksum(void) {
    u32 sum = 0;
    u8* p = (u8*) gApData.speciesScores;
    s32 i;
    for (i = 0; i < (s32) sizeof(gApData.speciesScores); i++) {
        sum += p[i];
    }
    return sum;
}

// Read the AP block from FLASH (page-aligned, no overrun since sizeof is a multiple of 0x80).
static void exp_apFlashRead(void) {
    OSIoMesg mb;
    u8* ram = (u8*) &gApData;
    s32 page = AP_FLASH_PAGE;
    s32 size = sizeof(gApData);

    while (size > 0) {
        osInvalDCache(ram, 0x80);
        osFlashReadArray(&mb, 0, page, ram, 1, &D_800E17A8_7E648);
        osRecvMesg(&D_800E17A8_7E648, NULL, 1);
        ram += 0x80;
        page++;
        size -= 0x80;
    }
}

// Write the AP block to FLASH. Must be called right after the main save: the
// main write erases sector 7 (which also covers these spare pages), so they are
// already blank here and we must NOT erase again (that would wipe the main save).
static void exp_apFlashWrite(void) {
    OSIoMesg mb;
    u8* ram = (u8*) &gApData;
    s32 page = AP_FLASH_PAGE;
    s32 size = sizeof(gApData);

    while (size > 0) {
        osWritebackDCache(ram, 0x80);
        osFlashWriteBuffer(&mb, 0, ram, &D_800E17A8_7E648);
        osRecvMesg(&D_800E17A8_7E648, NULL, 1);
        osFlashWriteArray(page);
        ram += 0x80;
        page++;
        size -= 0x80;
    }
}

void exp_apLoad(void) {
    exp_apFlashRead();
    if (gApData.magic != AP_MAGIC || gApData.checksum != exp_apChecksum()) {
        u8* p = (u8*) &gApData;
        s32 i;
        for (i = 0; i < (s32) sizeof(gApData); i++) {
            p[i] = 0;
        }
    }
}

void exp_apSave(void) {
    gApData.magic = AP_MAGIC;
    gApData.checksum = exp_apChecksum();
    exp_apFlashWrite();
}

// Wraps the main-save flash write (func_800C09C0): on success, also persist AP.
s32 exp_saveMain(uintptr_t addr, s32 size) {
    s32 ret = func_800C09C0_5D860(addr, size);
    if (ret == 0) {
        exp_apSave();
    }
    return ret;
}

// Wraps the main-save flash read (func_800C06A8): also load+validate AP.
s32 exp_loadMain(uintptr_t addr, s32 size) {
    func_800C06A8_5D548(addr, size);
    exp_apLoad();
    return 0;
}

s16 max(s16 a, s16 b) {
    return (a > b ? a : b);
}

// Wraps photocheck_oaksMark: after the photo scoring UI runs, 
// we record each species' best bonus values into the AP block.
//   Index 0: Special Score (e.g. 1000 for Surfing Pikachu)
//   Index 1: Pose Score
//   Index 2: Size Score
//   Index 3: Technique Score (0 = Normal, 1 = Wonderful)
//   Index 4: Same Pokemon Score
//   Index 5: Bit field containing special pose IDs
//     Special Pose IDs range from 1 to 12 (inclusive)  
//     and are mapped to bits 0 to 11 of index 5
//       1: Surfing Pikachu      7: Gust-Using Pidgey
//       2: Pikachu on a Ball    8: Jigglypuff on Stage
//       3: Balloon Pikachu      9: Graveler's Group Dance
//       4: Speed Pikachu       10: Rare Pokemon Mew
//       5: Pikachu on a Stump  11: Fighting Magmar
//       6: Flying Pikachu      12: Jigglypuff Trio on Stage
//   Index 6: Bit field containing level IDs of photos taken.
//     Some Pokemon, e.g. Magikarp, can be photographed on more
//     than one level. This bit field tracks photos taken, with the
//     following index-level correspondence.
//       0: Beach
//       1: Tunnel
//       2: Volcano
//       3: River
//       4: Cave
//       5: Valley
//       6: Rainbow Cloud
//   Index 7: Unused. Having 8 values makes debugging easier.
s32 exp_registerPhoto(Photo* photo) {
    // Run the UI first so that we don't spoil the results
    s32 ret = photocheck_oaksMark(photo);

    s32 isPokemonSign = !(photo->pkmnID < PokemonID_1004);
    s32 slot = func_8009BB4C(photo->pkmnID);

    if (0 <= slot && slot < (s32) ARRAY_COUNT(gApData.speciesScores)) {
        s16* score = gApData.speciesScores[slot];

        if (isPokemonSign) {
            // Pokemon Signs are in slots 63 - 68, in course order, i.e.
            //   Kingler Rock, Pinsir Shadow, Koffing Smoke,
            //   Cubone Tree, Mewtwo Constellation, Dugtrio Mountain
            // Sign photos are not scored, so we'll just set every nibble = 1.
            int i;
            for (i = 0; i < 8; i++) {
                score[i] = 0x1111;
            }
        } else {
            score[0] = max(score[0], photo->specialBonus);
            score[1] = max(score[1], photo->posePts);
            score[2] = max(score[2], photo->proximityScore);
            score[3] = max(score[3], photo->isWellFramed);
            score[4] = max(score[4], photo->samePkmnBonus);
            
            if (photo->specialID > 0) {
                score[5] |= (1 << (photo->specialID - 1));
            }

            score[6] |= (1 << photo->unk_0->levelID);
        }
    }

    return ret;
}
