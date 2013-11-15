.text
.globl main
main:
     addi $29, $0, 4
     addi $30, $0, 0x13DB
label: sw $30, 0($29)
     lbu $28, 1($29)
     jal label
     jr    $31

