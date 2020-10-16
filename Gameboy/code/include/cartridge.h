#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "hardware.h"

int readROM(char*);

/* The size of a cartridge can be anywhere from 32 kb (0x8000) to 2 MB 0x200000 */
#define MAX_CARTRIDGE_SIZE	0x200000
#define MAX_EXT_RAM_SIZE	0x8000
BYTE mCartridge[MAX_CARTRIDGE_SIZE];
BYTE mExtRAM[MAX_EXT_RAM_SIZE];

#define TITLE_BYTE		0x0134
#define CART_TYPE_BYTE	0x0147
#define ROM_SIZE_BYTE	0x0148
#define RAM_SIZE_BYTE	0x0149

typedef struct
{
	BYTE title[16];			// Bytes 0x134-0x143 of the cartridge
	BYTE cartridgeType;		// Byte 0x147 of the cartridge
	int romSize;			// Byte 0x148 of the cartridge
	int ramSize;			// Byte 0x149 of the cartridge
} cartridgeHeader;

typedef struct
{
	BYTE register0;
	BYTE register2;
	BYTE register3;
	WORD romBank;
	BYTE ramBank;	
} memoryBankController;

cartridgeHeader mCartridgeHeader;
memoryBankController mMBC;

// Cartridge bank sizes
#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000

#endif