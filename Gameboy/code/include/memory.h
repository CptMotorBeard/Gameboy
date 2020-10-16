#ifndef MEMORY_H
#define MEMORY_H

#include "hardware.h"

BYTE readMemory(WORD);
void writeMemory(WORD, BYTE);
void pushStack(WORD);
WORD popStack(void);
#endif
