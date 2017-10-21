// events.h of events.S
// prototypes those coded in events.S

#ifndef _EVENTS_H_

#define _EVENTS_H_
#define TIMER_EVENTS_ 0x20
#define SYSCALL_EVENTS_ 0x80
#ifndef ASSEMBLER  // skip below if ASSEMBLER defined (from an assembly code)
                   // since below is not in assembler syntax
__BEGIN_DECLS



#include "types.h"                // proc_frame_t

void TimerEvent(void);            // coded in events.S, assembler won't like this syntax
void ProcLoader(proc_frame_t *);  // coded in events.S
void SyscallEvent(void);
void Term1Event(void);
void Term2Event(void);

__END_DECLS

#endif // ifndef ASSEMBLER

#endif // ifndef _EVENTS_H_

