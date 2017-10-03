// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "spede.h"      // cons_...() needs
#include "data.h"       // run_pid needed
#include "proc.h"       // prototypes of processes
#include "syscall.h"   // API of system service calls

void SystemProc(void) {
   while(1){
	asm("inb $0x80"); // do nothing for now
   }
}
void UserProc(void) {
   char my_str[] = "  ";  // 2 spaces 
   while(1) {
      my_str[0] = GetPid() + '0';  // fill out 1st space
      Write(STDOUT, my_str);       // STDOUT fileno == 1
      Sleep( GetPid() % 5 );       // sleep for a few seconds (PID 5?)
   }
}




