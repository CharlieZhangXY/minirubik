.equ BYTES, 65536

.text
.globl main
main:
    li t0, 0x20000000
    li t1, BYTES
    add t1, t0, t1
fill:
    sw t0, 0(t0)
    addi t0, t0, 4
    bltu t0, t1, fill

    li t2, 5000000
hold:
    addi t2, t2, -1
    bnez t2, hold
    li a7, 10
    ecall
