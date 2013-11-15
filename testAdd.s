.text
.globl main
main:
     addi $31, $0, 1
     addi $30, $0, 0
     nor $28, $30, $31
     nor $29, $31, $31
     nor $27, $30, $30
     bne $30, $31, label
     addi $31, $0, 1
     addi $31, $0, 1
label:     addi $30, $0, 0
     jr    $31