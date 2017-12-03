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

void Aout(void){
	int my_pid, term;
	int sec;
	//Get the pid of the process	
	my_pid=GetPid();
	// compute waiting time
	sec=my_pid%5;
	//choose terminal
	term = (my_pid%2 == 1)? TERM1 : TERM2;
	PutStr(term, "Aout");
	if(sec==0)sec=5;
	Sleep(sec);
	Exit(my_pid * 100 );
}
	
void ShellProc(void) {   // new user proc
	int term, my_pid, forked_pid;
	char my_str[] = " ";
	char my_msg[] = ": (BLOS_0x06) Shell>";
	char get_str[100];
	
	Signal(SIGINT, Ouch);
	
		
	while(1) {
		my_pid=GetPid();
		my_str[0] = my_pid + '0';  // fill out 1st space
		term = (my_pid%2 == 1)? TERM1 : TERM2;
	
		PutStr(term, my_str);
         	PutStr(term, my_msg);
	
		GetStr(term, get_str, 100); // syscall will add null
		//FORK CALL
		if(MyStrcmp(get_str, "fork") == 1) { //1 is the same
			forked_pid = Fork();
	
			if(forked_pid == -1) PutStr(term, "ShellProc: cannot fork!\n\r");	
			if(forked_pid > 0) CallWaitPidNow(); // parent waits, this will block	
         	}//FORK BACKGROUND
		else if(MyStrcmp(get_str, "fork&") == 1 || MyStrcmp(get_str, "fork &") == 1) {  // background runner
			Signal(SIGCHLD, CallWaitPidNow); // register handler before fork!
            		forked_pid = Fork();    // since child may run so fast & exit 1st

            		if(forked_pid == -1) {
               			PutStr(term, "ShellProc: cannot fork!\n\r");
               			Signal(SIGCHLD, (func_p_t)0); // cancel handler!
            		}
		}//EXIT 
		else if(MyStrcmp(get_str, "exit") == 1) { // only try on child process
          		  Exit(my_pid * 100);  // what if parent exits? Then child exits?
         	}//AOUT 
		else if(MyStrcmp(get_str, "a.out") == 1){
			forked_pid = Fork();
			if(forked_pid == -1) PutStr(term, "ShellProc: cannot fork!\n\r");
			if(forked_pid == 0){
				Exec(Aout);
			}
			if(forked_pid > 0) CallWaitPidNow(); // parent waits, this will block

		}//AOUT BACKGROUND 
		else if(MyStrcmp(get_str, "a.out&") == 1 || MyStrcmp(get_str, "a.out &") == 1){
			Signal(SIGCHLD, CallWaitPidNow); // register handler before fork!
			forked_pid = Fork();    // since child may run so fast & exit 1st
			if(forked_pid == -1) {
                                PutStr(term, "ShellProc: cannot fork!\n\r");
                                Signal(SIGCHLD, (func_p_t)0); // cancel handler!
                        }
			if(forked_pid == 0){
                                Exec(Aout);
                        }
		}//LS
		else if(MyStrcmp(get_str, "ls") == 1) PutStr(term, "./\n\r../\n\ra.out-rxrw 7217 June 6 1992 9:17\n\r");
		//CLEAR
		else if(MyStrcmp(get_str, "clear") == 1) PutStr(term, "\e[2J");
		
		//Sleep( GetPid() % 5 );       // sleep for a few seconds (PID 5?)
      	}
}

void CallWaitPidNow(void) {    // ShellProc's SIGCHLD handler
      int my_pid, term, child_pid, exit_num;
      char my_msg[100];

      child_pid = WaitPid(&exit_num);
      sprintf(my_msg, "Child %d exited, exit # %d.\n\r",
         child_pid, exit_num);

      my_pid = GetPid();
      term = (my_pid%2 == 1)? TERM1 : TERM2;  // which term to use
      PutStr(term, my_msg);
}

void Wrapper(func_p_t sig_handler){
        asm("pusha");
        sig_handler();
        asm("popa");		
	//asm("ret");
}

void Ouch(void){
        int term = (GetPid()%2)? TERM1 : TERM2; // terminal ?
        PutStr(term, "Ouch, that stings, Cowboy! "); // msg
}

