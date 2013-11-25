main:
        lui     $4, 0x1234
        ori     $4, 0xABCD #Load in some data
        addiu   $5, $0, 0 #Put the address into a register
        sw      $4, 0($5) #Store the data into the address
        lw      $6, 0($5) #Read out the entire word
        lbu     $7, 0($5) #Load the 0th byte
        lbu     $8, 1($5) #Load the 1st byte
        lbu     $9, 2($5) #Load the 2nd byte
        lbu     $10, 3($5) #Load the 3rd byte

        sb      $4, 4($5) #Store one byte at a time
        lw      $11, 4($5) #And check the full word
        sb      $4, 5($5)
        lw      $12, 4($5)
        sb      $4, 6($5)
        lw      $13, 4($5)
        sb      $4, 7($5)
        lw      $14, 4($5)
        jr      $ra

#The following is the register output I got on unix3:
#0
#0
#0
#0
#1234abcd
#0
#1234abcd
#12
#34
#ab
#cd
#cd000000
#cdcd0000
#cdcdcd00
#cdcdcdcd


