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

main:
    addi $sp, $sp, -4
    sw $ra,0($sp)
    jal read
    lw $ra 0($sp)
    addi $sp,$sp,4
    move $t2, $v0
    addi $t2, $t2, 1
    li $t1, 0
    bgt $t2, $t1,label1
    j label2
label1:
    li $a0, 1
    addi $sp, $sp, -4
    sw $ra,0($sp)
    jal write
    lw $ra 0($sp)
    addi $sp,$sp,4
    j label3
label2:
    li $t1,0
    blt $t2, $t1, label4
    j label5
label4:
    li $t1, 0
    li $t3, 0
    sub $t1, $t3, $t1
    move $a0, $t1
    addi $sp, $sp, -4
    sw $ra,0($sp)
    jal write
    lw $ra 0($sp)
    addi $sp,$sp,4
    j label6
label5:
    li $t1, 0
    move $a0, $t1
    addi $sp, $sp, -4
    sw $ra,0($sp)
    jal write
    lw $ra 0($sp)
    addi $sp,$sp,4
label6:
label3:
    move $v0,$0
    jr $ra

