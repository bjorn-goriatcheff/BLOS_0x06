// syscalls.h

#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

int GetPid(void);         // no input, 1 return
void Write(int, char*);
void Sleep(int);
void Mutex(int);
char GetChar(int fileno);
void PutChar(int fileno, char ch);
void PutStr(int fileno, char *p);
void GetStr(int fileno, char *p, int size);
#endif
