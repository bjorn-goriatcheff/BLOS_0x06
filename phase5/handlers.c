// handlers.c, 159

#include "spede.h"
#include "types.h"
#include "data.h"
#include "tools.h"
#include "proc.h"
#include "handlers.h"
#include "syscall.h"

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
   //MyBzero(proc_stack[pid], PROC_STACK_SIZE);
   MyBzero((char *)&pcb[pid], sizeof(pcb_t));
   //set state of process in PCB
   //state_t s = RUN;
   pcb[pid].state=RUN;
//queue it (pid) to be run_q unless it's 0 (SystemProc)
   if(pid!=0){
	EnQ(pid, &run_q); 
   }
   //point proc_frame_p to into stack (to where best to place a process frame)
   pcb[pid].proc_frame_p=(proc_frame_t *)&proc_stack[pid][PROC_STACK_SIZE-sizeof(proc_frame_t)];
   //fill out EFL with "EF_DEFAULT_VALUE|EF_INTR" // to enable intr!
   pcb[pid].proc_frame_p->EFL=EF_DEFAULT_VALUE|EF_INTR;
   //fill out EIP to p
   pcb[pid].proc_frame_p->EIP=(unsigned int)p;  
   //fill CS with the return from a get_cs() call */
   pcb[pid].proc_frame_p->CS=get_cs();
}

// count run_time of running process and preempt it if reaching time slice
void TimerHandler(void) {
   int i;
   timer_ticks++;
   for(i=0;i<PROC_NUM;i++){
	if(pcb[i].state==SLEEPING && pcb[i].wake_time==timer_ticks){
		EnQ(i, &run_q);
		pcb[i].state=READY;
	}
   }
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

void GetPidHandler(void){
	// The pid is given to the procframe
	pcb[run_pid].proc_frame_p->EAX=run_pid; 		
}

void WriteHandler(void){
	char* str;
	int j;
	str=(char *)pcb[run_pid].proc_frame_p->ECX;
	// check the default outpout (Linux bash)
	if(pcb[run_pid].proc_frame_p->EBX==STDOUT){
		cons_printf(str);
	}
	// Check if wrong EBX
	else if(pcb[run_pid].proc_frame_p->EBX!=0){
		while(*str){
			// Send char to TERM1 or TERM2
			outportb(pcb[run_pid].proc_frame_p->EBX + DATA, *str);
			// Wait for hardware
			for(j=0;j<50000;j++) asm("inb $0x80");
			// Next char
			str++;
		}
	}

}

void SleepHandler(void){
	// Compute the waiting time regarding the PID
	pcb[run_pid].wake_time=timer_ticks+100*pcb[run_pid].proc_frame_p->EBX;
	// Update process'state
	pcb[run_pid].state=SLEEPING;
	// Process Scheduel
	run_pid=-1;
}
void MutexLockHandler(void){
	// If the pies are already UNLOCK
	if(mutex.lock==UNLOCK) mutex.lock=LOCK;
	// The pies are lock -> go waiting
        else{
		// Add process to wait_q
		EnQ(run_pid, &mutex.wait_q);
		//Update process'state
		pcb[run_pid].state=WAIT;
		// Process Schedueler
		run_pid=-1;
	}
	
}
void MutexUnlockHandler(void){
	int pid;
	// The wait_q is empty -> leave the pies UNLOCK
	if(mutex.wait_q.size==0) mutex.lock=UNLOCK;
	// The wait_q is not empty
	else if(mutex.wait_q.size!=0) {
		//Descrease wait_q
		pid=DeQ(&mutex.wait_q);
		// Run the process
		EnQ(pid, &run_q);
		// Update the state
		pcb[pid].state=RUN;
	}	
}

void GetCharHandler(int fileno){ // GetChar() call, like MutexLock()
	int i;
	
	if(fileno == TERM1){
		i = 0;
	else{
		i = 1;
	}
}

void TermHandler(int port){ // IRQ3 or IRQ4, like MutexUnlock()
	int i;
	char ch;
	
	if(port == TERM1){
		i = 0;
	else{
		i = 1;
	}
}
