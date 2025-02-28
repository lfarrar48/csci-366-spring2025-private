##################################################################
# LMC ASSEMBLY ASSIGNMENT
#
# Write a little man assembly program that asks for two numbers
# and prints the multiplication of them.  It should do so in the
# *minimum* number of instructions necessary to do so regardless
# of the order that the user enters the numbers in.  (Think about
# this!)
#
##################################################################
.text
main:
    li $v0, 4
    la $a0, prompt1
    syscall


    li $v0, 5
    syscall
    move $t0, $v0

    li $v0, 4
    la $a0, prompt2
    syscall

    li $v0, 5
    syscall
    move $t1, $v0

    mul $t2, $t0, $t1  # $t2 = $t0 * $t1

    li $v0, 4
    la $a0, result_msg
    syscall

    li $v0, 1
    move $a0, $t2
    syscall

    li $v0, 10
    syscall

.data
prompt1:    .asciiz "Enter first number: "
prompt2:    .asciiz "Enter second number: "
result_msg: .asciiz "Multiplication result: "
