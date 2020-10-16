#include "hardware.h"
#include "cartridge.h"
#include "timers.h"
#include "interrupts.h"
#include <stdio.h>

BYTE readMemory(WORD address)
{
	/* ------ MEMORY BANK CONTROLLER ADDRESSES ------ */

	// Address 0x0000 to 0x3FFF is always ROM Bank #0
	if ((address >= 0x0000) && (address < 0x4000))
	{
		return mCartridge[address];
	}
	// Address 0x4000 - 0x7FFF is the switchable ROM bank
	else if ((address >= 0x4000) && (address < 0x8000))
	{
		unsigned long targetAddress = address - 0x4000;	// This will bring our address between 0x0000 and 0x4000

		unsigned long cartridgeAddress = (mMBC.romBank * ROM_BANK_SIZE) + targetAddress;	// 0x4000 addresses per ROM bank

		return mCartridge[cartridgeAddress];
	}
	else if ((address >= 0xA000) && (address < 0xC000))
	{
		unsigned long targetAddress = address - 0xA000;
		unsigned long ramAddress = (mMBC.ramBank * RAM_BANK_SIZE) + targetAddress;

		return mExtRAM[ramAddress];
	}
	else if ((address >= 0xE000) && (address < 0xFE00))
	{
		// duplicate from ext RAM
		return readMemory(address - 0x2000);
	}
	// TODO - more for 0xFF00 to fix input

	/* ------ CPU ADDRESSES ------ */
	else
	{
		return cpu[address];
	}
}

void writeMemory(WORD address, BYTE data)
{
	/* ------ MEMORY BANK CONTROLLER ADDRESSES ------ */

	/*
		Register 0	(RAMCS gate data - serves as write-protection for RAM):
		Write Area: 0x0000 - 0x1FFF
		Write Data: 0x0A
		Writing 0x0A to the write area causes the CS to be output, allowing access to RAM
	*/
	if ((address >= 0x0000) && (address < 0x2000))
	{
		mMBC.register0 = (data & 0x0A);
	}

	/*
		Register 1	(ROM bank code):
		Write Area: 0x2000 - 0x3FFF
		Write Data: 0x01 - 0x1F
		Allows the ROM bank to be selected in 16 kb increments
	*/
	else if ((address >= 0x2000) && (address < 0x4000))
	{
		// A value of 0 is seen as a 1
		if (data == 0x00)
		{
			data = 0x01;
		}

		if (data < 0x01 || data > 0x1F)
		{
			return;
		}

		mMBC.romBank &= 0xE0;	// clear the lower 5 bits of the rom bank
		mMBC.romBank |= data;	// set the lower 5 bits of the rom bank
	}

	/*
		Register 2	(Upper ROM bank code when using 1 mb or more of ROM and register 3 is 0):
		Write Area: 0x4000 - 0x5FFF
		Write Data: 0x00 - 0x03
		The upper ROM banks can be selected in 512 kb increments:
			0 - banks 0x01 - 0x1f
			1 - banks 0x21 - 0x3F
			2 - banks 0x41 - 0x5F
			3 - banks 0x61 - 0x7F

		Register 2	(Upper ROM bank code when using 1 mb or more of ROM and register 3 is 1):
		Write Area: 0x4000 - 0x5FFF
		Write Data: 0x00 - 0x03
		The RAM bank can be selected in 8 kb increments.
	*/

	else if ((address >= 0x4000) && (address < 0x6000))
	{
		if (!mMBC.register3)
		{
			mMBC.romBank &= 0x1F;	// Clear the upper bits of the rom bank
			mMBC.romBank |= data << 5;
		}
		else
		{
			if (data > 0x03)
			{
				return;
			}

			mMBC.ramBank = data;
		}
	}

	/*
		Register 3	(ROM / RAM change)
		Write Area: 0x6000 - 0x7FFF
		Write Data: 0x00 - 0x01
		Writing 0 causes register 2 output to control switching the higher ROM bank
		Writing 1 causes register 2 output to control switching of the RAM bank
	*/

	else if ((address >= 0x6000) && (address < 0x8000))
	{
		mMBC.register3 = data;
	}


	/* ------ REGULAR ADDRESSES ------ */

	// Read only memory areas
	else if ((address >= 0xFEA0) && (address < 0xFEFF))
	{
		return;
	}

	// switchable RAM addresses
	else if ((address >= 0xA000) && (address < 0xC000))
	{
		unsigned long targetAddress = address - 0xA000;
		unsigned long ramAddress = (mMBC.ramBank * RAM_BANK_SIZE) + targetAddress;

		mExtRAM[ramAddress] = data;
	}
	// TODO 0x8000 - 0x9FFF can only be accessed when FF41 is set to the correct mode
	// 0xE000 - 0xFE00 also writes to RAM
	else if ((address >= 0xE000) && (address < 0xFE00))
	{
		// Write to memory
		cpu[address] = data;
		// duplicate to regular RAM
		writeMemory(address - 0x2000, data);
	}
	// TODO 0xFE00 - 0xFE9F can only be accessed when FF41 is set to the correct mode
	// Scanline resets if written to
	else if (address == 0xFF44)
	{
		cpu[address] = 0;
	}
	// OAM
	else if (address == 0xFF46)
	{
		// Writing to FF46 (DMA register) initiates a transfer from RAM to OAM
		WORD from = data << 8;

		int i;
		WORD oamAddress;
		WORD ramAddress;

		for (i = 0; i < SCREEN_WIDTH; i++)
		{
			oamAddress = 0xFE00 + i;
			ramAddress = from + i;

			cpu[oamAddress] = cpu[ramAddress];

			// TODO writeMemory(oamAddress, readMemory(ramAddress));
		}
	}
	// Timers
	else if (address == 0xFF04)
	{
		// Writing to the divider register resets it
		cpu[address] = 0;
	}
	else if (address == 0xFF07)
	{
		BYTE curFreq = getFrequency();
		cpu[address] = data;
		BYTE newFreq = getFrequency();
		if (curFreq != newFreq)
		{
			setFrequency();
		}
	}
	// Interrupts
	else if (address == 0xFF0F)
	{
		cpu[address] = data;
		interrupt.flags = data;
	}
	else if (address == 0xFFFF)
	{
		cpu[address] = data;
		interrupt.enable = data;
	}
	// No other special areas, just write the data
	else
	{
		cpu[address] = data;
	}
}

void pushStack(WORD immediate)
{
	BYTE msb = (immediate >> 8) & 0xFF;
	BYTE lsb = immediate & 0xFF;
	/*
		------------------
		SP		0000
		SP -1	lsb
		SP -2	msb		-- new SP
		------------------
	*/
	writeMemory(--SP.pair, lsb);
	writeMemory(--SP.pair, msb);
}

WORD popStack()
{
	BYTE msb;
	BYTE lsb;

	/*
	------------------
	SP +2	0000	-- new SP
	SP +1	lsb
	SP		msb
	------------------
	*/
	msb = readMemory(SP.pair++);
	lsb = readMemory(SP.pair++);

	WORD value = (msb << 8) | lsb;

	return value;
}