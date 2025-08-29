#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include "opcodes.h"
#include "cpu.h"
#include "debug.h"

uint8_t memory[MEMORY_MAX];
uint8_t regs[R_COUNT];
uint8_t flags = 0;
uint16_t PC = 0x0800;
uint16_t SP = 0xFFFF;

uint8_t opcode;
uint8_t running = 1;

// debug
int step_sec = 0;

void load_program(char *program_path)
{
    long filesize;
    uint16_t load_addr = 0x0800;

    FILE *f = fopen(program_path, "rb");
    if (f == NULL)
    {
        fprintf(stderr, "Error: cannot open program\n");
        exit(1);
    }

    // get filesize
    fseek(f, 0, SEEK_END);
    filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // ensure program doesn't cross into stack segment
    if (load_addr + filesize >= STACK_SEGMENT_START)
    {
        fprintf(stderr, "Error: program doesn't fit into memory\n");
        exit(1);
    }

    fread(&memory[load_addr], 1, filesize, f);
    fclose(f);
    PC = load_addr;
}

// program loop
void *run_prog(void *args)
{   
    char **argv = (char **)args;

    // load program into memory
    load_program(argv[1]);

    // main loop
    while (PC < STACK_SEGMENT_START && running)
    {
        opcode = memory[PC++];
        if (opcode_table[opcode] != NULL)
            opcode_table[opcode]();
        else
            printf("Unknown opcode %02X at %04X\n", opcode, PC - 1);

        if (step_sec)
            sleep(step_sec);
    }
}

int main(int argc, char **argv)
{
    pthread_t prog_thread, debug_thread;
    int ret_prog, ret_debug;

    printf("8085vm v1.0 by theos78\n");
    printf("Type \"help\" for a list of all available debugger commands\n");

    if (argv[1] == NULL)
    {
        fprintf(stderr, "Error: no file provided\n");
        fprintf(stderr, "Usage: %s <program> [initial step delay]\n", argv[0]);
        exit(1);
    }

    // initialization
    init_opcodes();
    memset(memory, 0, sizeof(memory));
    flags = 0;
    if (argv[2] != NULL)
        step_sec = (uint32_t)strtol(argv[2], NULL, 0);

    // spawn program and debugger thread
    ret_prog = pthread_create(&prog_thread, NULL, run_prog, (void *)argv);
    ret_debug = pthread_create(&debug_thread, NULL, debugger_loop, NULL);

    // wait until threads are done
    pthread_join(prog_thread, NULL);
    pthread_join(debug_thread, NULL);

    printf("Execution finished.\n");
    d_dump(NULL);
}