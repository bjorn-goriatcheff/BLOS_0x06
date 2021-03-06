// data.h, 159
// kernel data are all declared in main.c during bootstrap, but
// other kernel .c code must reference them as 'extern' (already declared)

#ifndef _DATA_H_                    // 'name-mangling' prevention
#define _DATA_H_                    // 'name-mangling' prevention

#include "types.h"                  // defines q_t, pcb_t, PROC_NUM, PROC_STACK_SIZE

extern int run_pid;                 // PID of current selected process to run, 0 means none
extern unsigned int timer_ticks;    //counter timer
extern q_t ready_q, run_q;          // ready and runable PID's
extern q_t terminal_buffer[2], terminal_wait_queue[2];
extern q_t term_kb_wait_q[2], term_screen_wait_q[2];
extern pcb_t pcb[PROC_NUM];         // 20 Process Control Blocks
extern char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // 20 process runtime stacks
extern mutex_t mutex[4];
extern int pies;
extern page_t page[PAGE_NUM];
extern int kernel_cr3;
#endif                              // endif of ifndef
