.text
# a0=P, a1=O; returns a0=length, or -1. Preserves s0-s11.
search:
    addi sp, sp, -48
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw s3, 12(sp)
    sw s4, 16(sp)
    sw s5, 20(sp)
    sw s6, 24(sp)
    sw s7, 28(sp)
    sw s8, 32(sp)
    sw s9, 36(sp)
    sw s10, 40(sp)
    sw s11, 44(sp)
    la t0, root_p
    sh a0, 0(t0)
    sh a1, 2(t0)
    la s10, p_distance
    la s11, o_distance
    add t0, s10, a0
    lbu a5, 0(t0)
    add t0, s11, a1
    lbu t1, 0(t0)
    bge a5, t1, search_bound
    mv a5, t1
search_bound:
    li t0, 11
    blt t0, a5, search_failed
    la t0, root_p
    lhu s0, 0(t0)
    lhu s1, 2(t0)
    or t0, s0, s1
    beqz t0, search_zero
    mv s2, s0
    mv s3, s1
    li s4, 0
    li s5, 0
    li s6, 0
    li s7, 3
    addi s8, a5, -1
    li s9, 0
    la a2, p_move
    la a3, o_move
    la a4, frames
    li a6, 10080
    li a7, 1458
    li t4, 3
search_next:
    beq s4, t4, search_backtrack
    bnez s5, search_generate
    beq s4, s7, search_skip_face
    mv s2, s0
    mv s3, s1
search_generate:
    mv t5, s4
    slli t0, s2, 1
    add t0, a2, t0
    lhu s2, 0(t0)
    slli t0, s3, 1
    add t0, a3, t0
    lhu s3, 0(t0)
    addi s6, s6, 1
    addi s5, s5, 1
    bne s5, t4, search_prune
    li s5, 0
    addi s4, s4, 1
    add a2, a2, a6
    add a3, a3, a7
search_prune:
    add t0, s10, s2
    lbu t0, 0(t0)
    blt s8, t0, search_next
    add t0, s11, s3
    lbu t0, 0(t0)
    blt s8, t0, search_next
    la t0, path
    add t0, t0, s9
    addi t1, s6, -1
    sb t1, 0(t0)
    or t0, s2, s3
    beqz t0, search_found
    sh s0, 0(a4)
    sh s1, 2(a4)
    sh s2, 4(a4)
    sh s3, 6(a4)
    sb s4, 8(a4)
    sb s5, 9(a4)
    sb s6, 10(a4)
    sb s7, 11(a4)
    sw a2, 12(a4)
    sw a3, 16(a4)
    addi a4, a4, 32
    mv s0, s2
    mv s1, s3
    li s4, 0
    li s5, 0
    li s6, 0
    mv s7, t5
    addi s8, s8, -1
    addi s9, s9, 1
    la a2, p_move
    la a3, o_move
    j search_next
search_skip_face:
    addi s4, s4, 1
    addi s6, s6, 3
    add a2, a2, a6
    add a3, a3, a7
    j search_next
search_backtrack:
    beqz s9, search_raise_bound
    addi s9, s9, -1
    addi s8, s8, 1
    addi a4, a4, -32
    lhu s0, 0(a4)
    lhu s1, 2(a4)
    lhu s2, 4(a4)
    lhu s3, 6(a4)
    lbu s4, 8(a4)
    lbu s5, 9(a4)
    lbu s6, 10(a4)
    lbu s7, 11(a4)
    lw a2, 12(a4)
    lw a3, 16(a4)
    j search_next
search_raise_bound:
    addi a5, a5, 1
    j search_bound
search_zero:
    li a0, 0
    j search_return
search_found:
    addi a0, s9, 1
    j search_return
search_failed:
    li a0, -1
search_return:
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw s3, 12(sp)
    lw s4, 16(sp)
    lw s5, 20(sp)
    lw s6, 24(sp)
    lw s7, 28(sp)
    lw s8, 32(sp)
    lw s9, 36(sp)
    lw s10, 40(sp)
    lw s11, 44(sp)
    addi sp, sp, 48
    ret
