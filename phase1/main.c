// main.c, 159
// OS bootstrap and kernel code for OS phase 1
//
// Team Name: BLOS_0x6 (Members: Bjorn Goriatcheff, Lawrence Ly)

#include "spede.h"      // given SPEDE stuff
#include "types.h"      // data types
#include "events.h"     // events for kernel to serve
#include "tools.h"      // small functions for handlers
#include "proc.h"       // process names such as SystemProc()
#include "handlers.h"   // handler code

// kernel data are all declared here:
int run_pid;            // currently running PID; if -1, none selected
q_t ready_q, run_q;     // processes ready to be created and runables
pcb_t pcb[PROC_NUM];    // Process Control Blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks


void ProcScheduler(void) {              // choose run_pid to load/run
   if(run_pid>0) return; // no need if PID is a user proc
   
   if(run_q.size==0) run_pid=0;
   else{
	run_pid=DeQ(&run_q);
   }

 //  accumulate its life_time by adding its run_time
   pcb[run_pid].life_time+=pcb[run_pid].run_time;   
//and then reset its run_time to zero 
   pcb[run_pid].run_time=0;
}

int main(void) {  // OS bootstraps
   int i;
   struct i386_gate *IDT_p; // DRAM location where IDT is

   run_pid=-1; // needs to find a runable PID
   //use your tool function MyBzero to clear the two queues
   MyBzero((char *)&run_q, sizeof(q_t));
   MyBzero((char *)&ready_q, sizeof(q_t));
   //enqueue 0~19 to ready_q (all PID's are ready)
   for(i=0; i<20;i++){
	EnQ(i, &ready_q);
   }
   //get the IDT_p (to point to/locate IDT, like in the lab exercise)
   IDT_p = get_idt_base();
   cons_printf("IDT located at DRAM addr %x (%d).\n", IDT_p);

   //fill out IDT entry #32 like in the timer lab exercise
   //set the PIC mask to open up for timer event signal (IRQ0) only
   fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0); 
   outportb(0x21, ~1); //PIC command

   //first process
   NewProcHandler(SystemProc);
   //call ProcScheduler() to select the run_pid
   ProcScheduler();
   //call ProcLoader with the proc_frame_p of the selected run_pid
   ProcLoader(pcb[run_pid].proc_frame_p);
   return 0; // compiler needs for syntax altho this statement is never exec
}

void Kernel(proc_frame_t *proc_frame_p) {   // kernel code runs (100 times/second)
   char key;

   //save the proc_frame_p to the PCB of run_pid
   pcb[run_pid].proc_frame_p = proc_frame_p;
   //call the timer even handler routine to handle the timer interrupt event
   TimerHandler();
   if (cons_kbhit()){
      key=cons_getchar();
      if (key=='n') NewProcHandler(UserProc);
      if (key=='b') breakpoint();
   }

   //call ProcScheduler() to select run_pid (if needed)
   ProcScheduler();
   ProcLoader(pcb[run_pid].proc_frame_p); // given the proc_frame_p of the run_pid
}

