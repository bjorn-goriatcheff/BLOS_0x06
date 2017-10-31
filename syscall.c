// syscalls.c
// API calls to kernel system services
#include "syscall.h"

//Mutex
void Mutex(int lock) {
   asm("pushl %%EAX;
        pushl %%ECX;
        movl $102, %%EAX;
        movl %0, %%ECX;
        int $128;
        popl %%ECX;
        popl %%EAX"
       :   
       : "g" (lock)
    ); 

}

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
	movl $4, %%EAX; // service #4
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
//GetChar
char GetChar(int fileno){
    int ch;
    asm("pusha;
	movl $103, %%EAX; // service 103
	movl %1, %%EBX; //fileno
	int $128; 
	movl %%ECX, %0; //copy char to output ch
	popa"
	: "=g" ((int)ch)
	: "g" (fileno)
   );
   return (char)ch;
}

void PutChar(int fileno, char ch){
	asm("pusha;
	     movl $104, %%EAX;
	     movl %0, %%EBX;
	     movl %1, %%ECX;
	     int $128;
	     popa"
	     : 
             : "g" (fileno), "g" (ch)
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
	     : "=g" ((int)i)
             :
             : "eax", "ebx", "ecx","edx","esp","ebp","esi","edi"
	   );
	return i;

}	
