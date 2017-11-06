// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "spede.h"      // cons_...() needs
#include "data.h"       // run_pid needed
#include "proc.h"       // prototypes of processes
#include "syscall.h"   // API of system service calls
#include "tools.h"      //for MyStrcmp call

void SystemProc(void) {
   while(1){
	asm("inb $0x80"); // do nothing for now
   }
}
	
void ShellProc(void) {   // new user proc
	int term, my_pid, forked_pid;
	char my_str[] = " ";
	char my_msg[] = ": (BLOS_0x06) Shell>";
	char get_str[100];

	while(1) {
		my_pid=GetPid();
		my_str[0] = my_pid + '0';  // fill out 1st space
		term = (my_pid%2 == 1)? TERM1 : TERM2;
	
		PutStr(term, my_str);
         	PutStr(term, my_msg);
	
		GetStr(term, get_str, 100); // syscall will add null
		if(MyStrcmp(get_str, "fork") == 1) { //1 is the same
			forked_pid = Fork();
	
			if(forked_pid == -1) PutStr(term, "ShellProc: cannot fork!\n\r");	
         	}
		//Sleep( GetPid() % 5 );       // sleep for a few seconds (PID 5?)
      	}
}




