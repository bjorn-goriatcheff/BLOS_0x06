// syscalls.c
// API calls to kernel system services
#include "syscall.h"
#include "types.h"
//Mutex
void Mutex(int id, int lock) {
   asm("movl $102, %%EAX;
        movl %0, %%EBX;
	movl %1, %%ECX;
        int $128"
       :   
       : "g" (id), "g" (lock) 
       : "eax", "ebx", "ecx"
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

//PutChar
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
//PutStr
void PutStr(int fileno, char *p){	
	if(fileno==TERM1) Mutex(KB1, LOCK);
	else Mutex(KB2, LOCK);
	while(*p!='\0'){
		PutChar(fileno, *p);
		p++;
	}
	if(fileno==TERM1) Mutex(KB1, UNLOCK);
        else Mutex(KB2, UNLOCK);
}


//GetStr
void GetStr(int fileno, char *p, int size){	
	char ch;
	int i=0;
	if(fileno==TERM1) Mutex(SCREEN1, LOCK);
        else Mutex(SCREEN2, LOCK);
	for(i=0;i<size-1;i++){		
		ch=GetChar(fileno);
		if(ch == '\r') PutChar(fileno, '\n'); 
		PutChar(fileno, ch);
		if(ch== (char)10 || ch== (char)13){	
			 break;
		}
		p[i]=ch;

	}	
	p[i]='\0';		
	if(fileno==TERM1) Mutex(SCREEN1, UNLOCK);
        else Mutex(SCREEN2, UNLOCK);
}

//Fork
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
//Signal
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

//Exit
void Exit(int exit_num){
	asm("movl $1, %%EAX; // EXIT service
             movl %0, %%EBX; // EXIT NUM
	     int $128"
	     :
	     : "g" (exit_num)
	     : "eax", "ebx"

	);
}

//WaitPid 
int WaitPid(int *exit_num_p){
	int child_pid;
	asm("movl $7, %%EAX;
	     movl %1, %%EBX;
	     int $128;
	     movl %%ECX, %0"
	     : "=g" ((int)child_pid)
	     : "g" (exit_num_p)
	     : "eax", "ebx", "ecx"
	);
	return child_pid;
}

//Exec
void Exec(func_p_t p){
	asm("movl $11, %%EAX;
	     movl %0, %%EBX;
	     int $128"
	     :
	     : "g" ((int)p)
	     : "eax", "ebx"
	);
}

	
