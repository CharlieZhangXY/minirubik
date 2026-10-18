.text
# Leaf function: only caller-saved registers are modified.
render_cube:
    li t0, LED_MATRIX_0_WIDTH
    li t1, 35
    bne t0, t1, render_bad_size
    li t0, LED_MATRIX_0_HEIGHT
    li t1, 25
    bne t0, t1, render_bad_size
    li a0, LED_MATRIX_0_BASE
    li t0, LED_MATRIX_0_HEIGHT
render_clear_row:
    li t1, LED_MATRIX_0_WIDTH
render_clear_pixel:
    sw zero, 0(a0)
    addi a0, a0, 4
    addi t1, t1, -1
    bnez t1, render_clear_pixel
    addi t0, t0, -1
    bnez t0, render_clear_row

    li a0, LED_MATRIX_0_BASE
    li a1, LED_MATRIX_0_WIDTH
    slli a1, a1, 2
    la a2, cubie_p
    la a3, cubie_o
    la a4, render_palette
    la a5, render_corner_colors
    la a6, render_stickers
    li a7, 24
render_sticker:
    lbu t0, 0(a6)
    li t1, 0
    beqz t0, render_fixed_corner
    addi t0, t0, -1
    add t1, a3, t0
    lbu t1, 0(t1)
    add t0, a2, t0
    lbu t0, 0(t0)
    addi t0, t0, 1
render_fixed_corner:
    lbu t2, 1(a6)
    sub t2, t2, t1
    bgez t2, render_color_index
    addi t2, t2, 3
render_color_index:
    slli t1, t0, 1
    add t0, t0, t1
    add t0, t0, t2
    add t0, a5, t0
    lbu t2, 0(t0)
    slli t2, t2, 2
    add t2, a4, t2
    lw t2, 0(t2)

    lbu t0, 2(a6)
    slli t0, t0, 2
    add t0, a0, t0
    lbu t1, 3(a6)
    beqz t1, render_draw
render_find_row:
    add t0, t0, a1
    addi t1, t1, -1
    bnez t1, render_find_row
render_draw:
    li t1, 3
render_draw_row:
    li t3, 4
    mv t4, t0
render_draw_pixel:
    sw t2, 0(t4)
    addi t4, t4, 4
    addi t3, t3, -1
    bnez t3, render_draw_pixel
    add t0, t0, a1
    addi t1, t1, -1
    bnez t1, render_draw_row
    addi a6, a6, 4
    addi a7, a7, -1
    bnez a7, render_sticker

# Put a GUI breakpoint here to inspect each complete frame, then press Run.
frame_ready:
    li t0, FRAME_DELAY
    beqz t0, render_return
render_delay:
    addi t0, t0, -1
    bnez t0, render_delay
render_return:
    ret
render_bad_size:
    la a0, render_size_message
    li a7, 4
    ecall
    li a0, 5
    li a7, 93
    ecall

.data
.align 2
# U=white, F=green, R=red, B=blue, L=orange, D=yellow.
render_palette: .word 0xffffff,0x00bb40,0xff2020,0x2060ff,0xff8800,0xffff00
# Physical cubies 0..7; cyclic face order agrees with solver.c twist increments.
# 0:ULF 1:UFR 2:DRF 3:DFL 4:URB 5:DBR 6:DLB 7:UBL
render_corner_colors:
    .byte 0,4,1, 0,1,2, 5,2,1, 5,1,4
    .byte 0,2,3, 5,3,2, 5,4,3, 0,3,4
# Each entry: physical position, cyclic face slot, x, y (top-left pixel).
# U: 7 4 / 0 1
render_stickers:
    .byte 7,0,9,2, 4,0,13,2, 0,0,9,5, 1,0,13,5
# L: 7 0 / 6 3
    .byte 7,2,0,9, 0,1,4,9, 6,1,0,12, 3,2,4,12
# F: 0 1 / 3 2
    .byte 0,2,9,9, 1,1,13,9, 3,1,9,12, 2,2,13,12
# R: 1 4 / 2 5
    .byte 1,2,18,9, 4,1,22,9, 2,1,18,12, 5,2,22,12
# B: 4 7 / 5 6
    .byte 4,2,27,9, 7,1,31,9, 5,1,27,12, 6,2,31,12
# D: 3 2 / 6 5
    .byte 3,0,9,16, 2,0,13,16, 6,0,9,19, 5,0,13,19
render_size_message: .asciz "Set LED Matrix 0 to Width=35, Height=25.\n"
