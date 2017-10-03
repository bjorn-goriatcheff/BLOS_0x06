// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "spede.h"      // cons_xxx below needs
#include "data.h"       // current_pid needed below
#include "proc.h"       // prototypes of processes

void SystemProc(void) {
   int i;

   while(1) {
      cons_printf("0 ");    // SystemProc has PID 0
      for(i=0; i<LOOP; i++){
	  asm("inb $0x80");  // to cause delay approx 1 second
      }
   }

}

void UserProc(void) {
   int i;

   while(1) {
      cons_printf("%d ", run_pid);
      for(i=0;i<LOOP;i++) asm("inb $0x80");  // to cause delay approx 1 second
   }
}
