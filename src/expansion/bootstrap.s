# Resident bootstrap stub. Overwrites the two SP anti-tamper checks
# (check_sp_imem/check_sp_dmem, 0x60 bytes total) byte-for-byte so the
# surrounding resident layout does not shift. DMAs the expansion segment
# from the ROM tail into the Expansion Pak at 0x80400000, runs its init,
# and forces both SP-okay flags true (disabling the anti-tamper payload).

glabel check_sp_imem
    addiu  $sp, $sp, -0x18
    sw     $ra, 0x14($sp)
    lui    $a0, %hi(expansion_ROM_START)
    addiu  $a0, $a0, %lo(expansion_ROM_START)
    lui    $a2, %hi(expansion_ROM_END)
    addiu  $a2, $a2, %lo(expansion_ROM_END)
    subu   $a2, $a2, $a0
    jal    dmaReadRom
     lui   $a1, 0x8040
    jal    expansion_init
     nop
    addiu  $t8, $zero, 1
    lui    $t9, %hi(gSPImemOkay)
    sb     $t8, %lo(gSPImemOkay)($t9)
    lui    $t9, %hi(gSPDmemOkay)
    sb     $t8, %lo(gSPDmemOkay)($t9)
    lw     $ra, 0x14($sp)
    jr     $ra
     addiu $sp, $sp, 0x18

glabel check_sp_dmem
    jr     $ra
     nop
    nop
    nop
    nop
