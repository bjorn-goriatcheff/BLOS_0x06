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
   //new
   pid=DeQ(&ready_q);
   
   //use tool function MyBzero to clear PCB and runtime stack
   MyBzero(proc_stack[pid], PROC_STACK_SIZE);
   MyBzero((char *)&pcb[pid], sizeof(pcb_t));
   //set state of process in PCB
   //state_t s = RUN;
   pcb[pid].state=RUN;
   pcb[pid].cr3=kernel_cr3; 
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
		pcb[i].state=RUN;
	}
   }
   //dismiss timer event (IRQ0)
   outportb(0x20, 0x60); //dismiss timer event (IRQ 0) 

   if (run_pid==0) return;

   pcb[run_pid].run_time++;
   if(pcb[run_pid].run_time==TIME_SLICE){ 
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
			for(j=0;j<5000;j++) asm("inb $0x80");
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
void MutexLockHandler(int i){
	// If the pies are already UNLOCK
	if(mutex[i].lock==UNLOCK) mutex[i].lock=LOCK;
	// The pies are lock -> go waiting
        else{
		// Add process to wait_q
		EnQ(run_pid, &mutex[i].wait_q);
		//Update process'state
		pcb[run_pid].state=WAIT;
		// Process Schedueler
		run_pid=-1;
	}
	
}
void MutexUnlockHandler(int i){
	int pid;
	// The wait_q is empty -> leave the pies UNLOCK
	if(mutex[i].wait_q.size==0) mutex[i].lock=UNLOCK;
	// The wait_q is not empty
	else if(mutex[i].wait_q.size!=0) {
		//Descrease wait_q
		pid=DeQ(&mutex[i].wait_q);
		// Run the process
		EnQ(pid, &run_q);
		// Update the state
		pcb[pid].state=RUN;
	}	
}
void GetCharHandler(int fileno){
	int i;
	char ch;
	if(fileno==TERM1) i=0;
	else i=1;
	if(terminal_buffer[i].size>0){	
		ch=DeQ(&terminal_buffer[i]); // Give the char in the buffer to the proc_frame to finish GetChar process
		pcb[run_pid].proc_frame_p->ECX=ch;
	}
	// blocking the current process
	else{	
 		pcb[run_pid].state=WAIT;
		EnQ(run_pid, &term_kb_wait_q[i]);
		run_pid=-1;
	}
}

void PutCharHandler(int fileno){
	int i;
	char ch;

	if(fileno==TERM1) i=0; 
	else  i=1;
	// Read ch in ECX from PutChar call
	ch=pcb[run_pid].proc_frame_p->ECX;
	//Display char
	outportb(fileno + DATA, ch);
	// Block process
	pcb[run_pid].state=WAIT;
	EnQ(run_pid, &term_screen_wait_q[i]);
	run_pid=-1;
}

// TermHandler called when a key is pressed in TERM0 or TERM2
void TermHandler(int port){
	int i, pid, indicator;
	char ch;
	if(port==TERM1)i=0 ; 
	else i=1;
	//Dismiss Timer Event
        if(i==0) outportb(0x20, 0x63);
        if(i==1) outportb(0x20, 0x64);
	// Indicator : key pressed or char displayed
	indicator=inportb(port + IIR);
	if(indicator==IIR_RXRDY){  	// Key pressed	
		ch=inportb(port + DATA);		//Read Char
		if(term_kb_wait_q[i].size==0){		// if there is no waiting process
			EnQ(ch, &terminal_buffer[i]);	// put the char in the buffer
		}
	
		else{  	//There is a waiting process
			pid=DeQ(&term_kb_wait_q[i]);     //Get the pid of the first waiting process
			pcb[pid].state=RUN;		// state to RUN
			pcb[pid].proc_frame_p->ECX=ch;		//Give the char to the proc_frame
			EnQ(pid, &run_q);			// Update run_q
			if(ch==(char) 3 && pcb[pid].sigint_handler != (func_p_t)0) {//ctrl-c char and activated handler
                        	InsertWrapper(pid, pcb[pid].sigint_handler);	 //altered proc_frame		
                        }
		}
	}	
	else{    // Char displayed	
		if(term_screen_wait_q[i].size>0){
			pid=DeQ(&term_screen_wait_q[i]);
			pcb[pid].state=RUN;
			EnQ(pid, &run_q);			
		}
	}
}

void ForkHandler(proc_frame_t *parent_frame_p) {
	int child_pid, delta, *bp;
	proc_frame_t *child_frame_p;

	if(ready_q.size==0){
		 cons_printf("Kernel Panic: cannot create more process!\n");
	       	 parent_frame_p->EBX=-1;
		 return;
	}
	child_pid=DeQ(&ready_q);
	EnQ(child_pid, &run_q);
	MyBzero((char*)&pcb[child_pid], sizeof(pcb_t));	//zap PCB
	pcb[child_pid].state=RUN;	//update state
	pcb[child_pid].cr3=kernel_cr3;
	//mem copy
	pcb[child_pid].ppid=run_pid;
	pcb[child_pid].sigint_handler=pcb[run_pid].sigint_handler;
	MyMemcpy((char*)&proc_stack[child_pid],(char*)&proc_stack[run_pid], PROC_STACK_SIZE);
	delta = proc_stack[child_pid] - proc_stack[run_pid];	
	child_frame_p = (proc_frame_t *)((int)parent_frame_p + delta);
	pcb[child_pid].proc_frame_p=child_frame_p;	
	child_frame_p->ESP+=delta;
	child_frame_p->EBP+=delta;
	child_frame_p->ESI+=delta;
	child_frame_p->EDI+=delta;
	child_frame_p->EBX=0;
	
	parent_frame_p->EBX=child_pid;

	bp = (int*)child_frame_p->EBP;
	while(*bp!='\0'){
		*bp+=delta;
		bp=(int*)*bp;
	}
}

void SignalHandler(proc_frame_t* p){
        if(p->EBX==SIGINT) pcb[run_pid].sigint_handler=(func_p_t)p->ECX;
	if(p->EBX==SIGCHLD) pcb[run_pid].sigchld_handler=(func_p_t)p->ECX;
}


void InsertWrapper(int pid, func_p_t handler){
        int *p;
	proc_frame_t temp_frame;

	temp_frame=*pcb[pid].proc_frame_p;
        p = (int*)&pcb[pid].proc_frame_p->EFL;       
        *p = (int)handler;
      	p--; 
        *p=(int)temp_frame.EIP;
	pcb[pid].proc_frame_p = (proc_frame_t *)((int)pcb[pid].proc_frame_p - sizeof(int[2]));
	//MyMemcpy((char*)pcb[pid].proc_frame_p, (char*)&temp_frame, sizeof(proc_frame_t));		
	*pcb[pid].proc_frame_p = temp_frame;
	pcb[pid].proc_frame_p->EIP=(unsigned int)Wrapper; 	
}

void ExitHandler(proc_frame_t *p){
	int i,ppid, child_exit_num, *parent_exit_num_p;
	
	ppid=pcb[run_pid].ppid;
	//The parent is not waiting
	if(pcb[ppid].state!=WAITCHLD){
		pcb[run_pid].state=ZOMBIE;
		run_pid=-1;
		if(pcb[ppid].sigchld_handler!=0){
			InsertWrapper(ppid, pcb[ppid].sigchld_handler);
		}
	// the parent is waiting -> release
	} else{
		pcb[ppid].state=RUN;
		EnQ(ppid, &run_q);
		
		pcb[ppid].proc_frame_p->ECX=run_pid;	
		parent_exit_num_p=(int*)pcb[ppid].proc_frame_p->EBX;
		child_exit_num=p->EBX;
		*parent_exit_num_p=child_exit_num;
		
		//reclaim page
		for(i=0;i<PAGE_NUM;i++){
			if(page[i].r_pid==run_pid){
				page[i].r_pid=-1;
			}
		}
		//set cr3
		set_cr3(kernel_cr3);
		EnQ(run_pid, &ready_q);
		pcb[run_pid].state=READY;
		run_pid=-1;
	}

}

void WaitPidHandler(proc_frame_t *p){
	int i,child_pid, child_exit_num, *parent_exit_num_p;
	for(i=0; i<PROC_NUM;i++){
		if(pcb[i].state==ZOMBIE && pcb[i].ppid==run_pid){	
			child_pid=i;
			break;
		}
	}
	// Not found
	if(i==PROC_NUM){
		pcb[run_pid].state=WAITCHLD;
		run_pid=-1;
	// Found
	} else{
		
		p->ECX=child_pid;
		set_cr3(pcb[child_pid].cr3);
		child_exit_num=pcb[child_pid].proc_frame_p->EBX;
		set_cr3(pcb[run_pid].cr3);
		parent_exit_num_p=(int*)p->EBX;	
		*parent_exit_num_p=child_exit_num;
	
		//reclaim page
		for(i=0;i<PAGE_NUM;i++){
			if(page[i].r_pid==child_pid){
				page[i].r_pid=-1;
			}
		}
		EnQ(child_pid, &ready_q);
		pcb[child_pid].state=READY;
	}
}

void ExecHandler(proc_frame_t *p){
	int i,j;
	int index[5];
	j=0;
	for(i=0;i<PAGE_NUM;i++){				//Search an unused page
		if(page[i].r_pid==-1) index[j++]=i;
		if(j==5) break;	
	}
	if(j<5){
		 cons_printf("No more pages\n");  
		return;
	}
	for(j=0;j<5;j++){
	   page[index[j]].r_pid=run_pid;
	   MyBzero((char *)page[index[j]].addr, PAGE_SIZE);
	} 
	MyMemcpy((char *)page[index[0]].addr,(char *)p->EBX, PAGE_SIZE);	//Instruction copy to page0
	p->EIP=(unsigned int)0x40000000;		//Modify EIP to 1G virtual : 256= hex 40 
	*(proc_frame_t *)((int)page[index[1]].addr + PAGE_SIZE - sizeof(proc_frame_t))=*p;	// copy proc_frame to DRAM page1 stack
	
	//copy first four entries	
	MyMemcpy((char *)page[index[2]].addr,(char*)kernel_cr3, 4*sizeof(int));
	//copy 511 and 256
	page[index[2]].addr[256]=(int)page[index[3]].addr+3;   //ENTRY 256 main table + FLAG init
	page[index[2]].addr[511]=(int)page[index[4]].addr+3;	  //ENTRY 512 main table  + FLAG init
	page[index[3]].addr[0]=(int)page[index[0]].addr+3;		 //ENTRY 1 of subtable inst + FLAG init
	page[index[4]].addr[1023]=(int)page[index[1]].addr+3; // ENTRY 1023 of subtable stack + FLAG init
	//Give PCB main table adress
	pcb[run_pid].cr3=(int)page[index[2]].addr;	//for set_cr3()
	pcb[run_pid].proc_frame_p=(proc_frame_t *)(0x80000000-sizeof(proc_frame_t)); // 2G -1 proc_frame
	set_cr3(pcb[run_pid].cr3);
}


