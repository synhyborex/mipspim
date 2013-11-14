.text
.globl main
main:
     addi $31, $0, 1
     addi $30, $0, 0
     nor $28, $30, $31
     nor $29, $31, $31
     nor $27, $30, $30
     jr    $31
