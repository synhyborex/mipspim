.text
.globl main
main:
     addi $31, $0, 4
     addi $30, $0, 0x1A2B
     sw $30, 0($31)
     lbu $29, 1($31)
     jr    $31
