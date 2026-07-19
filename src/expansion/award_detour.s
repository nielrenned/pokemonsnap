# Detour body for the photo-evaluation item-award block (func_801E2ED4_992944).
# Entered via `j` from offset +0x4A8 (after the sp38 gate), so $sp is still the
# parent frame. Reads sp48 (the report score) from 0x48($sp), runs the new
# independent/flute-rank-5 award logic, then jumps back to the block end.

glabel exp_awardDetour
    jal    exp_awardItems
     lw    $a0, 0x48($sp)
    .word  0x08078d26   /* j 0x801e3498 (block end) */
     nop
