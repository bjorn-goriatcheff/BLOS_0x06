// syscalls.h

#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_
#include "types.h"
int GetPid(void);         // no input, 1 return
void Write(int, char*);
void Sleep(int);
void Mutex(int);
char GetChar(int fileno);
void PutChar(int fileno, char ch);
void PutStr(int fileno, char *p);
void GetStr(int fileno, char *p, int size);
int Fork(void);
void Signal(int num, func_p_t addr);
int WaitPid(int *exit_num_p);
void Exit(int exit_num);
#endif
