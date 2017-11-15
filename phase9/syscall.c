// syscalls.c
// API calls to kernel system services
#include "syscall.h"
#include "types.h"
//Mutex
void Mutex(int lock) {
   asm("movl $102, %%EAX;
        movl %0, %%ECX;
        int $128"
       :   
       : "g" (lock)
       : "eax", "ecx"
    ); 
}

// GetPid() call
int GetPid(void) {          // no input, has return
   int pid; 
   asm("movl $100, %%EAX;   // service #100
        int $128;           // interrupt CPU with IDT Event 128
        movl %%EAX, %0"     // after, copy EAX to variable 'pid' (%0 means 1st item below)
       : "=g" (pid)         // output syntax, for output argument
       :                    // no input items
       : "eax"
    ); 
   return pid;
}

// Write() call
void Write(int fileno, char *p) {
   asm("movl $4, %%EAX; // service #4
	movl %0, %%EBX; //fileno
	movl %1, %%ECX;  //char
	int $128"
       :          
       : "g" (fileno), "g" ((int)p)
	:  "eax", "ebx", "ecx"
       );
}

// Sleep() call
void Sleep(int sec){
   
   asm("movl $101, %%EAX;
	movl %0, %%EBX;
	int $128"
	:
	: "g" (sec)
	: "eax", "ebx"
	);
  
}
//GetChar
char GetChar(int fileno){
    int ch;
    asm("movl $103, %%EAX; // service 103
	movl %1, %%EBX; //fileno
	int $128; 
	movl %%ECX, %0" //copy char to output ch
	: "=g" ((int)ch)
	: "g" (fileno)
        : "eax", "ebx", "ecx"
   );
   return (char)ch;
}

void PutChar(int fileno, char ch){
	asm("movl $104, %%EAX;
	     movl %0, %%EBX;
	     movl %1, %%ECX;
	     int $128"
	     : 
             : "g" (fileno), "g" (ch)
	     : "eax", "ebx", "ecx"
	);
}

void PutStr(int fileno, char *p){
	while(*p!='\0'){
		PutChar(fileno, *p);
		p++;
	}
}

void GetStr(int fileno, char *p, int size){
	char ch;
	int i=0;
	
	for(i=0;i<size-1;i++){		
		ch=GetChar(fileno);
		if(ch==(char)13 || ch==(char)13){
			 PutChar(fileno, ch);	
			 PutChar(fileno, '\n');
			 break;
		}
		p[i]=ch;
		PutChar(fileno, ch);
	}	
	p[i]='\0';	
}

int Fork(void){
	int i;
	asm("movl $2, %%EAX;
	     int $128;
	     movl %%EBX, %0"
	     : "=g" (i) 
             :
             : "eax", "ebx"
	   );
	return i;

}
void Signal(int num, func_p_t addr){
        asm("
             movl $48, %%EAX;
             movl %0, %%EBX;
             movl %1, %%ECX;
             int $128"
            :
            : "g" (num), "g" ((int)addr)
	    : "eax","ebx", "ecx"
        );
}


void Exit(int exit_num){
	asm("movl $1, %%EAX;
             movl %0, %%EBX;
	     int $128"
	     :
	     : "g" (exit_num)
	     : "eax", "ebx"

	);
}

int WaitPid(int *exit_num_p){
	int child_pid;
	asm("movl $1, %%EAX;
	     movl %1, %%EBX;
	     int $128;
	     movl %%ECX, %0"
	     : "=g" ((int)child_pid)
	     : "g" (exit_num_p)
	     : "eax", "ebx", "ecx"
	);
	return child_pid;
}
	
