// syscalls.c
// API calls to kernel system services
#include "syscall.h"
// GetPid() call
int GetPid(void) {          // no input, has return
   int pid; 
   asm("pushl %%EAX;        // save register EAX to stack
        movl $100, %%EAX;   // service #100
        int $128;           // interrupt CPU with IDT Event 128
        movl %%EAX, %0;     // after, copy EAX to variable 'pid' (%0 means 1st item below)
        popl %%EAX"         // restore original EAX
       : "=g" (pid)         // output syntax, for output argument
       :                    // no input items
    ); 
   return pid;
}

// Write() call
void Write(int fileno, char *p) {
   asm("pushl %%EAX; //save registers
	pushl %%EBX;
	pushl %%ECX;
	movl $4, %%EAX; // service #101
	movl %0, %%EBX; //fileno
	movl %1, %%ECX;  //char
	int $128;
	popl %%ECX; //restore
	popl %%EBX; //restore
	popl %%EAX"
       :          
       : "g" (fileno), "g" ((int)p)
       );
}

// Sleep() call
void Sleep(int sec){
   asm("pushl %%EAX;
        pushl %%EBX
        movl $101, %%EAX;
	movl %0, %%EBX;
	int $128;
	popl %%EBX;
        popl %%EAX"
	:
	: "g" (sec)
	);
}

