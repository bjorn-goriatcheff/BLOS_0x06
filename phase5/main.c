// main.c, 159
// OS bootstrap and kernel code for OS phase 5
//
// Team Name: BLOS_0x6 (Members: Bjorn Goriatcheff && Lawrence Ly)

#include "spede.h"      // given SPEDE stuff
#include "types.h"      // data types
#include "events.h"     // events for kernel to serve
#include "tools.h"      // small functions for handlers
#include "proc.h"       // process names such as SystemProc()
#include "handlers.h"   // handler code


// kernel data are all declared here:
int run_pid;            // currently running PID; if -1, none selected
unsigned int timer_ticks;
q_t ready_q, run_q;     // processes ready to be created and runables
pcb_t pcb[PROC_NUM];    // Process Control Blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks
mutex_t mutex;
int pies;
int port;

void InitTerms(int port) {
   int i;

// set baud, Control Format Control Register 7-E-1 (data- parity-stop bits)
// // raise DTR, RTS of the serial port to start read/write
// Terminal 1
   outportb(TERM1 + CFCR, CFCR_DLAB);             // CFCR_DLAB is 0x80
   outportb(TERM1 + BAUDLO, LOBYTE(115200/9600)); // period of each of 9600 bauds
   outportb(TERM1 + BAUDHI, HIBYTE(115200/9600));
   outportb(TERM1 + CFCR, CFCR_PEVEN | CFCR_PENAB | CFCR_7BITS);
   outportb(TERM1 + IER, 0);
   outportb(TERM1 + MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   outportb(TERM1 + IER, IER_ERXRDY);
   for(i=0;i<LOOP;i++) asm("inb $0x80");               // terminal H/W reset time
   //Terminal 2
   outportb(TERM2 + CFCR, CFCR_DLAB);             // CFCR_DLAB is 0x80
   outportb(TERM2 + BAUDLO, LOBYTE(115200/9600)); // period of each of 9600 bauds
   outportb(TERM2 + BAUDHI, HIBYTE(115200/9600));
   outportb(TERM2 + CFCR, CFCR_PEVEN | CFCR_PENAB | CFCR_7BITS);
   outportb(TERM2 + IER, 0);
   outportb(TERM2 + MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   outportb(TERM2 + IER, IER_ERXRDY);
   for(i=0;i<LOOP;i++) asm("inb $0x80");               // terminal H/W reset time

}

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
   struct i386_gate *IDT_p; // DRAM location where IDT is
   int i;   
   run_pid=-1; // needs to find a runable PID
   timer_ticks=0; //inuit timer_ticks
   pies=0;
   
   //use your tool function MyBzero to clear the two queues
   MyBzero((char *)&mutex.wait_q, sizeof(q_t));
   MyBzero((char *)&run_q, sizeof(q_t));
   MyBzero((char *)&ready_q, sizeof(q_t));
   //enqueue 0~19 to ready_q (all PID's are ready)
   for(i=0; i<20;i++){
	EnQ(i, &ready_q);
   }
   mutex.lock=UNLOCK;
   //get the IDT_p (to point to/locate IDT, like in the lab exercise)
   IDT_p = get_idt_base();
   cons_printf("IDT located at DRAM addr %x (%d).\n", IDT_p);

   //fill out IDT entry #32 like in the timer lab exercise
   //set the PIC mask to open up for timer event signal (IRQ0) only
   fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0); 
   fill_gate(&IDT_p[SYSCALL_EVENT], (int)SyscallEvent, get_cs(), ACC_INTR_GATE, 0);
   outportb(0x21, ~1); //PIC command
   InitTerms(port);
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

   //reading proc_frame_p event_type
   if(proc_frame_p->event_type==TIMER_EVENT) TimerHandler();   //call the timer even handler routine to handle the timer interrupt event 
   if(proc_frame_p->event_type==SYSCALL_EVENT){ // call Service
	//reading EAX value
	if(proc_frame_p->EAX==100) GetPidHandler();
	if(proc_frame_p->EAX==4) WriteHandler();
	if(proc_frame_p->EAX==101) SleepHandler(); 
        if(proc_frame_p->EAX==102){
		if (proc_frame_p->ECX==LOCK) MutexLockHandler();
		if (proc_frame_p->ECX==UNLOCK) MutexUnlockHandler();
	}
   } 
   if (cons_kbhit()){
      key=cons_getchar();
      if (key=='n') NewProcHandler(UserProc);
      if (key=='b') breakpoint();
      if (key=='c') NewProcHandler(CookerProc);
      if (key=='e') NewProcHandler(EaterProc);

   }

   //call ProcScheduler() to select run_pid (if needed)
   ProcScheduler();
   ProcLoader(pcb[run_pid].proc_frame_p); // given the proc_frame_p of the run_pid
}

