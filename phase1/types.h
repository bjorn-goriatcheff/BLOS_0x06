// types.h, 159

#ifndef _TYPES_H_
#define _TYPES_H_

#define TIMER_EVENT 32       // IDT entry #32 has code addr for timer event (DOS IRQ0)

#define LOOP 1666666         // handly loop limit exec asm("inb $0x80");
#define TIME_SLICE 200       // max timer count, then rotate process
#define PROC_NUM 20          // max number of processes
#define Q_SIZE 20            // queuing capacity
#define PROC_STACK_SIZE 4096 // process runtime stack in bytes

typedef void (*func_p_t)(); // void-return function pointer type

typedef enum {READY, RUN} state_t;

typedef struct {
   unsigned int PUSHA[8];
   unsigned int EIP;
   unsigned int CS;
   unsigned int EFL;
} proc_frame_t;

typedef struct {
   state_t state;            // state of process
   int run_time;             // CPU runtime this time
   int life_time;            // total CPU runtime
   proc_frame_t *proc_frame_p; // points to saved process frame
} pcb_t;                     

typedef struct {             // generic queue type
   int size;                 // size is also where the tail is for new data
   int q[Q_SIZE];            // integers are queued in q[] array
} q_t;

#endif // _TYPES_H_
