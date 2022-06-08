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

# NORMAL INSTRUCTION 9
main:
    addi $sp,$sp,-4
    sw $fp 0($sp)
    move $fp, $sp
    addi $sp, $sp, -8
    li $8, 0
    addi $8, -1
    li $9, 1
    li $10, 4
    mul $11,$9,$10
    move $12,$fp
    sub $13,$12,$11
    sw $8,0($13)
    addi $12,$8,3
    addi $sp,$sp,-4
    sw $13,8($fp)
#save t3
    li $9, 0
    li $10, 4
    mul $13,$9,$10
    move $9,$fp
    sub $10,$9,$13
    sw $12,0($10)
    addi $sp,$sp,-4
    sw $10,12($fp)
#save t8
    li $9, 0
    li $10, 4
    mul $9,$9,$10
    addi $sp,$sp,-4
    sw $9,16($fp)
#save t12
    move $9,$fp
    lw $10,16($fp)
#load t12
    sub $9,$9,$10
    addi $sp,$sp,-4
    sw $9,20($fp)
#save t13
    lw $9,0($9)
    addi $sp,$sp,-4
    sw $9,24($fp)
#save t9
    sw $10,16($fp)
#save t12
    lw $9,24($fp)
#load t9
    li $9, 4
    mul $10,$9,$9
    addi $sp,$sp,-4
    sw $10,28($fp)
#save v4
    addi $sp,$sp,-4
    sw $10,32($fp)
#save v5
    lw $10,28($fp)
#load v4
    move $10, $10
    sw $10,28($fp)
#save v4
    sw $9,24($fp)
#save t9
    lw $9,32($fp)
#load v5
    li $10, 2
    div $9, $10
    mflo $8
    move $v0,$0
    jr $ra
# NORMAL INSTRUCTION 13
