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
   char str[] = " : Do you like this 159 guy?"; 
   int term;
   char get_str[100];
   term = (GetPid()%2==1)? TERM1 : TERM2 ;
   while(1) { 
	my_str[0] = GetPid() + '0';  // fill out 1st space
	PutStr(term, my_str);
	PutStr(term, str);

	GetStr(term, get_str, 100);
        PutStr(term, get_str);
	  
	PutStr(term, "\n\r");
	Sleep( GetPid() % 5 ); 
   }
}


void CookerProc(void) {   // will be created by pressing 'c' key
      int i;

      for(;;) {
         Mutex(LOCK);
         if(pies == 99) {
            cons_printf("+++++ Cooker %i: too many pies!\n", GetPid());
         } else {
            cons_printf("Cooker %i: making pie # %i...\n", GetPid(), ++pies);
            for(i=0; i<LOOP; i++) asm("inb $0x80");      // pie-making time
         }
         Mutex(UNLOCK);
      }
}
void EaterProc(void) {   // will be created by pressing 'e' key
      int i;

      for(;;) {
         Mutex(LOCK);
         if(pies == 0) {
            cons_printf("----- Eater %i: no pie to eat!\n", GetPid());
         } else {
            cons_printf("Eater %i: eating pie # %i...\n", GetPid(), pies--);
            for(i=0; i<LOOP; i++) asm("inb $0x80");
         }
         Mutex(UNLOCK);
      }   
}

