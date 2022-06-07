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
# NORMAL INSTRUCTION 5
    lw $10, t2
    li $8, 1
    li $9, 4
    mul $10,$8,$9
    lw $11, t3
    lw $12, t2
#assign to val
    li $13, 31833232 #?
    addi $13, 3
    lw $16, t7
    li $14, 0
    li $15, 4
    mul $16,$14,$15
    lw $17, t8
    lw $18, t7
    lw $19, t8
    lw $20, v3
    move $19, $20
    lw $23, t10
    li $21, 0
    li $22, 4
    mul $23,$21,$22
    lw $24, t11
    lw $25, t10
    lw $26, v4
    lw $27, t11
    move $26, $27
# NORMAL INSTRUCTION 14
