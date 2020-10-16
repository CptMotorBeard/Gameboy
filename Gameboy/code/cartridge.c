#include "cartridge.h"
#include <stdio.h>

void fillHeader(FILE *rom)
{
	BYTE tempROMSize;
	BYTE tempRAMSize;

	fseek(rom, TITLE_BYTE, SEEK_SET);
	fread(mCartridgeHeader.title, SIZE_OF_BYTE, 16, rom);

	fseek(rom, CART_TYPE_BYTE, SEEK_SET);
	fread(&mCartridgeHeader.cartridgeType, SIZE_OF_BYTE, 1, rom);

	fseek(rom, ROM_SIZE_BYTE, SEEK_SET);
	fread(&tempROMSize, SIZE_OF_BYTE, 1, rom);

	fseek(rom, RAM_SIZE_BYTE, SEEK_SET);
	fread(&tempRAMSize, SIZE_OF_BYTE, 1, rom);

	// Make sure we go back to the beginning
	fseek(rom, 0, SEEK_SET);

	switch (tempROMSize)
	{
	case 0x00:	// 32 kb ROM size	(no ROM banking)
		mCartridgeHeader.romSize = 0x8000;
		break;
	case 0x01:	// 64 kb ROM size	(4 banks)
		mCartridgeHeader.romSize = 0x10000;
		break;
	case 0x02:	// 128 kb ROM size	(8 banks)
		mCartridgeHeader.romSize = 0x20000;
		break;
	case 0x03:	// 256 kb ROM size	(16 banks)
		mCartridgeHeader.romSize = 0x40000;
		break;
	case 0x04:	// 512 kb ROM size	(32 banks)
		mCartridgeHeader.romSize = 0x80000;
		break;
	case 0x05:	// 1 mb ROM size	(64 banks	- only 63 used with MBC1)
		mCartridgeHeader.romSize = 0x100000;
		break;
	case 0x06:	// 2 mb ROM size	(128 banks	- only 125 used with MBC1)
		mCartridgeHeader.romSize = 0x200000;
		break;
	
	default:
		mCartridgeHeader.romSize = MAX_CARTRIDGE_SIZE;
		break;
	}
}

int readROM(char* input)
{
	// Binary files are read with the extra 'b' param
	// Note to self - Don't try using void * again for a FILE
	FILE *rom;
	fopen_s(&rom, input, "rb");
	if (rom == 0)
	{
		return 0;
	}
	else
	{
		fillHeader(rom);
		
		// Don't allow for memory overflows with bad cartridges
		if (mCartridgeHeader.romSize > MAX_CARTRIDGE_SIZE)
		{
			return 0;
		}

		fread(mCartridge, SIZE_OF_BYTE, mCartridgeHeader.romSize, rom);
		fclose(rom);

		return 1;
	}
}

/*
	MBC starts on page 215 of the gameboy programming manual
	
	Each bank of ROM memory takes up 0x4000 bytes
	The cartridge can have up to 0x80 banks for a max cartridge size of 0x200000 (2 MB)

	Bank 0 is always loaded on the cpu in the 16kB ROM bank #0
	The rest of the banks are switched on the cpu via the 16kB switchable ROM bank

	There are 4 RAM banks, each of 0x2000 bytes for a total of 0x8000 bytes
	These are swapped in via the 8kB switchable RAM bank on the cpu
*/

/*
	0xA000 - 0xBFFF is the RAM bank. If the cartridge contains a battery then the data written is able to be preserved across shutdowns.
*/

/*
	Memory Bank Controller 1

	MBC1 enables the use of 64 kb or more of ROM and 32 kb of RAM
	To control up to 512 kb of ROM
		MBC1 can control up to 32 kb of RAM
	To control 1 mb or more of ROM
		When used to control 1 mb of ROM, MCB only uses the first 0x1F banks and cannot access bank 0x20 or greater
		When used to control 2 mb of ROM, MCB can't use the following ROM addresses:
			0x8000	 - 0x083FFF (Bank 0x20)	- If accessed, bank 0x21 is accessed instead
			0x100000 - 0x103FFF (Bank 0x40)	- If accessed, bank 0x41 is accessed instead
			0x180000 - 0x183FFF (Bank 0x60)	- If accessed, bank 0x61 is accessed instead
		RAM used by MBC1 is restricted to 8 KB

	Register 0	(RAMCS gate data - serves as write-protection for RAM):
		Write Area: 0x0000 - 0x1FFF
		Write Data: 0x0A
		Writing 0x0A to the write area causes the CS to be output, allowing access to RAM

	Register 1	(ROM bank code):
		Write Area: 0x2000 - 0x3FFF
		Write Data: 0x01 - 0x1F
		Allows the ROM bank to be selected in 16 kb increments

	Register 2	(Upper ROM bank code when using 1 mb or more of ROM and register 3 is 0):
		Write Area: 0x4000 - 0x5FFF
		Write Data: 0x00 - 0x03
		The upper ROM banks can be selected in 512 kb increments:
			0 - banks 0x01 - 0x1f
			1 - banks 0x21 - 0x3F
			2 - banks 0x41 - 0x5F
			3 - banks 0x61 - 0x7F

	Register 2	(RAM bank code when using 1 mb or more of ROM and register 3 is 1):
		Write Area: 0x4000 - 0x5FFF
		Write Data: 0x00 - 0x03
		The RAM bank can be selected in 8 kb increments.

	Register 3	(ROM / RAM change)
		Write Area: 0x6000 - 0x7FFF
		Write Data: 0x00 - 0x01
		Writing 0 causes register 2 output to control switching the higher ROM bank
		Writing 1 causes register 2 output to control switching of the RAM bank
*/

/*
	Memory Bank Controller 2

	MBC2 enables the use of up to 256 kb of ROM with built-in backup RAM (512 x 4 bits)
	MBC2 only needs 0x200 bytes from A000 on the CPU for RAM. This backup RAM is write-protected
	MBC2 can only access the first 0x0F banks can cannot access bank 0x10 or greater

	Register 0	(RAMCS gate data, serves as write-protection for RAM)
		Write Area: 0x0000 - 0x0FFF
		Write Data: 0x0A
		Writing 0x0A to the write area causes the CS to be output, allowing access to RAM

	Register 1	(ROM bank code):
		Write Area: 0x2100 - 0x21FF
		Write Data: 0x01 - 0x0F
		The ROM bank can be selected
*/

/*
	Memory Bank Controller 3

	MBC3 allows the use of between 64 kb and 2 mb of ROM and 32 kb of RAM
	Built into this controller are clock counters (32.768 KHz)
	The clock counters are accessed by RAM bank switching

	Register 0	(Write protects RAM and the clock counters - defaults to 0 on startup):
		Write Area: 0x0000 - 0x1FFF
		Write Data: 0x0A
		Writing 0x0A to the write area causes the CS to be output, allowing access to RAM and clock counters

	Register 1	(ROM bank code, selects ROM bank 1 - defaults to 0 on startup):
		Write Area: 0x2000 - 0x3FFF
		Write Data: 0x01 - 0x7F
		Allows the ROM bank to be selected in 16 kb increments

	Register 2	(ROM bank code, selects RAM bank 0 - defaults to 0 on startup):
		Write Area: 0x4000 - 0x5FFF
		Write Data: 0x00 - 0x03
		Allows the RAM bank to be selected in 8 kb increments

		Write Data: 0x08 - 0x0C
		Allows the clock counter to be selected
		This value will be returned no matter what CPU address in RAM is read
			08	-	RTC_S (seconds counter, 6 bits): 0-59
			09	-	RTC_M (minutes counter, 6 bits): 0-59
			0A	-	RTC_H (hours counter, 5 bits): 0-23
			0B	-	RTC_DL (lower-order 8 bits of days counter): 0-255
			0C	-	RTC_DH (higher-order bit and carry bit of days counter)
				Bit 0: MSB of days counter
				Bit 6: HALT
				Bit 7: Carry bit of days counter

			The days counter consists of a 9-bit counter + a carry bit. It can count from 0 - 511
			Once the carry bit is set to 1, it remains 1 until 0 is written
			The counters operate when HALT is 0 and stop when HALT is 1
			Values outside the given counter ranges will not be correctly written

	Register 3	(Latches the data for all clock counters - defaults to 0 on startup):
		Write Area: 0x6000 - 0x7FFF
		Write Data: 0 -> 1
		Writing 0 -> 1 causes all counter data to be latched. Latched contents are retained until 0 -> 1 is written again
		The counter continues to operate, but the current counter value is saved

*/