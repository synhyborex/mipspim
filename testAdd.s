.text
.globl main
main:
     addi $29, $0, 4
     addi $30, $0, 0x45DB
	 sw $30, 0($29)
	 lbu $28, 0($29)
     jr    $31

