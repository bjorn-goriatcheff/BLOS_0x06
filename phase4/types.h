// types.h, 159

#ifndef _TYPES_H_
#define _TYPES_H_

#define TIMER_EVENT 32       // IDT entry #32 has code addr for timer event (DOS IRQ0)
#define SYSCALL_EVENT 128    // IDT entry #128 has code addr for syscall event
#define LOOP 1666666         // handly loop limit exec asm("inb $0x80");
#define TIME_SLICE 200       // max timer count, then rotate process
#define PROC_NUM 20          // max number of processes
#define Q_SIZE 20            // queuing capacity
#define PROC_STACK_SIZE 4096 // process runtime stack in bytes
#define WRITE 4			   // syscall id
#define GETPID 100                // Syscall id
#define SLEEP 101		  // Sleep syscall event id
#define MUTEX 102		 // Mutex syscall event id
#define LOCK 0			//Mutex Lock Constant
#define UNLOCK 1		// Mutex Constant
#define STDOUT 1		// Constant cons_printf call
#define TERM1 0x2f8           //address of terminal1 
#define TERM2 0x3e8		// address of terminal 2
typedef void (*func_p_t)(); // void-return function pointer type

typedef enum {READY, RUN, SLEEPING, WAIT} state_t;

typedef struct {
   unsigned int FOUR[4];
   unsigned int EBX;
   unsigned int EDX;
   unsigned int ECX;
   unsigned int EAX;
   unsigned int event_type;
   unsigned int EIP;
   unsigned int CS;
   unsigned int EFL;
} proc_frame_t;

typedef struct {
   state_t state;            // state of process
   int run_time;             // CPU runtime this time
   int life_time;            // total CPU runtime
   int wake_time;
   proc_frame_t *proc_frame_p; // points to saved process frame
} pcb_t;                     

typedef struct {             // generic queue type
   int size;                 // size is also where the tail is for new data
   int q[Q_SIZE];            // integers are queued in q[] array
} q_t;

typedef struct {
   int lock;
   q_t wait_q;
} mutex_t;

#endif // _TYPES_H_
