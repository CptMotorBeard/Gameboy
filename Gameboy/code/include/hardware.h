#ifndef HARDWARE_H
#define HARDWARE_H

// Let's define the height and width of the Screen
#define SCREEN_HEIGHT 144
#define SCREEN_WIDTH  160

// Defining the types based off of GB types and data sizes
typedef unsigned char	BYTE;
typedef signed char		SIGNED_BYTE;
typedef unsigned short	WORD;
// Define the size of a bit so there's no hardcoding
#define SIZE_OF_BYTE   1
#define BITS_PER_BYTE 8

// is cpu stopped?
int stopped;
// is cpu halted?
int halt;

// clock
int clock;

// Registers can work either as a single 8 bit register or a pair to make a 16 bit register.
typedef union {
	struct {
		BYTE lo;
		BYTE hi;
	};
	WORD pair;
} Register;

// Zero Flag
#define FLAG_Z (1 << 7)
// Negative Flag
#define FLAG_N (1 << 6)
// Half Carry Flag
#define FLAG_H (1 << 5)
// Carry Flag
#define FLAG_C (1 << 4)

// Define each bit so that code is a little easier to read
#define BIT_0 (1)
#define BIT_1 (1 << 1)
#define BIT_2 (1 << 2)
#define BIT_3 (1 << 3)
#define BIT_4 (1 << 4)
#define BIT_5 (1 << 5)
#define BIT_6 (1 << 6)
#define BIT_7 (1 << 7)

// We have our register pairs defined below.
Register registerAF;
Register registerBC;
Register registerDE;
Register registerHL;

// Our joypad
typedef struct {
	struct {
		BYTE a;
		BYTE b;
		BYTE select;
		BYTE start;
	}keys1;

	struct {
		BYTE right;
		BYTE left;
		BYTE up;
		BYTE down;
	}keys2;
} Keys;

Keys keys;

// program counter is 16 bits, or a word. Using a register because high and low bits are used
Register PC;

// stack pointer is 16 bits, but some opcodes use the high and low bits so declare it as a register
Register SP;

// Screen resolution is 160x144. RGB is the third value
BYTE screen[SCREEN_WIDTH][SCREEN_HEIGHT][3];

// The cpu memory map looks like :
//
//--------------------------- FFFF
// I/O ports + internal RAM
//--------------------------- FF00
// Internal RAM
//--------------------------- C000
// 8kB switchable RAM bank
//--------------------------- A000
// 16kB VRAM
//--------------------------- 8000
// 16kB switchable ROM bank
//--------------------------- 4000
// 16kB ROM bank #0
//--------------------------- 0000
//
// So total memory for the CPU is 0x10000 values
BYTE cpu[0x10000];

// functions

void initializeHardware(void);
void setJoypad(void);

#endif
