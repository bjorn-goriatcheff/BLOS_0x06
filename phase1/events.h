// events.h of events.S
// prototypes those coded in events.S

#ifndef _EVENTS_H_
#define _EVENTS_H_

#ifndef ASSEMBLER  // skip below if ASSEMBLER defined (from an assembly code)
                   // since below is not in assembler syntax
__BEGIN_DECLS

#include "types.h"                // proc_frame_t

void TimerEvent(void);            // coded in events.S, assembler won't like this syntax
void ProcLoader(proc_frame_t *);  // coded in events.S

__END_DECLS

#endif // ifndef ASSEMBLER

#endif // ifndef _EVENTS_H_

