.data
.align 2
temp: .word 0

.text
.globl main
main:
    la t0, temp
    li t1, 1000000
loop:
    sw t1, 0(t0)
    addi t1, t1, -1
    bnez t1, loop
    li a7, 10
    ecall
