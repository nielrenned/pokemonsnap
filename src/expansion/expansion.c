#include "common.h"
#include "more_funcs/more_funcs.h"
#include "app_level/app_level.h"
#include "sys/render.h"

extern UnkBigBoy* D_800C21B0_5F050;
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

u32 gExpansionMagic = 0x534E4150;

// Maximum film count. Below 60; settable at runtime.
s32 gMaxFilm = 15;

// While set, every item is usable regardless of its canUse flag (testing
// default-on). Clear this once an external grantor drives the canUse flags.
s32 gCanUseOverride = 1;

void expansion_init(void) {
    gExpansionMagic = 0x4F4B4159;
}

// Drives the dash engine's availability (icon + R-trigger boost) off canUse,
// via the single checkPlayerFlag(DASH) call in initUI that builds ProgressFlags.
s32 exp_dashAvailable(void) {
    return gCanUseOverride || D_800C21B0_5F050->data.canUseDashEngine;
}

void exp_handleItemButtonsPress(GObj* obj) {
    if (!IsInputDisabled) {
        if ((gContInputPressedButtons & D_CBUTTONS) &&
            (gCanUseOverride || D_800C21B0_5F050->data.canUseFlute) && PressPokeFluteTimeout == 0) {
            LastItemId = ITEM_ID_POKEFLUTE;
            PressPokeFluteTimeout = 45;
            Icons_ProcessButtonPress(ITEM_ID_POKEFLUTE);
            Items_PlayPokeFlute();
        } else if (ThrowItemTimeout == 0) {
            if ((gContInputPressedButtons & B_BUTTON) &&
                (gCanUseOverride || D_800C21B0_5F050->data.canUsePesterBall)) {
                LastItemId = ITEM_ID_PESTER_BALL;
                ThrowItemTimeout = 45;
                Items_SpawnPesterBall(&gMainCamera->viewMtx.lookAt.eye, &PlayerVelocity);
                Icons_ProcessButtonPress(ITEM_ID_PESTER_BALL);
                Items_StopPokeFlute();
                Icons_ProcessButtonPress(-1);
                PressPokeFluteTimeout = 0;
            } else if ((gContInputPressedButtons & A_BUTTON) &&
                       (gCanUseOverride || D_800C21B0_5F050->data.canUseApple)) {
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
    s32 apple = gCanUseOverride || D_800C21B0_5F050->data.canUseApple;
    s32 pester = gCanUseOverride || D_800C21B0_5F050->data.canUsePesterBall;
    s32 flute = gCanUseOverride || D_800C21B0_5F050->data.canUseFlute;

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
extern void func_800C0314_5D1B4(s32, s32);

// Independent item awards + flute requires rank 5 (was dash engine).
// Each tier checks "don't already have it" in its own condition, so owning a
// higher-tier item falls through to lower tiers instead of locking them out.
void exp_awardItems(s32 score) {
    if (score >= 130000 && func_800BFC5C_5CAFC() >= 5 && checkPlayerFlag(PFID_HAS_FLUTE) == 0) {
        D_801F3E28_9A3898 = 0x200;
        func_800C0314_5D1B4(2, 1);
    } else if (score >= 72500 && func_800BFC5C_5CAFC() >= 3 && checkPlayerFlag(PFID_HAS_PESTER_BALL) == 0) {
        D_801F3E28_9A3898 = 0x100;
        func_800C0314_5D1B4(1, 1);
    } else if (score >= 24000 && func_800BFC5C_5CAFC() > 0 && checkPlayerFlag(PFID_HAS_APPLE) == 0) {
        D_801F3E28_9A3898 = 0x80;
        func_800C0314_5D1B4(0, 1);
    }
}

#pragma GLOBAL_ASM("src/expansion/award_detour.s")
