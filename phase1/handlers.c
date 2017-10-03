// handlers.c, 159

#include "spede.h"
#include "types.h"
#include "data.h"
#include "tools.h"
#include "proc.h"
#include "handlers.h"

// to create process, 1st alloc PID, PCB, and process stack space
// build process frame, initialize PCB, record PID to run_q (if not 0)
void NewProcHandler(func_p_t p) {  // arg: where process code starts
   int pid;

   if (ready_q.size==0) { // may occur as too many processes been created
      cons_printf("Kernel Panic: cannot create more process!\n");
      breakpoint();                   // alternative: breakpoint() into GDB
   }

   pid=DeQ(&ready_q);
   //use tool function MyBzero to clear PCB and runtime stack
   MyBzero(proc_stack[pid], PROC_STACK_SIZE);
   MyBzero((char *)&pcb[pid], sizeof(pcb_t));
   //set state of process in PCB
   //state_t s = RUN;
   pcb[pid].state=RUN;
   //queue it (pid) to be run_q unless it's 0 (SystemProc)
   if(pid!=0){
	EnQ(pid, &run_q); 
   }
   //point proc_frame_p to into stack (to where best to place a process frame)
   pcb[pid].proc_frame_p=(proc_frame_t *)&proc_stack[pid][0];
   //fill out EFL with "EF_DEFAULT_VALUE|EF_INTR" // to enable intr!
   pcb[pid].proc_frame_p->EFL=EF_DEFAULT_VALUE|EF_INTR;
   //fill out EIP to p
   pcb[pid].proc_frame_p->EIP=(unsigned int)p;  
   //fill CS with the return from a get_cs() call */
   pcb[pid].proc_frame_p->CS=get_cs();
}

// count run_time of running process and preempt it if reaching time slice
void TimerHandler(void) {
   static unsigned int timer_tick=0;
   
   if (timer_tick++%75==0) cons_printf(".");

   //dismiss timer event (IRQ0)
   outportb(0x20, 0x60); //dismiss timer event (IRQ 0) 

   if (run_pid==0) return;

   pcb[run_pid].run_time++;
   if(pcb[run_pid].run_time==TIME_SLICE){
      //update/downgrade its state
      pcb[run_pid].state=READY;
      //queue it back to run queue
      EnQ(run_pid, &run_q);
      run_pid=-1;  // no process running anymore
   }
}

