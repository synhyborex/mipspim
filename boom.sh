#!/bin/bash

arg1=$1
arg2=$2
arg3=$3

export PATH=/home/clupo/mips/bin:${PATH}
mips-elf-gcc $arg1 -static -msoft-float -o $arg2
mips-elf-objdump -D $arg2 | gensimcode > $arg3
./mipsim -d -s -c 256 -f $arg3



