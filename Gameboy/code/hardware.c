#include "hardware.h"
#include "cartridge.h"
#include "interrupts.h"
#include "timers.h"

// Initial values at bootup for the hardware
// Check section 3.2, Description of Registers
void initializeHardware()
{
	PC.pair = 0x100;
	SP.pair = 0xFFFE;
	registerAF.pair = 0x01B0;
	registerBC.pair = 0x0013;
	registerDE.pair = 0x00D8;
	registerHL.pair = 0x014D;
	cpu[0xFF00] = 0xCF;
	cpu[0xFF05] = 0x00;
	cpu[0xFF06] = 0x00;
	cpu[0xFF07] = 0x00;
	cpu[0xFF10] = 0x80;
	cpu[0xFF11] = 0xBF;
	cpu[0xFF12] = 0xF3;
	cpu[0xFF14] = 0xBF;
	cpu[0xFF16] = 0x3F;
	cpu[0xFF17] = 0x00;
	cpu[0xFF19] = 0xBF;
	cpu[0xFF1A] = 0x7F;
	cpu[0xFF1B] = 0xFF;
	cpu[0xFF1C] = 0x9F;
	cpu[0xFF1E] = 0xBF;
	cpu[0xFF20] = 0xFF;
	cpu[0xFF21] = 0x00;
	cpu[0xFF22] = 0x00;
	cpu[0xFF23] = 0xBF;
	cpu[0xFF24] = 0x77;
	cpu[0xFF25] = 0xF3;
	cpu[0xFF26] = 0xF1;
	cpu[0xFF40] = 0x91;
	cpu[0xFF42] = 0x00;
	cpu[0xFF43] = 0x00;
	cpu[0xFF45] = 0x00;
	cpu[0xFF47] = 0xFC;
	cpu[0xFF48] = 0xFF;
	cpu[0xFF49] = 0xFF;
	cpu[0xFF4A] = 0x00;
	cpu[0xFF4B] = 0x00;
	cpu[0xFFFF] = 0x00;

	interrupt.master = 1;
	interrupt.enable = 0;
	interrupt.flags = 0;
	interrupt.timer = 0xFF;

	stopped = 0;

	keys.keys1.a = 1;
	keys.keys1.b = 1;
	keys.keys1.start = 1;
	keys.keys1.select = 1;

	keys.keys2.up = 1;
	keys.keys2.down = 1;
	keys.keys2.left = 1;
	keys.keys2.right = 1;

	mMBC.register0 = 0;
	mMBC.register2 = 0;
	mMBC.register3 = 0;

	mMBC.romBank = 0;
	mMBC.ramBank = 0;	
}

void setJoypad() {
	// TODO Document and fix this, it goes into readMemory
	if (!(cpu[0xFF00] & 0x20))
	{
		cpu[0xFF00] = (BYTE)(0xC0 | keys.keys1.a | keys.keys1.b << 1 | keys.keys1.select << 2 | keys.keys1.start << 3 | 0x10);
	}
	else if (!(cpu[0xFF00] & 0x10))
	{
		cpu[0xFF00] = (BYTE)(0xC0 | keys.keys2.right | keys.keys2.left << 1 | keys.keys2.up << 2 | keys.keys2.down << 3 | 0x20);
	}
	else if (!(cpu[0xFF00] & 0x30))
	{
		cpu[0xFF00] = 0xFF;
	}
	else cpu[0xFF00] = 0xCF;
}
