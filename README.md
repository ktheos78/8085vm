# 8085vm v1.1

## About

This is an emulator for the Intel 8085 processor that can load and run 8085 Assembly programs from memory. Almost all of the functionality of the original processor has been ported, except for interrupts, the AC flag and conditional instructions based on the S and P flags. However, addresses `0xE000 - 0xFFFF` (8 KiB) are reserved for the stack. Additionally, addresses `0x2000` and `0x3000` function as standard input and standard output respectively.

To run the emulator and load a program into memory (address 0x0800), the following command is used in the shell: `./8085vm <file> [initial step delay]`, where `[initial step delay]` is the initial value in seconds for the delay between each instruction (0 by default).

Note that the program has to be a binary comprised of assembled bytecode. I've written an assembler for this purpose, which is available [here](https://github.com/ktheos78/asm8085). 

## Debugger

This emulator also comes with a basic debugger, with the following commands:

1. `help` - get help about a command  
    Usage: `help [command number]`

2. `dump` - dump CPU state (registers, flags, stdin, stdout)  
    Usage: `dump`

3. `info` - display register/flag/address contents  
    Usage: `info [r [register] | f [flag] | a <address>]`

4. `set` - change contents of memory address  
    Usage: `set <address> <value>`

5. `step` - change step delay between instructions  
    Usage: `step [seconds]`

6. `exit` - halt execution, dump CPU state and exit  
    Usage: `exit`