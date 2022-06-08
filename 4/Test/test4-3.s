.data
_prompt: .ascii "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
    li $v0, 4
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
    addi $sp, $sp,-4
    sw $fp, 0($sp)
    move $fp, $sp
    addi $sp, $sp, -8
    li $t1, 1
    li $t2, 0
    li $t3, 4
    mul $t2, $t2, $t3
    sub $t2, $fp, $t2
    sw $t1, 0($t2) #a[0]=1
    li $t2, 0
    li $t3, 4
    mul $t2, $t2, $t3
    add $t2, $fp, $t2
    lw $t4, 0($t2)
    add $t4, $t4,1 #t4 = a[0]+1
    li $t2, 1
    li $t3, 4
    mul $t2, $t2, $t3
    sub $t2, $fp, $t2
    sw $t4, 0($t2) #a[1]=t4
    li $t2, 1
    li $t3, 4
    mul $t2, $t2, $t3
    sub $t2, $fp, $t2
    lw $a0, 0($t2) #write(a[1])
    addi $sp, $sp, -4
    sw $ra 0($sp)
    jal write
    lw $ra 0($sp)
    addi $sp, $sp, 4
    move $v0, $0
    jr $ra

