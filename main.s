.text
.globl main
text_begin:
main:
    la sp, call_stack_end
    jal ra, parse_input
    bnez a0, invalid_input
    jal ra, rank_state
    jal ra, search
    bltz a0, no_solution
    la t0, solution_length
    sw a0, 0(t0)
    mv s1, a0
    la a0, length_message
    li a7, 4
    ecall
    mv a0, s1
    li a7, 1
    ecall
    la a0, solution_message
    li a7, 4
    ecall
    li s0, 0
print_moves:
    beq s0, s1, verify_solution
    la t0, path
    add t0, t0, s0
    lbu s2, 0(t0)
    slli t0, s2, 2
    la t1, move_names
    add t0, t0, t1
    lw a0, 0(t0)
    li a7, 4
    ecall
    li a0, 32
    li a7, 11
    ecall
    mv a0, s2
    jal ra, apply_move
    addi s0, s0, 1
    j print_moves
verify_solution:
    jal ra, rank_state
    or t0, a0, a1
    bnez t0, replay_failed
    la a0, passed_message
    li a7, 4
    ecall
    li a0, 0
    mv a1, s1
    li a4, 0x50415353
    j finish
invalid_input:
    li a0, 2
    j failure
no_solution:
    li a0, 3
    j failure
replay_failed:
    li a0, 4
failure:
    mv s0, a0
    la a0, failed_message
    li a7, 4
    ecall
    mv a0, s0
    li a1, -1
    li a4, 0
finish:
    la t0, text_end
    la t1, text_begin
    sub a5, t0, t1
    la t0, data_end
    la t1, data_begin
    sub a6, t0, t1
    addi a6, a6, 1
    li a7, 93
    ecall

# Validate the embedded input and decode its two seven-byte arrays.
parse_input:
    la t6, input
    la a2, cubie_p
    la a3, cubie_o
    li t0, 0
    li t1, 0
parse_p:
    add t2, t6, t0
    lbu t2, 0(t2)
    addi t2, t2, -49
    li t3, 7
    bgeu t2, t3, parse_bad
    li t3, 1
    sll t3, t3, t2
    and t4, t1, t3
    bnez t4, parse_bad
    or t1, t1, t3
    add t3, a2, t0
    sb t2, 0(t3)
    addi t0, t0, 1
    li t3, 7
    blt t0, t3, parse_p
    li t0, 0
    li t1, 0
    addi t6, t6, 7
parse_o:
    add t2, t6, t0
    lbu t2, 0(t2)
    addi t2, t2, -49
    li t3, 3
    bgeu t2, t3, parse_bad
    add t1, t1, t2
    add t3, a3, t0
    sb t2, 0(t3)
    addi t0, t0, 1
    li t3, 7
    blt t0, t3, parse_o
    lbu t2, 7(t6)
    bnez t2, parse_bad
    li t3, 3
parse_sum:
    blt t1, t3, parse_sum_done
    addi t1, t1, -3
    j parse_sum
parse_sum_done:
    bnez t1, parse_bad
    li a0, 0
    ret
parse_bad:
    li a0, 1
    ret

# Rank decoded cubies. Returns a0=P, a1=O; no multiply/divide helpers.
rank_state:
    la a2, cubie_p
    la a3, cubie_o
    li a4, 0
    li a5, 0
rank_p:
    add t0, a2, a5
    lbu t0, 0(t0)
    li t1, 0
    addi t2, a5, 1
rank_smaller:
    li t3, 7
    bge t2, t3, rank_product
    add t3, a2, t2
    lbu t3, 0(t3)
    sltu t3, t3, t0
    add t1, t1, t3
    addi t2, t2, 1
    j rank_smaller
rank_product:
    mv t2, a4
    li t3, 7
    sub t3, t3, a5
    li a4, 0
rank_multiply:
    add a4, a4, t2
    addi t3, t3, -1
    bnez t3, rank_multiply
    add a4, a4, t1
    addi a5, a5, 1
    li t3, 7
    blt a5, t3, rank_p
    li a1, 0
    li t0, 0
rank_o:
    add t1, a3, t0
    lbu t1, 0(t1)
    slli t2, a1, 1
    add a1, a1, t2
    add a1, a1, t1
    addi t0, t0, 1
    li t1, 6
    blt t0, t1, rank_o
    mv a0, a4
    ret

# Replay a move using the seven-cubie model, independently of rank tables.
apply_move:
    li t0, 3
    li t1, 0
move_face:
    blt a0, t0, move_face_ready
    addi a0, a0, -3
    addi t1, t1, 1
    j move_face
move_face_ready:
    addi a6, a0, 1
    slli t0, t1, 3
    sub t0, t0, t1
    la a2, sources
    add a2, a2, t0
    la a3, twists
    add a3, a3, t0
    la a4, cubie_p
    la a5, cubie_o
move_quarter:
    li t0, 0
move_cubie:
    add t1, a2, t0
    lbu t1, 0(t1)
    add t2, a4, t1
    lbu t2, 0(t2)
    la t3, temporary_p
    add t3, t3, t0
    sb t2, 0(t3)
    add t2, a5, t1
    lbu t2, 0(t2)
    add t3, a3, t0
    lbu t3, 0(t3)
    add t2, t2, t3
    addi t2, t2, -3
    bgez t2, twist_reduced
    addi t2, t2, 3
twist_reduced:
    la t3, temporary_o
    add t3, t3, t0
    sb t2, 0(t3)
    addi t0, t0, 1
    li t1, 7
    blt t0, t1, move_cubie
    li t0, 0
move_copy:
    la t1, temporary_p
    add t1, t1, t0
    lbu t2, 0(t1)
    add t1, a4, t0
    sb t2, 0(t1)
    la t1, temporary_o
    add t1, t1, t0
    lbu t2, 0(t1)
    add t1, a5, t0
    sb t2, 0(t1)
    addi t0, t0, 1
    li t1, 7
    blt t0, t1, move_copy
    addi a6, a6, -1
    bnez a6, move_quarter
    ret

.data
cubie_p: .zero 7
cubie_o: .zero 7
temporary_p: .zero 7
temporary_o: .zero 7
path: .zero 12
.align 2
root_p: .half 0
root_o: .half 0
solution_length: .word 0
frames: .zero 384
.align 4
call_stack: .zero 512
call_stack_end:
sources: .byte 1,4,2,0,3,5,6, 0,1,2,4,5,6,3, 0,2,5,3,1,4,6
twists: .byte 1,2,0,2,1,0,0, 0,0,0,1,2,1,2, 0,0,0,0,0,0,0
length_message: .asciz "length: "
solution_message: .asciz "\nsolution: "
passed_message: .asciz "\nPASS: replay reached solved\n"
failed_message: .asciz "FAIL\n"
name_r: .asciz "R"
name_r2: .asciz "R2"
name_ri: .asciz "R'"
name_b: .asciz "B"
name_b2: .asciz "B2"
name_bi: .asciz "B'"
name_d: .asciz "D"
name_d2: .asciz "D2"
name_di: .asciz "D'"
.align 2
move_names: .word name_r,name_r2,name_ri,name_b,name_b2,name_bi,name_d,name_d2,name_di
