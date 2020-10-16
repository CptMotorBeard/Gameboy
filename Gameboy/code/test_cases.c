#include "opcodes.h"
#include "cpu.h"
#include "cartridge.h"
#include <assert.h>

#define NZ (assert(!isFlagSet(FLAG_Z)))
#define NH (assert(!isFlagSet(FLAG_H)))
#define NC (assert(!isFlagSet(FLAG_C)))
#define CHECKSUM 0xC000

void RESET_RUN()
{
	PC.pair = 0x00;
}

void TEST_RUN_CPU()
{
	PC.pair = 0x0000;

	while (PC.pair != 0xFFFF)
	{
		cpuStep();
	}

	return;
}

BYTE GET_BYTE_VALUE(void *function)
{
	BYTE i;
	for (i = 0; i <= 0xFF; i++)
	{
		if (mOpcodes[i].function == function)
		{
			return i;
		}
	}
	return 0x00;
}

void ADD_END()
{
	mCartridge[PC.pair++] = GET_BYTE_VALUE(JP);			// 0x00
	mCartridge[PC.pair++] = 0xFF;						// 0x01
	mCartridge[PC.pair++] = 0xFF;						// 0x02
}

void UPDATE_CRC()
{
	PC.pair = 0x1000;

	mCartridge[PC.pair++] = GET_BYTE_VALUE(PUSH_AF);		// 0x00
	mCartridge[PC.pair++] = GET_BYTE_VALUE(PUSH_BC);		// 0x01
	mCartridge[PC.pair++] = GET_BYTE_VALUE(PUSH_DE);		// 0x02
	mCartridge[PC.pair++] = GET_BYTE_VALUE(PUSH_HL);		// 0x03
	mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_HL_WORD);		// 0x04
	mCartridge[PC.pair++] = ((CHECKSUM & 0xFF) >> 8) + 3;	// 0x05
	mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_B_HL);		// 0x06
	mCartridge[PC.pair++] = GET_BYTE_VALUE(DEC_L);			// 0x07
	mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_C_HL);		// 0x08
	mCartridge[PC.pair++] = GET_BYTE_VALUE(DEC_L);			// 0x09
	mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_D_HL);		// 0x0A
	mCartridge[PC.pair++] = GET_BYTE_VALUE(DEC_L);			// 0x0B
	mCartridge[PC.pair++] = GET_BYTE_VALUE(XOR_HL);			// 0x0C

	mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_HL_BYTE);		// 0x0D
	mCartridge[PC.pair++] = 0x08;							// 0x0E
	mCartridge[PC.pair++] = GET_BYTE_VALUE(CB);				// 0x0F	--
	mCartridge[PC.pair++] = 0x38;							// 0x10
}

void TEST_SPECIAL()
{
	// JR negative
	{
		RESET_RUN();

		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_A_BYTE);	// 0x00
		mCartridge[PC.pair++] = 0x00;						// 0x01
		mCartridge[PC.pair++] = GET_BYTE_VALUE(JP);			// 0x02
		mCartridge[PC.pair++] = 0x0D;						// 0x03
		mCartridge[PC.pair++] = 0x00;						// 0x04
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_A);		// 0x05
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_A);		// 0x06
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_A);		// 0x07
		mCartridge[PC.pair++] = GET_BYTE_VALUE(CP_BYTE);	// 0x08
		mCartridge[PC.pair++] = 2;							// 0x09
		ADD_END();											// 0x0A, 0x0B,  0x0C
		mCartridge[PC.pair++] = GET_BYTE_VALUE(JR);			// 0x0D
		mCartridge[PC.pair++] = -9;							// 0x0E

		TEST_RUN_CPU();

		NZ;
	}

	// JR positive
	{
		RESET_RUN();
		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_A_BYTE);	// 0x00
		mCartridge[PC.pair++] = 0x00;						// 0x01
		mCartridge[PC.pair++] = GET_BYTE_VALUE(JR);			// 0x02
		mCartridge[PC.pair++] = 0x01;						// 0x03
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_A);		// 0x04
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_A);		// 0x05
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_A);		// 0x06
		mCartridge[PC.pair++] = GET_BYTE_VALUE(CP_BYTE);	// 0x07
		mCartridge[PC.pair++] = 0x02;						// 0x08
		ADD_END();

		TEST_RUN_CPU();

		NZ;
	}

	// JP HL
	{
		RESET_RUN();

		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_HL_WORD);	// 0x00
		mCartridge[PC.pair++] = 0x07;						// 0x01
		mCartridge[PC.pair++] = 0x00;						// 0x02
		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_A_BYTE);	// 0x03		
		mCartridge[PC.pair++] = 0x00;						// 0x04
		mCartridge[PC.pair++] = GET_BYTE_VALUE(JP_HL);		// 0x05
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_A);		// 0x06		
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_A);		// 0x07		
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_A);		// 0x08	
		mCartridge[PC.pair++] = GET_BYTE_VALUE(CP_BYTE);	// 0x09
		mCartridge[PC.pair++] = 0x02;						// 0x0A
		ADD_END();

		TEST_RUN_CPU();


		NZ;
	}

	// POP AF
	{
		RESET_RUN();

		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_A_WORD);	// 0x00
		mCartridge[PC.pair++] = 0x12;						// 0x01
		mCartridge[PC.pair++] = 0x00;						// 0x02

		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_BC_A);	// 0x00
		mCartridge[PC.pair++] = GET_BYTE_VALUE(PUSH_BC);	// 0x01	--
		mCartridge[PC.pair++] = GET_BYTE_VALUE(POP_AF);		// 0x02
		mCartridge[PC.pair++] = GET_BYTE_VALUE(PUSH_AF);	// 0x03
		mCartridge[PC.pair++] = GET_BYTE_VALUE(POP_DE);		// 0x04
		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_A_C);		// 0x05
		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_HL_WORD);	// 0x06
		mCartridge[PC.pair++] = 0x00;						// 0x07
		mCartridge[PC.pair++] = 0xF0;						// 0x08		
		mCartridge[PC.pair++] = GET_BYTE_VALUE(AND_HL);		// 0x09
		mCartridge[PC.pair++] = GET_BYTE_VALUE(CP_BYTE);	// 0x0A
		mCartridge[PC.pair++] = 0x0E;						// 0x0B
		mCartridge[PC.pair++] = GET_BYTE_VALUE(JP_NZ);		// 0x0C
		mCartridge[PC.pair++] = 0xFF;						// 0x0D
		mCartridge[PC.pair++] = 0xFF;						// 0x0E
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_B);		// 0x0F
		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_C);		// 0x10
		mCartridge[PC.pair++] = GET_BYTE_VALUE(JR_NZ);		// 0x11
		mCartridge[PC.pair++] = -17;						// 0x12
		ADD_END();

		TEST_RUN_CPU();

		NZ;
	}	

	// DAA
	{
		RESET_RUN();

		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_DE);		// 0x00
		mCartridge[PC.pair++] = 0x00;						// 0x01
		mCartridge[PC.pair++] = 0x00;						// 0x02
		mCartridge[PC.pair++] = GET_BYTE_VALUE(PUSH_DE);	// 0x03	--
		mCartridge[PC.pair++] = GET_BYTE_VALUE(POP_AF);		// 0x04
		mCartridge[PC.pair++] = GET_BYTE_VALUE(DAA);		// 0x05

		mCartridge[PC.pair++] = GET_BYTE_VALUE(PUSH_AF);	// 0x06
		mCartridge[PC.pair++] = GET_BYTE_VALUE(CALL);		// 0x07		
		mCartridge[PC.pair++] = 0x00;						// 0x08
		mCartridge[PC.pair++] = 0x00;						// 0x09
		mCartridge[PC.pair++] = GET_BYTE_VALUE(POP_HL);		// 0x0A
		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_A_L);		// 0x0B
		mCartridge[PC.pair++] = GET_BYTE_VALUE(CALL);		// 0x0C	
		mCartridge[PC.pair++] = 0x00;						// 0x0D
		mCartridge[PC.pair++] = 0x00;						// 0x0E

		mCartridge[PC.pair++] = GET_BYTE_VALUE(INC_D);		// 0x0F
		mCartridge[PC.pair++] = GET_BYTE_VALUE(JR_NZ);		// 0x10
		mCartridge[PC.pair++] = -14;						// 0x11

		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_A_E);		// 0x12
		mCartridge[PC.pair++] = GET_BYTE_VALUE(ADD_BYTE);	// 0x13
		mCartridge[PC.pair++] = GET_BYTE_VALUE(LD_E_A);		// 0x14
		mCartridge[PC.pair++] = GET_BYTE_VALUE(JR_NZ);		// 0x15
		mCartridge[PC.pair++] = -19;						// 0x16

		mCartridge[PC.pair++] = GET_BYTE_VALUE(CALL);		// 0x17	
		mCartridge[PC.pair++] = 0x00;						// 0x18
		mCartridge[PC.pair++] = 0x00;						// 0x19
		ADD_END();
	}
}

void TEST_HL_SP()
{

}

void TEST_LD()
{

}

void TEST_8BIT_LOGIC()
{

}

void TEST_16BIT_LOGIC()
{

}

void TEST_JR_JP_CALL_RET_RST()
{

}

void TEST_A_HL()
{

}

void TEST_MISC()
{

}

void TEST_BIT_OPS()
{

}

void TEST_OPCODES()
{
	// TEST_SPECIAL();
	TEST_HL_SP();
	TEST_LD();
	TEST_8BIT_LOGIC();
	TEST_16BIT_LOGIC();
	TEST_JR_JP_CALL_RET_RST();
	TEST_A_HL();
	TEST_MISC();
	TEST_BIT_OPS();
}