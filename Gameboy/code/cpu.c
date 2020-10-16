#include "hardware.h"
#include "memory.h"
#include "cpu.h"
#include "cartridge.h"
#include <stdio.h>

struct opcode mOpcodes[256] =
{
  { 0, NOP },			// 0x00
  { 2, LD_BC },			// 0x01
  { 0, LD_BC_A },		// 0x02
  { 0, INC_BC },		// 0x03
  { 0, INC_B },			// 0x04
  { 0, DEC_B },			// 0x05
  { 1, LD_B },			// 0x06
  { 0, RLCA },			// 0x07
  { 2, LD_04X_SP },		// 0x08
  { 0, ADD_HL_BC },		// 0x09
  { 0, LD_A_BC },		// 0x0a
  { 0, DEC_BC },		// 0x0b
  { 0, INC_C },			// 0x0c
  { 0, DEC_C },			// 0x0d
  { 1, LD_C },			// 0x0e
  { 0, RRCA },			// 0x0f
  { 1, STOP },			// 0x10
  { 2, LD_DE },			// 0x11
  { 0, LD_DE_A },		// 0x12
  { 0, INC_DE },		// 0x13
  { 0, INC_D },			// 0x14
  { 0, DEC_D },			// 0x15
  { 1, LD_D },			// 0x16
  { 0, RLA },			// 0x17
  { 1, JR },			// 0x18
  { 0, ADD_HL_DE },		// 0x19
  { 0, LD_A_DE },		// 0x1a
  { 0, DEC_DE },		// 0x1b
  { 0, INC_E },			// 0x1c
  { 0, DEC_E },			// 0x1d
  { 1, LD_E },			// 0x1e
  { 0, RRA },			// 0x1f
  { 1, JR_NZ },			// 0x20
  { 2, LD_HL_WORD },	// 0x21
  { 0, LDI_HL_A },		// 0x22
  { 0, INC_HL },		// 0x23
  { 0, INC_H },			// 0x24
  { 0, DEC_H },			// 0x25
  { 1, LD_H },			// 0x26
  { 0, DAA },			// 0x27
  { 1, JR_Z },			// 0x28
  { 0, ADD_HL_HL },		// 0x29
  { 0, LDI_A_HL },		// 0x2a
  { 0, DEC_HL },		// 0x2b
  { 0, INC_L },			// 0x2c
  { 0, DEC_L },			// 0x2d
  { 1, LD_L },			// 0x2e
  { 0, CPL },			// 0x2f
  { 1, JR_NC },			// 0x30
  { 2, LD_SP },			// 0x31
  { 0, LDD_HL_A },		// 0x32
  { 0, INC_SP },		// 0x33
  { 0, INC_HL_P },		// 0x34
  { 0, DEC_HL_P },		// 0x35
  { 1, LD_HL_BYTE },	// 0x36
  { 0, SCF },			// 0x37
  { 1, JR_C },			// 0x38
  { 0, ADD_HL_SP },		// 0x39
  { 0, LDD_A_HL },		// 0x3a
  { 0, DEC_SP },		// 0x3b
  { 0, INC_A },			// 0x3c
  { 0, DEC_A },			// 0x3d
  { 1, LD_A_BYTE },		// 0x3e
  { 0, CCF },			// 0x3f
  { 0, LD_B_B },		// 0x40
  { 0, LD_B_C },		// 0x41
  { 0, LD_B_D },		// 0x42
  { 0, LD_B_E },		// 0x43
  { 0, LD_B_H },		// 0x44
  { 0, LD_B_L },		// 0x45
  { 0, LD_B_HL },		// 0x46
  { 0, LD_B_A },		// 0x47
  { 0, LD_C_B },		// 0x48
  { 0, LD_C_C },		// 0x49
  { 0, LD_C_D },		// 0x4a
  { 0, LD_C_E },		// 0x4b
  { 0, LD_C_H },		// 0x4c
  { 0, LD_C_L },		// 0x4d
  { 0, LD_C_HL },		// 0x4e
  { 0, LD_C_A },		// 0x4f
  { 0, LD_D_B },		// 0x50
  { 0, LD_D_C },		// 0x51
  { 0, LD_D_D },		// 0x52
  { 0, LD_D_E },		// 0x53
  { 0, LD_D_H },		// 0x54
  { 0, LD_D_L },		// 0x55
  { 0, LD_D_HL },		// 0x56
  { 0, LD_D_A },		// 0x57
  { 0, LD_E_B },		// 0x58
  { 0, LD_E_C },		// 0x59
  { 0, LD_E_D },		// 0x5a
  { 0, LD_E_E },		// 0x5b
  { 0, LD_E_H },		// 0x5c
  { 0, LD_E_L },		// 0x5d
  { 0, LD_E_HL },		// 0x5e
  { 0, LD_E_A },		// 0x5f
  { 0, LD_H_B },		// 0x60
  { 0, LD_H_C },		// 0x61
  { 0, LD_H_D },		// 0x62
  { 0, LD_H_E },		// 0x63
  { 0, LD_H_H },		// 0x64
  { 0, LD_H_L },		// 0x65
  { 0, LD_H_HL },		// 0x66
  { 0, LD_H_A },		// 0x67
  { 0, LD_L_B },		// 0x68
  { 0, LD_L_C },		// 0x69
  { 0, LD_L_D },		// 0x6a
  { 0, LD_L_E },		// 0x6b
  { 0, LD_L_H },		// 0x6c
  { 0, LD_L_L },		// 0x6d
  { 0, LD_L_HL },		// 0x6e
  { 0, LD_L_A },		// 0x6f
  { 0, LD_HL_B },		// 0x70
  { 0, LD_HL_C },		// 0x71
  { 0, LD_HL_D },		// 0x72
  { 0, LD_HL_E },		// 0x73
  { 0, LD_HL_H },		// 0x74
  { 0, LD_HL_L },		// 0x75
  { 0, HALT },			// 0x76
  { 0, LD_HL_A },		// 0x77
  { 0, LD_A_B },		// 0x78
  { 0, LD_A_C },		// 0x79
  { 0, LD_A_D },		// 0x7a
  { 0, LD_A_E },		// 0x7b
  { 0, LD_A_H },		// 0x7c
  { 0, LD_A_L },		// 0x7d
  { 0, LD_A_HL },		// 0x7e
  { 0, LD_A_A },		// 0x7f
  { 0, ADD_A_B },		// 0x80
  { 0, ADD_A_C },		// 0x81
  { 0, ADD_A_D },		// 0x82
  { 0, ADD_A_E },		// 0x83
  { 0, ADD_A_H },		// 0x84
  { 0, ADD_A_L },		// 0x85
  { 0, ADD_A_HL },		// 0x86
  { 0, ADD_A },			// 0x87
  { 0, ADC_B },			// 0x88
  { 0, ADC_C },			// 0x89
  { 0, ADC_D },			// 0x8a
  { 0, ADC_E },			// 0x8b
  { 0, ADC_H },			// 0x8c
  { 0, ADC_L },			// 0x8d
  { 0, ADC_HL }	,		// 0x8e
  { 0, ADC_A },			// 0x8f
  { 0, SUB_B },			// 0x90
  { 0, SUB_C },			// 0x91
  { 0, SUB_D },			// 0x92
  { 0, SUB_E },			// 0x93
  { 0, SUB_H },			// 0x94
  { 0, SUB_L },			// 0x95
  { 0, SUB_HL },		// 0x96
  { 0, SUB_A },			// 0x97
  { 0, SBC_B },			// 0x98
  { 0, SBC_C },			// 0x99
  { 0, SBC_D },			// 0x9a
  { 0, SBC_E },			// 0x9b
  { 0, SBC_H },			// 0x9c
  { 0, SBC_L },			// 0x9d
  { 0, SBC_HL },		// 0x9e
  { 0, SBC_A },			// 0x9f
  { 0, AND_B },			// 0xa0
  { 0, AND_C },			// 0xa1
  { 0, AND_D },			// 0xa2
  { 0, AND_E },			// 0xa3
  { 0, AND_H },			// 0xa4
  { 0, AND_L },			// 0xa5
  { 0, AND_HL },		// 0xa6
  { 0, AND_A },			// 0xa7
  { 0, XOR_B },			// 0xa8
  { 0, XOR_C },			// 0xa9
  { 0, XOR_D },			// 0xaa
  { 0, XOR_E },			// 0xab
  { 0, XOR_H },			// 0xac
  { 0, XOR_L },			// 0xad
  { 0, XOR_HL },		// 0xae
  { 0, XOR_A },			// 0xaf
  { 0, OR_B },			// 0xb0
  { 0, OR_C },			// 0xb1
  { 0, OR_D },			// 0xb2
  { 0, OR_E },			// 0xb3
  { 0, OR_H },			// 0xb4
  { 0, OR_L },			// 0xb5
  { 0, OR_HL },			// 0xb6
  { 0, OR_A },			// 0xb7
  { 0, CP_B },			// 0xb8
  { 0, CP_C },			// 0xb9
  { 0, CP_D },			// 0xba
  { 0, CP_E },			// 0xbb
  { 0, CP_H },			// 0xbc
  { 0, CP_L },			// 0xbd
  { 0, CP_HL },			// 0xbe
  { 0, CP_A },			// 0xbf
  { 0, RET_NZ },		// 0xc0
  { 0, POP_BC },		// 0xc1
  { 2, JP_NZ },			// 0xc2
  { 2, JP },			// 0xc3
  { 2, CALL_NZ },		// 0xc4
  { 0, PUSH_BC },		// 0xc5
  { 1, ADD_BYTE },		// 0xc6
  { 0, RST_00 },		// 0xc7
  { 0, RET_Z },			// 0xc8
  { 0, RET },			// 0xc9
  { 2, JP_Z },			// 0xca
  { 1, CB },			// 0xcb
  { 2, CALL_Z },		// 0xcc
  { 2, CALL },			// 0xcd
  { 1, ADC_BYTE },		// 0xce
  { 0, RST_08 },		// 0xcf
  { 0, RET_NC },		// 0xd0
  { 0, POP_DE },		// 0xd1
  { 2, JP_NC },			// 0xd2
  { 0, NOP },			// 0xd3
  { 2, CALL_NC },		// 0xd4
  { 0, PUSH_DE },		// 0xd5
  { 1, SUB_BYTE },		// 0xd6
  { 0, RST_10 },		// 0xd7
  { 0, RET_C },			// 0xd8
  { 0, RETI },			// 0xd9
  { 2, JP_C },			// 0xda
  { 0, NOP },			// 0xdb
  { 2, CALL_C },		// 0xdc
  { 0, NOP },			// 0xdd
  { 1, SBC_BYTE },		// 0xde
  { 0, RST_18 },		// 0xdf
  { 1, LD_FF02X_A },	// 0xe0
  { 0, POP_HL },		// 0xe1
  { 0, LD_FFC_A },		// 0xe2
  { 0, NOP },			// 0xe3
  { 0, NOP },			// 0xe4
  { 0, PUSH_HL },		// 0xe5
  { 1, AND_BYTE },		// 0xe6
  { 0, RST_20 },		// 0xe7
  { 1, ADD_SP },		// 0xe8
  { 0, JP_HL },			// 0xe9
  { 2, LD_04X_A },		// 0xea
  { 0, NOP },			// 0xeb
  { 0, NOP },			// 0xec
  { 0, NOP },			// 0xed
  { 1, XOR_BYTE },		// 0xee
  { 0, RST_28 },		// 0xef
  { 1, LD_A_FF02X },	// 0xf0
  { 0, POP_AF },		// 0xf1
  { 0, LD_A_FFC },		// 0xf2
  { 0, DI },			// 0xf3
  { 0 , NOP },			// 0xf4
  { 0, PUSH_AF },		// 0xf5
  { 1, OR_BYTE },		// 0xf6
  { 0, RST_30 },		// 0xf7
  { 1, LD_HL_SP02X },	// 0xf8
  { 0, LD_SP_HL },		// 0xf9
  { 2, LD_A_WORD },		// 0xfa
  { 0, EI },			// 0xfb
  { 0, NOP },			// 0xfc
  { 0, NOP },			// 0xfd
  { 1, CP_BYTE },		// 0xfe
  { 0, RST_38 },		// 0xff
};

int PRINT_LOGS = 0;

void PRINT_CPU_LOGS();

void cpuStep()
{

	// cpu cycles noops (4 cycles) while stopped
	if (halt)
	{
		clock += 4;
		return;
	}

	PRINT_CPU_LOGS();

	int operands;
	BYTE currentOpcode = readMemory(PC.pair);

	// We need to find the number of operands the current opcode uses and store it
	operands = mOpcodes[currentOpcode].operands;

	if (operands == 1)
	{
		// If we have one operand, we get the parameter for the opcode from the next byte		
		PC.pair++;
		BYTE fullOperand = readMemory(PC.pair);

		((void(*)(BYTE)) mOpcodes[currentOpcode].function)(fullOperand);
	}
	else if (operands == 2)
	{
		// If we have 2 operands, we get the lsb's for the opcode from the next byte and the msb's from the byte after
		// In total the fullOperand consists of 2 bytes of data
		PC.pair++;
		WORD fullOperand = readMemory(PC.pair);

		PC.pair++;
		fullOperand = fullOperand | (readMemory(PC.pair) << 8);

		((void(*)(WORD))mOpcodes[currentOpcode].function)(fullOperand);		
	}
	else
	{
		// No operands all we need to do is run the function
		((void(*)(void))mOpcodes[currentOpcode].function)();
	}

	// Move to the next byte of data after completing the function
	// TODO figure out if there is a better way to do this. Not every opcode moves forward one byte after completing
	PC.pair++;
}

void PRINT_CPU_LOGS()
{
	static int i = 0;
	static int inLoop = 0;
	static FILE *fp;

	if (i == 0)
	{
		fopen_s(&fp, "DEBUG_LOGS_CPU.txt", "w");
		fwrite(&"", sizeof(char), 0, fp);
		fclose(fp);
		fopen_s(&fp, "DEBUG_LOGS_CPU.txt", "a");
	}

	if (inLoop)
	{
		if (PC.pair == 0x0749 || PC.pair == 0x0788 || PC.pair == 0x07D6 || PC.pair == 0x00849 || PC.pair == 0x0217 || PC.pair == 0x020B || PC.pair == 0xC007 || PC.pair == 0xC084)
		{
			inLoop = 0;
		}

		return;
	}

	i++;	

	if (i > 100000)
	{
		return;
	}

#define len 42
	char *log = { "PC: %04X, opcode: %02X, mb: %04X, regA: %02X\n" };
	char print_log[len];

	BYTE currOp = readMemory(PC.pair);

	sprintf_s(print_log, sizeof(char) * len, log, PC.pair, currOp, mMBC.romBank, registerAF.hi);
	fwrite(&print_log, sizeof(char), len, fp);

	if (PC.pair == 0x73E || PC.pair == 0x0784 || PC.pair == 0x07CE || PC.pair ==  0x0847 || PC.pair == 0x0213 || PC.pair == 0x0209 || PC.pair == 0xC003 || PC.pair == 0xC06A)
	{
		inLoop = 1;
	}
}

void DEBUG_CARTRIDGE()
{

	char *log = { "PC: %04X, opcode: %02X\n" };
	char print_log[22];

	WORD tempPC = 0x00;
	int inc = 0;

	FILE *fp;
	fopen_s(&fp, "DEBUG_LOGS.txt", "w");
	fwrite(&"", sizeof(char), 0, fp);
	fclose(fp);
	fopen_s(&fp, "DEBUG_LOGS.txt", "a");

	tempPC = 0x00;

	for (int i = 0x00; i < 0x0F; i++)
	{
		mMBC.romBank = i;

		while (tempPC < 0x4000)
		{
			BYTE currOp = readMemory(tempPC + 0x4000);
			inc++;

			sprintf_s(print_log, sizeof(char) * 22, log, tempPC, currOp);
			fwrite(&print_log, sizeof(char), 22, fp);

			tempPC += inc;
			inc = 0;
		}			

		fwrite("\n", sizeof(char), 2, fp);
		tempPC = 0;
	}
	
	fclose(fp);
}