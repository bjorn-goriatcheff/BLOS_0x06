// proc.h, 159

#ifndef _PROC_H_
#define _PROC_H_

void SystemProc(void);      // PID 0, never preempted
void UserProc(void);        // PID 1, 2, 3, ...
void CookerProc(void);
void EaterProc(void);

#endif
