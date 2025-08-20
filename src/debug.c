#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "cpu.h"

#define NUM_CMDS 6
#define CHAR_DELIM " \t"
#define TOKEN_BUFFER_SIZE 64
#define MAX_BUFF_SIZE 256

pthread_mutex_t debug_mutex = PTHREAD_MUTEX_INITIALIZER;

char *get_input(void)
{
    char *line = NULL;
    size_t buffer_size = 0;

    if (getline(&line, &buffer_size, stdin) == -1)
    {
        if (feof(stdin))
            exit(0);

        perror("Read line");
        exit(1);
    }

    return line;
}

char **tokenize(char *user_input)
{
    char **res = (char **)malloc(TOKEN_BUFFER_SIZE * sizeof(char *));
    if (res == NULL)
    {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }

    char *token;
    int token_count = 0;

    token = strtok(user_input, CHAR_DELIM);
    while (token != NULL)
    {
        res[token_count] = token;
        token_count++;
        if (token_count >= TOKEN_BUFFER_SIZE)
        {
            fprintf(stderr, "Error: Command too long, maximum token size exceeded\n");
            return NULL;
        }
        token = strtok(NULL, CHAR_DELIM);
    }

    // null terminate tokenized string array
    res[token_count] = NULL;

    // remove newline from end of last arg
    (res[token_count - 1])[strlen(res[token_count - 1]) - 1] = '\0';

    res = (char **)realloc((void *)res, (token_count + 1) * sizeof(char *));
    if (res == NULL)
    {
        fprintf(stderr, "Error: realloc failed\n");
        exit(1);
    }

    return res;
}

int exec_cmd(char **argv)
{
    if (argv == NULL)
        return 1;

    // check if cmd exists
    for (int i = 0; i < NUM_CMDS; ++i)
    {
        if (strcmp(argv[0], cmd_names[i]) == 0)
            return (*cmd_funcs[i])(argv);
    }

    fprintf(stderr, "Error: unknown command. Type \"help\" for a list of all available commands\n");
    return 1;
}

void* debugger_loop(void *argv)
{
    int status;
    char *user_input, **cmd;

    do
    {
        printf("8085vm# ");

        user_input = get_input();
        cmd = tokenize(user_input);
        status = exec_cmd(cmd);

        free(user_input);
        free(cmd);
    }
    while (status);
}


int d_help(char **argv)
{
    int cmd_idx;

    if (argv[1] == NULL)
    {
        printf("Available commands:\n");
        for (int i = 1; i < NUM_CMDS; ++i)
            printf(" (%d) - %s\n", i, cmd_names[i - 1]);

        printf("\n");
        printf("Type \"help <command number>\" for help regarding a certain command.\n");
    }

    else
    {
        cmd_idx = (int)strtol(argv[1], NULL, 0);

        switch (cmd_idx)
        {
            // help
            case 1:
                printf("help [command-number] - display help message\n");
                break;

            // dump
            case 2:
                printf("dump - display register, flag, stdout contents\n");
                break;

            // info
            case 3:
                printf("info [r [register] | f [flag] | a <address>] - get reg/flag/addr info\n");
                break;

            // set
            case 4:
                printf("set <addr> <val> - write value into address\n");
                break;

            // step
            case 5:
                printf("step <seconds> - set sleep time between commands (0 by default)\n");
                break;

            // exit
            case 6:
                printf("exit - exits debugger\n");
                break;
        }
    }

    return 1;
}

int d_dump(char **argv)
{
    printf("\nRegister state:\n");
    printf("PC = 0x%04X\n", PC);
    printf("SP = 0x%04X\n", SP);
    printf("A = 0x%02X\n", regs[R_A]);
    printf("B = 0x%02X\n", regs[R_B]);
    printf("C = 0x%02X\n", regs[R_C]);
    printf("D = 0x%02X\n", regs[R_D]);
    printf("E = 0x%02X\n", regs[R_E]);
    printf("H = 0x%02X\n", regs[R_H]);
    printf("L = 0x%02X\n", regs[R_L]);
    printf("\nFlags:\n");
    printf("CY: %d\n", flags & FL_CY);
    printf("P:  %d\n", (flags & FL_P) >> 2);
    printf("AC: %d\n", (flags & FL_AC) >> 4);
    printf("Z:  %d\n", (flags & FL_Z) >> 6);
    printf("S:  %d\n", (flags & FL_S) >> 7);
    printf("\nPort 0x3000 (standard output): %02X\n", memory[0x3000]);
    printf("Port 0x2000 (standard input): %02X\n\n", memory[0x2000]);

    return 1;
}

// not too proud of this one
int d_info (char **argv)
{
    // info with no args = dump
    if (argv[1] == NULL)
        return d_dump(argv);

    switch (argv[1][0])
    {
        // registers
        case 'r':
            if (argv[2] == NULL)
            {
                printf("PC = 0x%04X\n", PC);
                printf("SP = 0x%04X\n", SP);
                printf("A = 0x%02X\n", regs[R_A]);
                printf("B = 0x%02X\n", regs[R_B]);
                printf("C = 0x%02X\n", regs[R_C]);
                printf("D = 0x%02X\n", regs[R_D]);
                printf("E = 0x%02X\n", regs[R_E]);
                printf("H = 0x%02X\n", regs[R_H]);
                printf("L = 0x%02X\n", regs[R_L]);
                break;
            }

            switch (argv[2][0])
            {
                case 'A':
                case 'a':
                    printf("A = 0x%02X\n", regs[R_A]);
                    break;

                case 'B':
                case 'b':
                    printf("B = 0x%02X\n", regs[R_B]);
                    break;

                case 'C':
                case 'c':
                    printf("C = 0x%02X\n", regs[R_C]);
                    break;

                case 'D':
                case 'd':
                    printf("D = 0x%02X\n", regs[R_D]);
                    break;
                
                case 'E':
                case 'e':
                    printf("E = 0x%02X\n", regs[R_E]);
                    break;

                case 'H':
                case 'h':
                    printf("H = 0x%02X\n", regs[R_H]);
                    break;

                case 'L':
                case 'l':
                    printf("L = 0x%02X\n", regs[R_L]);
                    break;

                default:
                    if (strcmp(argv[2], "pc") == 0 || strcmp(argv[2], "PC") == 0)
                    {
                        printf("PC = 0x%04X\n", PC);
                        break;
                    }

                    if (strcmp(argv[2], "SP") == 0 || strcmp(argv[2], "sp") == 0)
                    {
                        printf("SP = 0x%04X\n", SP);
                        break;
                    }

                    fprintf(stderr, "Error: invalid register\n");
                    break;
            }

            break;

        // flags
        case 'f':
            if (argv[2] == NULL)
            {
                printf("CY: %d\n", flags & FL_CY);
                printf("P:  %d\n", (flags & FL_P) >> 2);
                printf("AC: %d\n", (flags & FL_AC) >> 4);
                printf("Z:  %d\n", (flags & FL_Z) >> 6);
                printf("S:  %d\n", (flags & FL_S) >> 7);
                break;
            }

            uint8_t val;

            if (strcmp(argv[2], "CY") == 0 || strcmp(argv[2], "cy") == 0)
                val = flags & FL_CY;
            else if (strcmp(argv[2], "P") == 0 || strcmp(argv[2], "p") == 0)
                val = (flags & FL_P) >> 2;
            else if (strcmp(argv[2], "AC") == 0 || strcmp(argv[2], "ac") == 0)
                val = (flags & FL_AC) >> 4;
            else if (strcmp(argv[2], "Z") == 0 || strcmp(argv[2], "z") == 0)
                val = (flags & FL_Z) >> 6;
            else if (strcmp(argv[2], "S") == 0 || strcmp(argv[2], "s") == 0)
                val = (flags & FL_S) >> 7;
            else
            {
                fprintf(stderr, "Error: invalid flag\n");
                break;
            }

            printf("%s: %d\n", argv[2], val);
            break;

        // address
        case 'a':
            uint16_t addr = (uint16_t)strtol(argv[2], NULL, 16);
            printf("Address 0x%04X: 0x%02X\n", addr, memory[addr]);
            break;

        default:
            fprintf(stderr, "Error: invalid argument\n");
            break;
    }

    return 1;
}

int d_set(char **argv)
{
    uint16_t addr;
    uint8_t val;

    if (argv[1] == NULL || argv[2] == NULL)
    {
        fprintf(stderr, "Error: missing argument for set\n");
        return 1;
    }

    addr = (uint16_t)strtol(argv[1], NULL, 16);
    val = (uint8_t)strtol(argv[2], NULL, 16);

    // lock mutex to change value
    pthread_mutex_lock(&debug_mutex);
    memory[addr] = val;
    pthread_mutex_unlock(&debug_mutex);

    return 1;
}

int d_step(char **argv)
{
    if (argv[1] == NULL)
    {
        printf("Current step value (seconds): %d\n", step_sec);
        return 1;
    }

    int res = (int)strtol(argv[1], NULL, 0);
    res = (res > 0) ? res : 0;

    step_sec = res;
    return 1;
}

int d_exit(char **argv)
{
    // lock mutex to change value
    pthread_mutex_lock(&debug_mutex);
    running = 0;
    pthread_mutex_unlock(&debug_mutex);
    
    return 0;
}

char *cmd_names[] =
{
    "help",
    "dump",
    "info",
    "set",
    "step",
    "exit"
};

int (*cmd_funcs[]) (char **) =
{
    &d_help,
    &d_dump,
    &d_info,
    &d_set,
    &d_step,
    &d_exit
};