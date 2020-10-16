#ifndef CPU_H
#define CPU_H

#include "opcodes.h"

// There are 256 opcodes for GB
struct opcode mOpcodes[256];

void cpuStep(void);

void DEBUG_CARTRIDGE(void);
#endif
