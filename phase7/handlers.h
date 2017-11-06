// handlers.h, 159

#ifndef _HANDLERS_H_
#define _HANDLERS_H_

#include "types.h"   // need definition of 'func_p_t' below

void NewProcHandler(func_p_t p);
void TimerHandler(void);
void GetPidHandler(void);
void WriteHandler(void);
void SleepHandler(void);
void MutexLockHandler(void);
void MutexUnlockHandler(void);
void GetCharHandler(int fileno);
void PutCharHandler(int fileno);
void TermHandler(int port);
void ForkHandler(proc_frame_t *parent_frame_p);
#endif
