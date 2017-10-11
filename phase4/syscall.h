// syscalls.h

#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

int GetPid(void);         // no input, 1 return
void Write(int, char*);
void Sleep(int);
void Mutex(int);
#endif
