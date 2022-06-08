.data
_prompt: .ascii "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
    li $2, 4
    la $a0, _prompt
    syscall
    li $v0, 5
    syscall
    jr $ra
write:
    li $v0, 1
    syscall
    li $v0, 4
    la $a0, _ret
    syscall
    move $v0, $0
    jr $ra

# NORMAL INSTRUCTION 10
    addi $sp, $sp, -8
    li $8, 0
    addi $8, -1
    lw $11, t2
    li $9, 1
    li $10, 4
    mul $11,$9,$10
    lw $12, v0
    lw $13, t2
    lw $14, t3
    sub $14,$12,$13
    lw $15, v1
    sw $15,0($14)
    lw $16, v3
    lw $17, v1
    addi $16,$17,3
    lw $20, t7
    li $18, 0
    li $19, 4
    mul $20,$18,$19
    lw $21, v0
    lw $22, t7
    lw $23, t8
    sub $23,$21,$22
    lw $24, v3
    sw $24,0($23)
    lw $27, t10
    li $25, 0
    li $26, 4
    mul $27,$25,$26
    lw $28, v0
    lw $29, t10
    lw $30, t11
    sub $30,$28,$29
    lw $31, v4
    lw $31,0($30)
# NORMAL INSTRUCTION 14
