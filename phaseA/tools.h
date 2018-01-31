// tools.h, 159

#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "types.h" // need definition of 'q_t' below

void MyBzero(char *, int);
void EnQ(int, q_t *);
int DeQ(q_t *);
int MyStrcmp(char* str, char* s);
void MyMemcpy(char *destination, char *source, int byte_size); 
#endif

