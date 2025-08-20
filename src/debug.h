#ifndef DEBUG_H_
#define DEBUG_H_

#include "cpu.h"
#include <stdint.h>

extern int step_sec;
extern char* cmd_names[];
extern int (*cmd_funcs[]) (char **);
extern uint8_t running;
extern uint16_t PC, SP;

void *debugger_loop(void *argv);
int d_help (char **argv);
int d_dump(char **argv);
int d_info(char **argv);
int d_set(char **argv);
int d_step(char **argv);
int d_exit(char **argv);

#endif /* DEBUG_H_ */