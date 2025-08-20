# 8085vm v1.0

## About

This is an emulator for the Intel 8085 processor that can load and run 8085 Assembly programs from memory. Almost all of the functionality of the original processor has been ported, except for interrupts and the AC flag. However, addresses `0xE000 - 0xFFFF` (8 KiB) are reserved for the stack. Additionally, addresses `0x2000` and `0x3000` function as standard input and standard output respectively.

To run the emulator and load a program into memory, the following command is used in the shell: `./8085vm <file> [address] [initial step delay]`, where `[address]` is the 16-bit memory address where the program will start (`0x0900` by default), and `[initial step delay]` is the initial value for the delay between instruction (0 by default).

Note that the program has to be a binary comprised of assembled bytecode. A small python script to turn a (manually typed) Assembly program into such a binary has been included, along with an example program (perhaps in the future I will also write an assembler to avoid this tedious process).

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

## Additional notes

To run the emulator, run the script `run.sh`, which generates the program binary to be loaded, compiles the emulator and launches it with load address `0x0900` and initial step delay of 1 second.