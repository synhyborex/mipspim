.text
.globl main
main:
     addi $29, $0, 4
     addi $30, $0, 0x13DB
     label:     lbu $28, 0($29)
	 sw $30, 0($29)
	 addi $29, $0, 4
jalr $31, label
addi $29, $0, 4
     addi $30, $0, 0x13DB
    # jal label
     jr    $31

