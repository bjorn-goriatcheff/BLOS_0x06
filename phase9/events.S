// events.S, initial stage handling an event, build proc frame mainly

#include <spede/machine/asmacros.h> // do not use/include "spede.h"
#include "events.h"                 // for K_DATA, K_STACK_SIZE below

.comm k_stack, 8192, 1      // declare kernel stack space (8192 x 1 byte)
.text

// loads the trapframe of the selected process in order to run it
ENTRY(ProcLoader)
   //copy (movl) 4 bytes right above ESP into EAX
   movl 4(%esp), %eax
   //copy EAX to ESP (point ESP to proc frame space)
   movl %eax, %esp
   popa
   addl $4, %esp // skip event_type
   //pop-all
   
   //interrupt return
   iret

ENTRY(SyscallEvent)
   pushl $128
   jmp TestRest

// build proc frame, and go into kernel
ENTRY(TimerEvent)           // will first push EFL, CS, EIP (hardware/circuit)   
   pushl $32 
   jmp TestRest

// Term1Event
ENTRY(Term1Event)
   pushl $35
   jmp TestRest

ENTRY(Term2Event)
   pushl $36
   jmp TestRest

TestRest:
   //push-all 
   pusha
   //copy ESP to EBX
   movl %ESP, %EBX
   //clear direction flag in EFL
   cld
   movw $0x10, %ax          // set kernel data segment registers
   mov %ax, %ds             // set kernel data segment registers
   mov %ax, %es             // set kernel data segment registers
   leal k_stack+8192, %esp  // point ESP to kernel stack
   //push EBX there (into kernel stack)
   push %EBX
   //call the Kernel C code
   call CNAME(Kernel)  
