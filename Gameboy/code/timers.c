#include "hardware.h"
#include "memory.h"
#include "timers.h"
#include "interrupts.h"

#define DIVIDER 0xFF04
#define TIMA    0xFF05
#define TMA     0xFF06
/*
	Bit 2 of TMC is timer control - 0: Timer Stop, 1: Timer Start
	Bit 0 and 1 of TMC is input clock select. Frequency is determined by this value
*/
#define TMC     0xFF07

#define CLOCKSPEED 4194304

#define FREQUENCY_0 4096
#define FREQUENCY_1 262144
#define FREQUENCY_2 65536
#define FREQUENCY_3 16382

int mDividerCounter = 0;

// Default frequency is frequency 0, 4096
int mTimerCounter = CLOCKSPEED / FREQUENCY_0;

void timerStep()
{
	dividerRegister(clock);
	if ((readMemory(TMC) >> 2) & 1)
	{
		mTimerCounter -= clock;
		if (mTimerCounter <= 0)
		{
			setFrequency();
			if (readMemory(TIMA) == 255)
			{
				writeMemory(TIMA, readMemory(TMA));
				if (interrupt.enable && INTERRUPTS_TIMER)
				{
					interrupt.flags |= INTERRUPTS_TIMER;
				}
			}
			else
			{
				cpu[TIMA]++;
			}
		}
	}
}

BYTE getFrequency()
{
	return readMemory(TMC) & (BIT_0 || BIT_1);
}

void setFrequency()
{
	BYTE f = getFrequency();
	int frequency;
	switch (f)
	{
	case 0:
		frequency = FREQUENCY_0 ;
		break;
	case 1:
		frequency = FREQUENCY_1;
		break;
	case 2:
		frequency = FREQUENCY_2;
		break;
	case 3:
		frequency = FREQUENCY_3;
		break;
	default:
		frequency = FREQUENCY_0;
		break;
	}
	mTimerCounter = CLOCKSPEED / frequency;
}

void dividerRegister(int cycles)
{
	mDividerCounter += cycles;
	// 255 is the max value of 1 byte, and we need to make sure an overflow increments our divider register
	if (mDividerCounter >= 255)
	{
		mDividerCounter = 0;
		cpu[DIVIDER]++;
	}
}
