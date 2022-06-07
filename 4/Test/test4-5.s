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
main:
    li $8, 1
    addi $8, 2
    li $9, 2
    addi $9, 1
    li $10, 2
    addi $10, -1
    li $11, 2
    addi $11, -2
    lw $14, v8
    li $12, 1
    li $13, 2
    mul $14,$12,$13
    lw $17, v9
    li $15, 3
    li $16, 2
    mul $17,$15,$16
    lw $19, v8
    lw $20, v9
    li $18, 2
    div $18, $20
    mflo $19
    lw $22, v10
    lw $23, v9
    li $21, 3
    div $21, $23
    mflo $22
# NORMAL INSTRUCTION 14
    move $v0, $0
    jr $ra
