#include "hardware.h"
#include "memory.h"
#include "opcodes.h"
#include "interrupts.h"

// Helper functions for opcodes
// 8-bit loads
void LD(BYTE* r, BYTE immediate)
{
	*r = immediate;
}

// 16-bit loads
void LD_16(Register* r, WORD immediate)
{
	r->pair = immediate;
}

// Flag operations
void setFlag(BYTE flag)
{
	registerAF.lo |= flag;
}

void clearFlag(BYTE flag)
{
	registerAF.lo &= ~flag;
}

void flipFlag(BYTE flag)
{
	registerAF.lo ^= flag;
}

BYTE isFlagSet(BYTE flag)
{
	if (registerAF.lo & flag)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// 8 bit ALU
BYTE INC(BYTE r)
{
	clearFlag(FLAG_N);

	if ((r & 0xF) == 0xF)
	{
		setFlag(FLAG_H);
	}
	else
	{
		clearFlag(FLAG_H);
	}

	r++;

	if (r == 0)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}

	return r;
}

BYTE DEC(BYTE r)
{
	setFlag(FLAG_N);

	if (r & 0x0F)
	{
		clearFlag(FLAG_H);
	}
	else
	{
		setFlag(FLAG_H);
	}

	r--;

	if (r == 0)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}

	return r;
}

void ADD(BYTE r)
{
	clearFlag(FLAG_N);

	int result = registerAF.hi + r;
	if (result & 0xFF00)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	if ((((registerAF.hi & 0xF) + (r & 0xF)) & 0x10) == 0x10)
	{
		setFlag(FLAG_H);
	}
	else
	{
		clearFlag(FLAG_H);
	}

	registerAF.hi = (BYTE)(result & 0xFF);
	if (registerAF.hi == 0)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}
}

void ADC(BYTE r)
{
	ADD(r + (BYTE)isFlagSet(FLAG_C));
}

void SUB(BYTE r)
{
	setFlag(FLAG_N);

	if (r > registerAF.hi)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	if ((r & 0xF) > (registerAF.hi & 0xF))
	{
		setFlag(FLAG_H);
	}
	else
	{
		clearFlag(FLAG_H);
	}

	registerAF.hi -= r;
	if (registerAF.hi == 0)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}
}

void SBC(BYTE r)
{
	SUB(r + (BYTE)isFlagSet(FLAG_C));
}

void AND(BYTE r)
{
	registerAF.hi = (registerAF.hi & r);

	if (registerAF.hi == 0)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}

	clearFlag(FLAG_N);
	setFlag(FLAG_H);
	clearFlag(FLAG_C);
}

void XOR(BYTE r)
{
	registerAF.hi = (registerAF.hi ^ r);

	if (registerAF.hi == 0)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);
	clearFlag(FLAG_C);
}

void OR(BYTE r)
{
	registerAF.hi = (registerAF.hi | r);

	if (registerAF.hi == 0)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);
	clearFlag(FLAG_C);
}

void CP(BYTE r)
{
	setFlag(FLAG_N);

	if (registerAF.hi == r)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}

	if (registerAF.hi < r)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	if ((registerAF.hi & 0xF) < (r & 0xF))
	{
		setFlag(FLAG_H);
	}
	else
	{
		clearFlag(FLAG_H);
	}
}

// 16 bit ALU
void ADD_16(WORD r)
{
	clearFlag(FLAG_N);

	unsigned long result = registerHL.pair + r;

	// Check if we carry past 2 bytes
	if (result & 0xFFFF0000)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	// Check for half carry
	if ((((registerHL.pair & 0xFFF) + (r & 0xFFF)) & 0x1000) == 0x1000)
	{
		setFlag(FLAG_H);
	}
	else
	{
		clearFlag(FLAG_H);
	}

	// Compress the result back into 2 bytes
	registerHL.pair = (WORD)(result & 0xFFFF);
}

// Functions for all of the opcodes
// TODO Fix the jumps, they are so bad
void NOP()
{
	clock += 4;
	return;
}

void LD_BC(WORD operand)
{
	clock += 12;
	LD_16(&registerBC, operand);
}

void LD_BC_A()
{
	clock += 8;
	writeMemory(registerBC.pair, registerAF.hi);
}

void INC_BC()
{
	clock += 8;
	registerBC.pair++;
}

void INC_B()
{
	clock += 4;
	registerBC.hi = INC(registerBC.hi);
}

void DEC_B()
{
	clock += 4;
	registerBC.hi = DEC(registerBC.hi);
}

void LD_B(BYTE operand)
{
	clock += 8;
	LD(&registerBC.hi, operand);
}

void RLCA()
{
	clock += 4;
	BYTE carry = (registerAF.hi & 0x80) >> 7;

	if (carry)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);
	clearFlag(FLAG_Z);

	registerAF.hi <<= 1;
	registerAF.hi += carry;
}

void LD_04X_SP(WORD operand)
{
	clock += 20;
	LD_16(&SP, operand);
}

void ADD_HL_BC()
{
	clock += 8;
	ADD_16(registerBC.pair);
}

void LD_A_BC()
{
	clock += 8;
	LD(&registerAF.hi, readMemory(registerBC.pair));
}

void DEC_BC()
{
	clock += 8;
	registerBC.pair--;
}

void INC_C()
{
	clock += 4;
	registerBC.lo = INC(registerBC.lo);
}

void DEC_C()
{
	clock += 4;
	registerBC.lo = DEC(registerBC.lo);
}

void LD_C(BYTE operand)
{
	clock += 8;
	LD(&registerBC.lo, operand);
}

void RRCA()
{
	clock += 4;

	BYTE carry = registerAF.hi & 0x1;

	registerAF.hi >>= 1;
	if (carry)
	{
		registerAF.hi |= 0x80;
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);
	clearFlag(FLAG_Z);
}

void STOP(BYTE operand)
{
	clock += 4;
	stopped = 1;
}

void LD_DE(WORD operand)
{
	clock += 12;
	LD_16(&registerDE, operand);
}

void LD_DE_A()
{
	clock += 8;
	writeMemory(registerDE.pair, registerAF.hi);
}

void INC_DE()
{
	clock += 8;
	registerDE.pair++;
}

void INC_D()
{
	clock += 4;
	registerDE.hi = INC(registerDE.hi);
}

void DEC_D()
{
	clock += 4;
	registerDE.hi = DEC(registerDE.hi);
}

void LD_D(BYTE operand)
{
	clock += 8;
	LD(&registerDE.hi, operand);
}

void RLA()
{
	clock += 4;

	BYTE carry = isFlagSet(FLAG_C);

	if ((registerAF.hi & 0x80) >> 7)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);
	clearFlag(FLAG_Z);

	registerAF.hi <<= 1;
	registerAF.hi += carry;
}

void JR(SIGNED_BYTE operand)
{
	clock += 12;
	PC.pair = PC.pair + operand;
}

void ADD_HL_DE()
{
	clock += 8;
	ADD_16(registerDE.pair);
}

void LD_A_DE()
{
	clock += 8;
	LD(&registerAF.hi, readMemory(registerDE.pair));
}

void DEC_DE()
{
	clock += 8;
	registerDE.pair--;
}

void INC_E()
{
	clock += 4;
	registerDE.lo = INC(registerDE.lo);
}

void DEC_E()
{
	clock += 4;
	registerDE.lo = DEC(registerDE.lo);
}

void LD_E(BYTE operand)
{
	clock += 8;
	LD(&registerDE.lo, operand);
}

void RRA()
{
	clock += 4;

	BYTE carry = registerAF.hi & 0x1;
	registerAF.hi >>= 1;

	if (isFlagSet(FLAG_C))
	{
		registerAF.hi |= 0x80;
	}

	if (carry)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);
	clearFlag(FLAG_Z);
}

void JR_NZ(BYTE operand)
{
	if (!isFlagSet(FLAG_Z))
	{
		JR(operand);
	}
	else
	{
		clock += 8;
	}
}

void LD_HL_WORD(WORD operand)
{
	clock += 12;
	LD_16(&registerHL, operand);
}

void LDI_HL_A()
{
	clock += 8;
	LD_HL_A();
	INC_HL();
}

void INC_HL()
{
	clock += 8;
	registerHL.pair++;
}

void INC_H()
{
	clock += 4;
	registerHL.hi = INC(registerHL.hi);
}

void DEC_H()
{
	clock += 4;
	registerHL.hi = DEC(registerHL.hi);
}

void LD_H(BYTE operand)
{
	clock += 8;
	LD(&registerHL.hi, operand);
}

void DAA()
{
	clock += 4;
	// This is the worst instruction

	// DAA differs depends on if the last instructions were to add or subtract
	// Add
	if (!isFlagSet(FLAG_N))
	{
		// If we need to carry
		if (isFlagSet(FLAG_C) || registerAF.hi > 0x99)
		{
			registerAF.hi += 0x60;
			setFlag(FLAG_C);
		}
		// Or half carry, or if the value is greater then 9
		if (isFlagSet(FLAG_H) || (registerAF.hi & 0x0F) > 0x09)
		{
			registerAF.hi += 0x06;
		}
	}
	// Subtract
	else
	{
		if (isFlagSet(FLAG_C))
		{
			registerAF.hi -= 0x60;
		}
		if (isFlagSet(FLAG_H))
		{
			registerAF.hi -= 0x06;
		}
	}

	// These flags are always updated
	if (registerAF.hi == 0)
	{
		setFlag(FLAG_Z);
	}

	clearFlag(FLAG_H);
}

void JR_Z(BYTE operand)
{
	if (isFlagSet(FLAG_Z))
	{
		JR(operand);
	}
	else
	{
		clock += 8;
	}
}

void ADD_HL_HL()
{
	clock += 8;
	ADD_16(registerHL.pair);
}

void LDI_A_HL()
{
	clock += 8;
	LD_A_HL();
	INC_HL();
}

void DEC_HL()
{
	clock += 8;
	registerHL.pair--;
}

void INC_L()
{
	clock += 4;
	registerHL.lo = INC(registerHL.lo);
}

void DEC_L()
{
	clock += 4;
	registerHL.lo = DEC(registerHL.lo);
}

void LD_L(BYTE operand)
{
	clock += 8;
	LD(&registerHL.lo, operand);
}

void CPL()
{
	clock += 4;

	setFlag(FLAG_N);
	setFlag(FLAG_H);
	registerAF.hi = ~registerAF.hi;
}

void JR_NC(BYTE operand)
{
	if (!isFlagSet(FLAG_C))
	{
		JR(operand);
	}
	else
	{
		clock += 8;
	}
}

void LD_SP(WORD operand)
{
	clock += 12;
	LD_16(&SP, operand);
}

void LDD_HL_A()
{
	clock += 8;

	LD_HL_A();
	DEC_HL();
}

void INC_SP()
{
	clock += 8;
	SP.pair++;
}

void LD_HL_BYTE(BYTE operand)
{
	clock += 12;
	writeMemory(registerHL.pair, operand);
}

void SCF()
{
	clock += 4;

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);
	setFlag(FLAG_C);
}

void JR_C(BYTE operand)
{
	if (isFlagSet(FLAG_C))
	{
		JR(operand);
	}
	else
	{
		clock += 8;
	}
}

void ADD_HL_SP()
{
	clock += 8;
	ADD_16(SP.pair);
}

void LDD_A_HL()
{
	clock += 8;

	LD_A_HL();
	DEC_HL();
}

void DEC_SP()
{
	clock += 8;
	SP.pair--;
}

void INC_A()
{
	clock += 4;
	registerAF.hi = INC(registerAF.hi);
}

void DEC_A()
{
	clock += 4;
	registerAF.hi = DEC(registerAF.hi);
}

void LD_A_BYTE(BYTE operand)
{
	clock += 8;
	LD(&registerAF.hi, operand);
}

void CCF()
{
	clock += 4;

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);
	flipFlag(FLAG_C);
}

void LD_B_B()
{
	clock += 4;
	LD(&registerBC.hi, registerBC.hi);
}
void LD_B_C()
{
	clock += 4;
	LD(&registerBC.hi, registerBC.lo);
}

void LD_B_D()
{
	clock += 4;
	LD(&registerBC.hi, registerDE.hi);
}

void LD_B_E()
{
	clock += 4;
	LD(&registerBC.hi, registerDE.lo);
}

void LD_B_H()
{
	clock += 4;
	LD(&registerBC.hi, registerHL.hi);
}

void LD_B_L()
{
	clock += 4;
	LD(&registerBC.hi, registerHL.lo);
}

void LD_B_HL()
{
	clock += 8;
	LD(&registerBC.hi, readMemory(registerHL.pair));
}

void LD_B_A()
{
	clock += 4;
	LD(&registerBC.hi, registerAF.hi);
}

void LD_C_B()
{
	clock += 4;
	LD(&registerBC.lo, registerBC.hi);
}

void LD_C_C()
{
	clock += 4;
	LD(&registerBC.lo, registerBC.lo);
}

void LD_C_D()
{
	clock += 4;
	LD(&registerBC.lo, registerDE.hi);
}

void LD_C_E()
{
	clock += 4;
	LD(&registerBC.lo, registerDE.lo);
}

void LD_C_H()
{
	clock += 4;
	LD(&registerBC.lo, registerHL.hi);
}

void LD_C_L()
{
	clock += 4;
	LD(&registerBC.lo, registerHL.lo);
}

void LD_C_HL()
{
	clock += 8;
	LD(&registerBC.lo, readMemory(registerHL.pair));
}

void LD_C_A()
{
	clock += 4;
	LD(&registerBC.lo, registerAF.hi);
}

void LD_D_B()
{
	clock += 4;
	LD(&registerDE.hi, registerBC.hi);
}

void LD_D_C()
{
	clock += 4;
	LD(&registerDE.hi, registerBC.lo);
}

void LD_D_D()
{
	clock += 4;
	LD(&registerDE.hi, registerDE.hi);
}

void LD_D_E()
{
	clock += 4;
	LD(&registerDE.hi, registerDE.lo);
}

void LD_D_H()
{
	clock += 4;
	LD(&registerDE.hi, registerHL.hi);
}

void LD_D_L()
{
	clock += 4;
	LD(&registerDE.hi, registerHL.lo);
}

void LD_D_HL()
{
	clock += 8;
	LD(&registerDE.hi, readMemory(registerHL.pair));
}

void LD_D_A()
{
	clock += 4;
	LD(&registerDE.hi, registerAF.hi);
}

void LD_E_B()
{
	clock += 4;
	LD(&registerDE.lo, registerBC.hi);
}

void LD_E_C()
{
	clock += 4;
	LD(&registerDE.lo, registerBC.lo);
}

void LD_E_D()
{
	clock += 4;
	LD(&registerDE.lo, registerDE.hi);
}

void LD_E_E()
{
	clock += 4;
	LD(&registerDE.lo, registerDE.lo);
}

void LD_E_H()
{
	clock += 4;
	LD(&registerDE.lo, registerHL.hi);
}

void LD_E_L()
{
	clock += 4;
	LD(&registerDE.lo, registerHL.lo);
}

void LD_E_HL()
{
	clock += 8;
	LD(&registerDE.lo, readMemory(registerHL.pair));
}

void LD_E_A()
{
	clock += 4;
	LD(&registerDE.lo, registerAF.hi);
}

void LD_H_B()
{
	clock += 4;
	LD(&registerHL.hi, registerBC.hi);
}

void LD_H_C()
{
	clock += 4;
	LD(&registerHL.hi, registerBC.lo);
}

void LD_H_D()
{
	clock += 4;
	LD(&registerHL.hi, registerDE.hi);
}

void LD_H_E()
{
	clock += 4;
	LD(&registerHL.hi, registerDE.lo);
}

void LD_H_H()
{
	clock += 4;
	LD(&registerHL.hi, registerHL.hi);
}

void LD_H_L()
{
	clock += 4;
	LD(&registerHL.hi, registerHL.lo);
}

void LD_H_HL()
{
	clock += 8;
	LD(&registerHL.hi, readMemory(registerHL.pair));
}

void LD_H_A()
{
	clock += 4;
	LD(&registerHL.hi, registerAF.hi);
}

void LD_L_B()
{
	clock += 4;
	LD(&registerHL.lo, registerBC.hi);
}

void LD_L_C()
{
	clock += 4;
	LD(&registerHL.lo, registerBC.lo);
}

void LD_L_D()
{
	clock += 4;
	LD(&registerHL.lo, registerDE.hi);
}

void LD_L_E()
{
	clock += 4;
	LD(&registerHL.lo, registerDE.lo);
}

void LD_L_H()
{
	clock += 4;
	LD(&registerHL.lo, registerHL.hi);
}

void LD_L_L()
{
	clock += 4;
	LD(&registerHL.lo, registerHL.lo);
}

void LD_L_HL()
{
	clock += 8;
	LD(&registerHL.lo, readMemory(registerHL.pair));
}

void LD_L_A()
{
	clock += 4;
	LD(&registerHL.lo, registerAF.hi);
}

void LD_HL_B()
{
	clock += 8;
	writeMemory(registerHL.pair, registerBC.hi);
}

void LD_HL_C()
{
	clock += 8;
	writeMemory(registerHL.pair, registerBC.lo);
}

void LD_HL_D()
{
	clock += 8;
	writeMemory(registerHL.pair, registerDE.hi);
}

void LD_HL_E()
{
	clock += 8;
	writeMemory(registerHL.pair, registerDE.lo);
}

void LD_HL_H()
{
	clock += 8;
	writeMemory(registerHL.pair, registerHL.hi);
}

void LD_HL_L()
{
	clock += 8;
	writeMemory(registerHL.pair, registerHL.lo);
}

void HALT()
{
	clock += 4;
	if (interrupt.master)
	{
		halt = 1;
	}
	else
	{
		PC.pair++;
	}
}

void LD_HL_A()
{
	clock += 8;
	writeMemory(registerHL.pair, registerAF.hi);
}

void LD_A_B()
{
	clock += 4;
	LD(&registerAF.hi, registerBC.hi);
}

void LD_A_C()
{
	clock += 4;
	LD(&registerAF.hi, registerBC.lo);
}

void LD_A_D()
{
	clock += 4;
	LD(&registerAF.hi, registerDE.hi);
}

void LD_A_E()
{
	clock += 4;
	LD(&registerAF.hi, registerDE.lo);
}

void LD_A_H()
{
	clock += 4;
	LD(&registerAF.hi, registerHL.hi);
}

void LD_A_L()
{
	clock += 4;
	LD(&registerAF.hi, registerHL.lo);
}

void LD_A_HL()
{
	clock += 8;
	LD(&registerAF.hi, readMemory(registerHL.pair));
}

void LD_A_A()
{
	clock += 4;
	LD(&registerAF.hi, registerAF.hi);
}

void ADD_A_B()
{
	clock += 4;
	ADD(registerBC.hi);
}

void ADD_A_C()
{
	clock += 4;
	ADD(registerBC.lo);
}

void ADD_A_D()
{
	clock += 4;
	ADD(registerDE.hi);
}

void ADD_A_E()
{
	clock += 4;
	ADD(registerDE.lo);
}

void ADD_A_H()
{
	clock += 4;
	ADD(registerHL.hi);
}

void ADD_A_L()
{
	clock += 4;
	ADD(registerHL.lo);
}

void ADD_A_HL()
{
	clock += 8;
	ADD(readMemory(registerHL.pair));
}

void ADD_A()
{
	clock += 4;
	ADD(registerAF.hi);
}

void ADC_B()
{
	clock += 4;
	ADC(registerBC.hi);
}

void ADC_C()
{
	clock += 4;
	ADC(registerBC.lo);
}

void ADC_D()
{
	clock += 4;
	ADC(registerDE.hi);
}

void ADC_E()
{
	clock += 4;
	ADC(registerDE.lo);
}

void ADC_H()
{
	clock += 4;
	ADC(registerHL.hi);
}

void ADC_L()
{
	clock += 4;
	ADC(registerHL.lo);
}

void ADC_HL()
{
	clock += 8;
	ADC(readMemory(registerHL.pair));
}

void ADC_A()
{
	clock += 4;
	ADC(registerAF.hi);
}

void ADC_BYTE(BYTE operand)
{
	clock += 8;
	ADC(operand);
}

void SUB_B()
{
	clock += 4;
	SUB(registerBC.hi);
}

void SUB_C()
{
	clock += 4;
	SUB(registerBC.lo);
}

void SUB_D()
{
	clock += 4;
	SUB(registerDE.hi);
}

void SUB_E()
{
	clock += 4;
	SUB(registerDE.lo);
}

void SUB_H()
{
	clock += 4;
	SUB(registerHL.hi);
}

void SUB_L()
{
	clock += 4;
	SUB(registerHL.lo);
}

void SUB_HL()
{
	clock += 8;
	SUB(readMemory(registerHL.pair));
}

void SUB_A()
{
	clock += 4;
	SUB(registerAF.hi);
}

void SUB_BYTE(BYTE operand)
{
	clock += 8;
	SUB(operand);
}

void SBC_B()
{
	clock += 4;
	SBC(registerBC.hi);
}

void SBC_C()
{
	clock += 4;
	SBC(registerBC.lo);
}

void SBC_D()
{
	clock += 4;
	SBC(registerDE.hi);
}

void SBC_E()
{
	clock += 4;
	SBC(registerDE.lo);
}

void SBC_H()
{
	clock += 4;
	SBC(registerHL.hi);
}

void SBC_L()
{
	clock += 4;
	SBC(registerHL.lo);
}

void SBC_HL()
{
	clock += 8;
	SBC(readMemory(registerHL.pair));
}

void SBC_A()
{
	clock += 4;
	SBC(registerAF.hi);
}

void SBC_BYTE(BYTE operand)
{
	clock += 8;
	SBC(operand);
}

void AND_B()
{
	clock += 4;
	AND(registerBC.hi);
}

void AND_C()
{
	clock += 4;
	AND(registerBC.lo);
}

void AND_D()
{
	clock += 4;
	AND(registerDE.hi);
}

void AND_E()
{
	clock += 4;
	AND(registerDE.lo);
}

void AND_H()
{
	clock += 4;
	AND(registerHL.hi);
}

void AND_L()
{
	clock += 4;
	AND(registerHL.lo);
}

void AND_HL()
{
	clock += 8;
	AND(readMemory(registerHL.pair));
}

void AND_A()
{
	clock += 4;
	AND(registerAF.hi);
}

void AND_BYTE(BYTE operand)
{
	clock += 8;
	AND(operand);
}

void XOR_B()
{
	clock += 4;
	XOR(registerBC.hi);
}

void XOR_C()
{
	clock += 4;
	XOR(registerBC.lo);
}

void XOR_D()
{
	clock += 4;
	XOR(registerDE.hi);
}

void XOR_E()
{
	clock += 4;
	XOR(registerDE.lo);
}

void XOR_H()
{
	clock += 4;
	XOR(registerHL.hi);
}

void XOR_L()
{
	clock += 4;
	XOR(registerHL.lo);
}

void XOR_HL()
{
	clock += 8;
	XOR(readMemory(registerHL.pair));
}

void XOR_A()
{
	clock += 4;
	XOR(registerAF.hi);
}

void XOR_BYTE(BYTE operand)
{
	clock += 8;
	XOR(operand);
}

void OR_B()
{
	clock += 4;
	OR(registerBC.hi);
}

void OR_C()
{
	clock += 4;
	OR(registerBC.lo);
}

void OR_D()
{
	clock += 4;
	OR(registerDE.hi);
}

void OR_E()
{
	clock += 4;
	OR(registerDE.lo);
}

void OR_H()
{
	clock += 4;
	OR(registerHL.hi);
}

void OR_L()
{
	clock += 4;
	OR(registerHL.lo);
}

void OR_HL()
{
	clock += 4;
	OR(readMemory(registerHL.pair));
}

void OR_A()
{
	clock += 4;
	OR(registerAF.hi);
}

void OR_BYTE(BYTE operand)
{
	clock += 8;
	OR(operand);
}

void CP_B()
{
	clock += 4;
	CP(registerBC.hi);
}

void CP_C()
{
	clock += 4;
	CP(registerBC.lo);
}

void CP_D()
{
	clock += 4;
	CP(registerDE.hi);
}

void CP_E()
{
	clock += 4;
	CP(registerDE.lo);
}

void CP_H()
{
	clock += 4;
	CP(registerHL.hi);
}

void CP_L()
{
	clock += 4;
	CP(registerHL.lo);
}

void CP_HL()
{
	clock += 4;
	CP(readMemory(registerHL.pair));
}

void CP_A()
{
	clock += 4;
	CP(registerAF.hi);
}

void CP_BYTE(BYTE operand)
{
	clock += 8;
	CP(operand);
}

void RET_NZ()
{
	if (!isFlagSet(FLAG_Z))
	{
		clock += 4;
		RET();
	}
	else
	{
		clock += 8;
	}
}

void POP_BC()
{
	clock += 12;

	LD_16(&registerBC, popStack());
}

void JP_NZ(WORD operand)
{
	if (!isFlagSet(FLAG_Z))
	{
		JP(operand);
	}
	else
	{
		clock += 12;
	}
}

void JP(WORD operand)
{
	clock += 16;

	PC.pair = operand - 1;
}

void CALL_NZ(WORD operand)
{
	if (!isFlagSet(FLAG_Z))
	{
		CALL(operand);
	}
	else
	{
		clock += 12;
	}
}

void PUSH_BC()
{
	clock += 16;
	pushStack(registerBC.pair);
}

void ADD_BYTE(BYTE operand)
{
	clock += 8;
	ADD(operand);
}

void RST_00()
{
	PC.pair++;
	pushStack(PC.pair);
	JP(0x0000);
}

void RET_Z()
{
	if (isFlagSet(FLAG_Z))
	{
		clock += 4;
		RET();
	}
	else
	{
		clock += 8;
	}
}

void RET()
{
	WORD loc = popStack();
	JP(loc);
}

void JP_Z(WORD operand)
{
	if (isFlagSet(FLAG_Z))
	{
		JP(operand);
	}
	else
	{
		clock += 12;
	}
}

// CB helpers
// Rotates
BYTE RLC(BYTE r)
{
	clock += 8;

	BYTE carry = (r & 0x80) >> 7;

	if (carry)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);

	r <<= 1;
	r |= carry;

	if (r)
	{
		clearFlag(FLAG_Z);
	}
	else
	{
		setFlag(FLAG_Z);
	}

	return r;
}

BYTE RRC(BYTE r)
{
	clock += 8;

	BYTE carry = r & 0x1;
	r >>= 1;
	if (carry)
	{
		r |= 0x80;
		setFlag(FLAG_C);
	}

	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);

	if (r)
	{
		clearFlag(FLAG_Z);
	}
	else
	{
		setFlag(FLAG_Z);
	}

	return r;
}

BYTE RL(BYTE r)
{
	clock += 8;

	BYTE carry = isFlagSet(FLAG_C);

	if ((r & 0x80) >> 7)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);

	r <<= 1;
	r += carry;

	if (r)
	{
		clearFlag(FLAG_Z);
	}
	else
	{
		setFlag(FLAG_Z);
	}

	return r;
}

BYTE RR(BYTE r)
{
	clock += 8;

	BYTE carry = r & 0x1;
	r >>= 1;

	if (isFlagSet(FLAG_C))
	{
		r |= 0x80;
	}

	if (carry)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);

	if (r)
	{
		clearFlag(FLAG_Z);
	}
	else
	{
		setFlag(FLAG_Z);
	}

	return r;
}

// Shifts
BYTE SLA(BYTE r)
{
	clock += 8;

	if ((r & 0x80) >> 7)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	r <<= 1;

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);

	if (r == 0)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}

	return r;
}

BYTE SRA(BYTE r)
{
	clock += 8;

	if (r & 0x1)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);

	BYTE o7 = r & 0x80;
	r >>= 1;
	r |= o7;

	if (r)
	{
		clearFlag(FLAG_Z);
	}
	else
	{
		setFlag(FLAG_Z);
	}

	return r;
}

BYTE SRL(BYTE r)
{
	clock += 8;

	if (r & 0x1)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);

	r >>= 1;

	if (r)
	{
		clearFlag(FLAG_Z);
	}
	else
	{
		setFlag(FLAG_Z);
	}

	return r;
}

// Misc
BYTE SWAP(BYTE r)
{
	clock += 8;

	clearFlag(FLAG_N);
	clearFlag(FLAG_H);
	clearFlag(FLAG_C);

	r = ((r & 0xF) << 4) | ((r & 0xF0) >> 4);

	if (r == 0)
	{
		setFlag(FLAG_Z);
	}
	else
	{
		clearFlag(FLAG_Z);
	}

	return r;
}

// BIT operations
void BIT(BYTE bit, BYTE r)
{
	clock += 8;

	if (r & bit)
	{
		clearFlag(FLAG_Z);
	}
	else
	{
		setFlag(FLAG_Z);
	}

	clearFlag(FLAG_N);
	setFlag(FLAG_H);
}

BYTE SET(BYTE bit, BYTE r)
{
	clock += 8;

	r |= bit;
	return r;
}

BYTE RES(BYTE bit, BYTE r)
{
	clock += 8;

	r &= ~bit;
	return r;
}


void CB(BYTE operand)
{
	switch (operand)
	{
	case 0x0:
		// RLC B
		registerBC.hi = RLC(registerBC.hi);
		break;
	case 0x1:
		// RLC C
		registerBC.lo = RLC(registerBC.lo);
		break;
	case 0x2:
		// RLC D
		registerDE.hi = RLC(registerDE.hi);
		break;
	case 0x3:
		// RLC E
		registerDE.lo = RLC(registerDE.lo);
		break;
	case 0x4:
		// RLC H
		registerHL.hi = RLC(registerHL.hi);
		break;
	case 0x5:
		// RLC L
		registerHL.lo = RLC(registerHL.lo);
		break;
	case 0x6:
		// RLC (HL)
		clock += 8;
		writeMemory(registerHL.pair, RLC(readMemory(registerHL.pair)));
		break;
	case 0x7:
		// RLC A
		registerAF.hi = RLC(registerAF.hi);
		break;
	case 0x8:
		// RRC B
		registerBC.hi = RRC(registerBC.hi);
		break;
	case 0x9:
		// RRC C
		registerBC.lo = RRC(registerBC.lo);
		break;
	case 0xa:
		// RRC D
		registerDE.hi = RRC(registerDE.hi);
		break;
	case 0xb:
		// RRC E
		registerDE.lo = RRC(registerDE.lo);
		break;
	case 0xc:
		// RRC H
		registerHL.hi = RRC(registerHL.hi);
		break;
	case 0xd:
		// RRC L
		registerHL.lo = RRC(registerHL.lo);
		break;
	case 0xe:
		// RRC (HL)
		clock += 8;
		writeMemory(registerHL.pair, RRC(readMemory(registerHL.pair)));
		break;
	case 0xf:
		// RRC A
		registerAF.hi = RRC(registerAF.hi);
		break;
	case 0x10:
		// RL B
		registerBC.hi = RL(registerBC.hi);
		break;
	case 0x11:
		// RL C
		registerBC.lo = RL(registerBC.lo);
		break;
	case 0x12:
		// RL D
		registerDE.hi = RL(registerDE.hi);
		break;
	case 0x13:
		// RL E
		registerDE.lo = RL(registerDE.lo);
		break;
	case 0x14:
		// RL H
		registerHL.hi = RL(registerHL.hi);
		break;
	case 0x15:
		// RL L
		registerHL.lo = RL(registerHL.lo);
		break;
	case 0x16:
		// RL (HL)
		clock += 8;
		writeMemory(registerHL.pair, RL(readMemory(registerHL.pair)));
		break;
	case 0x17:
		// RL A
		registerAF.hi = RL(registerAF.hi);
		break;
	case 0x18:
		// RR B
		registerBC.hi = RR(registerBC.hi);
		break;
	case 0x19:
		// RR C
		registerBC.lo = RR(registerBC.lo);
		break;
	case 0x1a:
		// RR D
		registerDE.hi = RR(registerDE.hi);
		break;
	case 0x1b:
		// RR E
		registerDE.lo = RR(registerDE.lo);
		break;
	case 0x1c:
		// RR H
		registerHL.hi = RR(registerHL.hi);
		break;
	case 0x1d:
		// RR L
		registerHL.lo = RR(registerHL.lo);
		break;
	case 0x1e:
		// RR (HL)
		clock += 8;
		writeMemory(registerHL.pair, RR(readMemory(registerHL.pair)));
		break;
	case 0x1f:
		// RR A
		registerAF.hi = RR(registerAF.hi);
		break;
	case 0x20:
		// SLA B
		registerBC.hi = SLA(registerBC.hi);
		break;
	case 0x21:
		// SLA C
		registerBC.lo = SLA(registerBC.lo);
		break;
	case 0x22:
		// SLA D
		registerDE.hi = SLA(registerDE.hi);
		break;
	case 0x23:
		// SLA E
		registerDE.lo = SLA(registerDE.lo);
		break;
	case 0x24:
		// SLA H
		registerHL.hi = SLA(registerHL.hi);
		break;
	case 0x25:
		// SLA L
		registerHL.lo = SLA(registerHL.lo);
		break;
	case 0x26:
		// SLA (HL)
		clock += 8;
		writeMemory(registerHL.pair, SLA(readMemory(registerHL.pair)));
		break;
	case 0x27:
		// SLA A
		registerAF.hi = SLA(registerAF.hi);
		break;
	case 0x28:
		// SRA B
		registerBC.hi = SRA(registerBC.hi);
		break;
	case 0x29:
		// SRA C
		registerBC.lo = SRA(registerBC.lo);
		break;
	case 0x2a:
		// SRA D
		registerDE.hi = SRA(registerDE.hi);
		break;
	case 0x2b:
		// SRA E
		registerDE.lo = SRA(registerDE.lo);
		break;
	case 0x2c:
		// SRA H
		registerHL.hi = SRA(registerHL.hi);
		break;
	case 0x2d:
		// SRA L
		registerHL.lo = SRA(registerHL.lo);
		break;
	case 0x2e:
		// SRA (HL)
		clock += 8;
		writeMemory(registerHL.pair, SRA(readMemory(registerHL.pair)));
		break;
	case 0x2f:
		// SRA A
		registerAF.hi = SRA(registerAF.hi);
		break;
	case 0x30:
		// SWAP B
		registerBC.hi = SWAP(registerBC.hi);
		break;
	case 0x31:
		// SWAP C
		registerBC.lo = SWAP(registerBC.lo);
		break;
	case 0x32:
		// SWAP D
		registerDE.hi = SWAP(registerDE.hi);
		break;
	case 0x33:
		// SWAP E
		registerDE.lo = SWAP(registerDE.lo);
		break;
	case 0x34:
		// SWAP H
		registerHL.hi = SWAP(registerHL.hi);
		break;
	case 0x35:
		// SWAP L
		registerHL.lo = SWAP(registerHL.lo);
		break;
	case 0x36:
		// SWAP (HL)
		clock += 8;
		writeMemory(registerHL.pair, SWAP(readMemory(registerHL.pair)));
		break;
	case 0x37:
		// SWAP A
		registerAF.hi = SWAP(registerAF.hi);
		break;
	case 0x38:
		// SRL B
		registerBC.hi = SRL(registerBC.hi);
		break;
	case 0x39:
		// SRL C
		registerBC.lo = SRL(registerBC.lo);
		break;
	case 0x3a:
		// SRL D
		registerDE.hi = SRL(registerDE.hi);
		break;
	case 0x3b:
		// SRL E
		registerDE.lo = SRL(registerDE.lo);
		break;
	case 0x3c:
		// SRL H
		registerHL.hi = SRL(registerHL.hi);
		break;
	case 0x3d:
		// SRL L
		registerHL.lo = SRL(registerHL.lo);
		break;
	case 0x3e:
		// SRL (HL)
		clock += 8;
		writeMemory(registerHL.pair, SRL(readMemory(registerHL.pair)));
		break;
	case 0x3f:
		// SRL A
		registerAF.hi = SRL(registerAF.hi);
		break;
	case 0x40:
		// BIT 0 B
		BIT(BIT_0, registerBC.hi);
		break;
	case 0x41:
		// BIT 0 C
		BIT(BIT_0, registerBC.lo);
		break;
	case 0x42:
		// BIT 0 D
		BIT(BIT_0, registerDE.hi);
		break;
	case 0x43:
		// BIT 0 E
		BIT(BIT_0, registerDE.lo);
		break;
	case 0x44:
		// BIT 0 H
		BIT(BIT_0, registerHL.hi);
		break;
	case 0x45:
		// BIT 0 L
		BIT(BIT_0, registerHL.lo);
		break;
	case 0x46:
		// BIT 0 (HL)
		clock += 8;
		BIT(BIT_0, readMemory(registerHL.pair));
		break;
	case 0x47:
		// BIT 0 A
		BIT(BIT_0, registerAF.hi);
		break;
	case 0x48:
		// BIT 1 B
		BIT(BIT_1, registerBC.hi);
		break;
	case 0x49:
		// BIT 1 C
		BIT(BIT_1, registerBC.lo);
		break;
	case 0x4a:
		// BIT 1 D
		BIT(BIT_1, registerDE.hi);
		break;
	case 0x4b:
		// BIT 1 E
		BIT(BIT_1, registerDE.lo);
		break;
	case 0x4c:
		// BIT 1 H
		BIT(BIT_1, registerHL.hi);
		break;
	case 0x4d:
		// BIT 1 L
		BIT(BIT_1, registerHL.lo);
		break;
	case 0x4e:
		// BIT 1 (HL)
		clock += 8;
		BIT(BIT_1, readMemory(registerHL.pair));
		break;
	case 0x4f:
		// BIT 1 A
		BIT(BIT_1, registerAF.hi);
		break;
	case 0x50:
		// BIT 2 B
		BIT(BIT_2, registerBC.hi);
		break;
	case 0x51:
		// BIT 2 C
		BIT(BIT_2, registerBC.lo);
		break;
	case 0x52:
		// BIT 2 D
		BIT(BIT_2, registerDE.hi);
		break;
	case 0x53:
		// BIT 2 E
		BIT(BIT_2, registerDE.lo);
		break;
	case 0x54:
		// BIT 2 H
		BIT(BIT_2, registerHL.hi);
		break;
	case 0x55:
		// BIT 2 L
		BIT(BIT_2, registerHL.lo);
		break;
	case 0x56:
		// BIT 2 (HL)
		clock += 8;
		BIT(BIT_2, readMemory(registerHL.pair));
		break;
	case 0x57:
		// BIT 2 A
		BIT(BIT_2, registerAF.hi);
		break;
	case 0x58:
		// BIT 3 B
		BIT(BIT_3, registerBC.hi);
		break;
	case 0x59:
		// BIT 3 C
		BIT(BIT_3, registerBC.lo);
		break;
	case 0x5a:
		// BIT 3 D
		BIT(BIT_3, registerDE.hi);
		break;
	case 0x5b:
		// BIT 3 E
		BIT(BIT_3, registerDE.lo);
		break;
	case 0x5c:
		// BIT 3 H
		BIT(BIT_3, registerHL.hi);
		break;
	case 0x5d:
		// BIT 3 L
		BIT(BIT_3, registerHL.lo);
		break;
	case 0x5e:
		// BIT 3 (HL)
		clock += 8;
		BIT(BIT_3, readMemory(registerHL.pair));
		break;
	case 0x5f:
		// BIT 3 A
		BIT(BIT_3, registerAF.hi);
		break;
	case 0x60:
		// BIT 4 B
		BIT(BIT_4, registerBC.hi);
		break;
	case 0x61:
		// BIT 4 C
		BIT(BIT_4, registerBC.lo);
		break;
	case 0x62:
		// BIT 4 D
		BIT(BIT_4, registerDE.hi);
		break;
	case 0x63:
		// BIT 4 E
		BIT(BIT_4, registerDE.lo);
		break;
	case 0x64:
		// BIT 4 H
		BIT(BIT_4, registerHL.hi);
		break;
	case 0x65:
		// BIT 4 L
		BIT(BIT_4, registerHL.lo);
		break;
	case 0x66:
		// BIT 4 (HL)
		clock += 8;
		BIT(BIT_4, readMemory(registerHL.pair));
		break;
	case 0x67:
		// BIT 4 A
		BIT(BIT_4, registerAF.hi);
		break;
	case 0x68:
		// BIT 5 B
		BIT(BIT_5, registerBC.hi);
		break;
	case 0x69:
		// BIT 5 C
		BIT(BIT_5, registerBC.lo);
		break;
	case 0x6a:
		// BIT 5 D
		BIT(BIT_5, registerDE.hi);
		break;
	case 0x6b:
		// BIT 5 E
		BIT(BIT_5, registerDE.lo);
		break;
	case 0x6c:
		// BIT 5 H
		BIT(BIT_5, registerHL.hi);
		break;
	case 0x6d:
		// BIT 5 L
		BIT(BIT_5, registerHL.lo);
		break;
	case 0x6e:
		// BIT 5 (HL)
		clock += 8;
		BIT(BIT_5, readMemory(registerHL.pair));
		break;
	case 0x6f:
		// BIT 5 A
		BIT(BIT_5, registerAF.hi);
		break;
	case 0x70:
		// BIT 6 B
		BIT(BIT_6, registerBC.hi);
		break;
	case 0x71:
		// BIT 6 C
		BIT(BIT_6, registerBC.lo);
		break;
	case 0x72:
		// BIT 6 D
		BIT(BIT_6, registerDE.hi);
		break;
	case 0x73:
		// BIT 6 E
		BIT(BIT_6, registerDE.lo);
		break;
	case 0x74:
		// BIT 6 H
		BIT(BIT_6, registerHL.hi);
		break;
	case 0x75:
		// BIT 6 L
		BIT(BIT_6, registerHL.lo);
		break;
	case 0x76:
		// BIT 6 (HL)
		clock += 8;
		BIT(BIT_6, readMemory(registerHL.pair));
		break;
	case 0x77:
		// BIT 6 A
		BIT(BIT_6, registerAF.hi);
		break;
	case 0x78:
		// BIT 7 B
		BIT(BIT_7, registerBC.hi);
		break;
	case 0x79:
		// BIT 7 C
		BIT(BIT_7, registerBC.lo);
		break;
	case 0x7a:
		// BIT 7 D
		BIT(BIT_7, registerDE.hi);
		break;
	case 0x7b:
		// BIT 7 E
		BIT(BIT_7, registerDE.lo);
		break;
	case 0x7c:
		// BIT 7 H
		BIT(BIT_7, registerHL.hi);
		break;
	case 0x7d:
		// BIT 7 L
		BIT(BIT_7, registerHL.lo);
		break;
	case 0x7e:
		// BIT 7 (HL)
		clock += 8;
		BIT(BIT_7, readMemory(registerHL.pair));
		break;
	case 0x7f:
		// BIT 7 A
		BIT(BIT_7, registerAF.hi);
		break;
	case 0x80:
		// RES 0 B
		registerBC.hi = RES(BIT_0, registerBC.hi);
		break;
	case 0x81:
		// RES 0 C
		registerBC.lo = RES(BIT_0, registerBC.lo);
		break;
	case 0x82:
		// RES 0 D
		registerDE.hi = RES(BIT_0, registerDE.hi);
		break;
	case 0x83:
		// RES 0 E
		registerDE.lo = RES(BIT_0, registerDE.lo);
		break;
	case 0x84:
		// RES 0 H
		registerHL.hi = RES(BIT_0, registerHL.hi);
		break;
	case 0x85:
		// RES 0 L
		registerHL.lo = RES(BIT_0, registerHL.lo);
		break;
	case 0x86:
		// RES 0 (HL)
		clock += 8;
		writeMemory(registerHL.pair, RES(BIT_0, readMemory(registerHL.pair)));
		break;
	case 0x87:
		// RES 0 A
		registerAF.hi = RES(BIT_0, registerAF.hi);
		break;
	case 0x88:
		// RES 1 B
		registerBC.hi = RES(BIT_1, registerBC.hi);
		break;
	case 0x89:
		// RES 1 C
		registerBC.lo = RES(BIT_1, registerBC.lo);
		break;
	case 0x8a:
		// RES 1 D
		registerDE.hi = RES(BIT_1, registerDE.hi);
		break;
	case 0x8b:
		// RES 1 E
		registerDE.lo = RES(BIT_1, registerDE.lo);
		break;
	case 0x8c:
		// RES 1 H
		registerHL.hi = RES(BIT_1, registerHL.hi);
		break;
	case 0x8d:
		// RES 1 L
		registerHL.lo = RES(BIT_1, registerHL.lo);
		break;
	case 0x8e:
		// RES 1 (HL)
		clock += 8;
		writeMemory(registerHL.pair, RES(BIT_1, readMemory(registerHL.pair)));
		break;
	case 0x8f:
		// RES 1 A
		registerAF.hi = RES(BIT_1, registerAF.hi);
		break;
	case 0x90:
		// RES 2 B
		registerBC.hi = RES(BIT_2, registerBC.hi);
		break;
	case 0x91:
		// RES 2 C
		registerBC.lo = RES(BIT_2, registerBC.lo);
		break;
	case 0x92:
		// RES 2 D
		registerDE.hi = RES(BIT_2, registerDE.hi);
		break;
	case 0x93:
		// RES 2 E
		registerDE.lo = RES(BIT_2, registerDE.lo);
		break;
	case 0x94:
		// RES 2 H
		registerHL.hi = RES(BIT_2, registerHL.hi);
		break;
	case 0x95:
		// RES 2 L
		registerHL.lo = RES(BIT_2, registerHL.lo);
		break;
	case 0x96:
		// RES 2 (HL)
		clock += 8;
		writeMemory(registerHL.pair, RES(BIT_2, readMemory(registerHL.pair)));
		break;
	case 0x97:
		// RES 2 A
		registerAF.hi = RES(BIT_2, registerAF.hi);
		break;
	case 0x98:
		// RES 3 B
		registerBC.hi = RES(BIT_3, registerBC.hi);
		break;
	case 0x99:
		// RES 3 C
		registerBC.lo = RES(BIT_3, registerBC.lo);
		break;
	case 0x9a:
		// RES 3 D
		registerDE.hi = RES(BIT_3, registerDE.hi);
		break;
	case 0x9b:
		// RES 3 E
		registerDE.lo = RES(BIT_3, registerDE.lo);
		break;
	case 0x9c:
		// RES 3 H
		registerHL.hi = RES(BIT_3, registerHL.hi);
		break;
	case 0x9d:
		// RES 3 L
		registerHL.lo = RES(BIT_3, registerHL.lo);
		break;
	case 0x9e:
		// RES 3 (HL)
		clock += 8;
		writeMemory(registerHL.pair, RES(BIT_3, readMemory(registerHL.pair)));
		break;
	case 0x9f:
		// RES 3 A
		registerAF.hi = RES(BIT_3, registerAF.hi);
		break;
	case 0xa0:
		// RES 4 B
		registerBC.hi = RES(BIT_4, registerBC.hi);
		break;
	case 0xa1:
		// RES 4 C
		registerBC.lo = RES(BIT_4, registerBC.lo);
		break;
	case 0xa2:
		// RES 4 D
		registerDE.hi = RES(BIT_4, registerDE.hi);
		break;
	case 0xa3:
		// RES 4 E
		registerDE.lo = RES(BIT_4, registerDE.lo);
		break;
	case 0xa4:
		// RES 4 H
		registerHL.hi = RES(BIT_4, registerHL.hi);
		break;
	case 0xa5:
		// RES 4 L
		registerHL.lo = RES(BIT_4, registerHL.lo);
		break;
	case 0xa6:
		// RES 4 (HL)
		clock += 8;
		writeMemory(registerHL.pair, RES(BIT_4, readMemory(registerHL.pair)));
		break;
	case 0xa7:
		// RES 4 A
		registerAF.hi = RES(BIT_4, registerAF.hi);
		break;
	case 0xa8:
		// RES 5 B
		registerBC.hi = RES(BIT_5, registerBC.hi);
		break;
	case 0xa9:
		// RES 5 C
		registerBC.lo = RES(BIT_5, registerBC.lo);
		break;
	case 0xaa:
		// RES 5 D
		registerDE.hi = RES(BIT_5, registerDE.hi);
		break;
	case 0xab:
		// RES 5 E
		registerDE.lo = RES(BIT_5, registerDE.lo);
		break;
	case 0xac:
		// RES 5 H
		registerHL.hi = RES(BIT_5, registerHL.hi);
		break;
	case 0xad:
		// RES 5 L
		registerHL.lo = RES(BIT_5, registerHL.lo);
		break;
	case 0xae:
		// RES 5 (HL)
		clock += 8;
		writeMemory(registerHL.pair, RES(BIT_5, readMemory(registerHL.pair)));
		break;
	case 0xaf:
		// RES 5 A
		registerAF.hi = RES(BIT_5, registerAF.hi);
		break;
	case 0xb0:
		// RES 6 B
		registerBC.hi = RES(BIT_6, registerBC.hi);
		break;
	case 0xb1:
		// RES 6 C
		registerBC.lo = RES(BIT_6, registerBC.lo);
		break;
	case 0xb2:
		// RES 6 D
		registerDE.hi = RES(BIT_6, registerDE.hi);
		break;
	case 0xb3:
		// RES 6 E
		registerDE.lo = RES(BIT_6, registerDE.lo);
		break;
	case 0xb4:
		// RES 6 H
		registerHL.hi = RES(BIT_6, registerHL.hi);
		break;
	case 0xb5:
		// RES 6 L
		registerHL.lo = RES(BIT_6, registerHL.lo);
		break;
	case 0xb6:
		// RES 6 (HL)
		clock += 8;
		writeMemory(registerHL.pair, RES(BIT_6, readMemory(registerHL.pair)));
		break;
	case 0xb7:
		// RES 6 A
		registerAF.hi = RES(BIT_6, registerAF.hi);
		break;
	case 0xb8:
		// RES 7 B
		registerBC.hi = RES(BIT_7, registerBC.hi);
		break;
	case 0xb9:
		// RES 7 C
		registerBC.lo = RES(BIT_7, registerBC.lo);
		break;
	case 0xba:
		// RES 7 D
		registerDE.hi = RES(BIT_7, registerDE.hi);
		break;
	case 0xbb:
		// RES 7 E
		registerDE.lo = RES(BIT_7, registerDE.lo);
		break;
	case 0xbc:
		// RES 7 H
		registerHL.hi = RES(BIT_7, registerHL.hi);
		break;
	case 0xbd:
		// RES 7 L
		registerHL.lo = RES(BIT_7, registerHL.lo);
		break;
	case 0xbe:
		// RES 7 (HL)
		clock += 8;
		writeMemory(registerHL.pair, RES(BIT_7, readMemory(registerHL.pair)));
		break;
	case 0xbf:
		// RES 7 A
		registerAF.hi = RES(BIT_7, registerAF.hi);
		break;
	case 0xc0:
		// SET 0 B
		registerBC.hi = SET(BIT_0, registerBC.hi);
		break;
	case 0xc1:
		// SET 0 C
		registerBC.lo = SET(BIT_0, registerBC.lo);
		break;
	case 0xc2:
		// SET 0 D
		registerDE.hi = SET(BIT_0, registerDE.hi);
		break;
	case 0xc3:
		// SET 0 E
		registerDE.lo = SET(BIT_0, registerDE.lo);
		break;
	case 0xc4:
		// SET 0 H
		registerHL.hi = SET(BIT_0, registerHL.hi);
		break;
	case 0xc5:
		// SET 0 L
		registerHL.lo = SET(BIT_0, registerHL.lo);
		break;
	case 0xc6:
		// SET 0 (HL)
		clock += 8;
		writeMemory(registerHL.pair, SET(BIT_0, readMemory(registerHL.pair)));
		break;
	case 0xc7:
		// SET 0 A
		registerAF.hi = SET(BIT_0, registerAF.hi);
		break;
	case 0xc8:
		// SET 1 B
		registerBC.hi = SET(BIT_1, registerBC.hi);
		break;
	case 0xc9:
		// SET 1 C
		registerBC.lo = SET(BIT_1, registerBC.lo);
		break;
	case 0xca:
		// SET 1 D
		registerDE.hi = SET(BIT_1, registerDE.hi);
		break;
	case 0xcb:
		// SET 1 E
		registerDE.lo = SET(BIT_1, registerDE.lo);
		break;
	case 0xcc:
		// SET 1 H
		registerHL.hi = SET(BIT_1, registerHL.hi);
		break;
	case 0xcd:
		// SET 1 L
		registerHL.lo = SET(BIT_1, registerHL.lo);
		break;
	case 0xce:
		// SET 1 (HL)
		clock += 8;
		writeMemory(registerHL.pair, SET(BIT_1, readMemory(registerHL.pair)));
		break;
	case 0xcf:
		// SET 1 A
		registerAF.hi = SET(BIT_1, registerAF.hi);
		break;
	case 0xd0:
		// SET 2 B
		registerBC.hi = SET(BIT_2, registerBC.hi);
		break;
	case 0xd1:
		// SET 2 C
		registerBC.lo = SET(BIT_2, registerBC.lo);
		break;
	case 0xd2:
		// SET 2 D
		registerDE.hi = SET(BIT_2, registerDE.hi);
		break;
	case 0xd3:
		// SET 2 E
		registerDE.lo = SET(BIT_2, registerDE.lo);
		break;
	case 0xd4:
		// SET 2 H
		registerHL.hi = SET(BIT_2, registerHL.hi);
		break;
	case 0xd5:
		// SET 2 L
		registerHL.lo = SET(BIT_2, registerHL.lo);
		break;
	case 0xd6:
		// SET 2 (HL
		clock += 8;
		writeMemory(registerHL.pair, SET(BIT_2, readMemory(registerHL.pair)));
		break;
	case 0xd7:
		// SET 2 A
		registerAF.hi = SET(BIT_2, registerAF.hi);
		break;
	case 0xd8:
		// SET 3 B
		registerBC.hi = SET(BIT_3, registerBC.hi);
		break;
	case 0xd9:
		// SET 3 C
		registerBC.lo = SET(BIT_3, registerBC.lo);
		break;
	case 0xda:
		// SET 3 D
		registerDE.hi = SET(BIT_3, registerDE.hi);
		break;
	case 0xdb:
		// SET 3 E
		registerDE.lo = SET(BIT_3, registerDE.lo);
		break;
	case 0xdc:
		// SET 3 H
		registerHL.hi = SET(BIT_3, registerHL.hi);
		break;
	case 0xdd:
		// SET 3 L
		registerHL.lo = SET(BIT_3, registerHL.lo);
		break;
	case 0xde:
		// SET 3 (HL)
		clock += 8;
		writeMemory(registerHL.pair, SET(BIT_3, readMemory(registerHL.pair)));
		break;
	case 0xdf:
		// SET 3 A
		registerAF.hi = SET(BIT_3, registerAF.hi);
		break;
	case 0xe0:
		// SET 4 B
		registerBC.hi = SET(BIT_4, registerBC.hi);
		break;
	case 0xe1:
		// SET 4 C
		registerBC.lo = SET(BIT_4, registerBC.lo);
		break;
	case 0xe2:
		// SET 4 D
		registerDE.hi = SET(BIT_4, registerDE.hi);
		break;
	case 0xe3:
		// SET 4 E
		registerDE.lo = SET(BIT_4, registerDE.lo);
		break;
	case 0xe4:
		// SET 4 H
		registerHL.hi = SET(BIT_4, registerHL.hi);
		break;
	case 0xe5:
		// SET 4 L
		registerHL.lo = SET(BIT_4, registerHL.lo);
		break;
	case 0xe6:
		// SET 4 (HL)
		clock += 8;
		writeMemory(registerHL.pair, SET(BIT_4, readMemory(registerHL.pair)));
		break;
	case 0xe7:
		// SET 4 A
		registerAF.hi = SET(BIT_4, registerAF.hi);
		break;
	case 0xe8:
		// SET 5 B
		registerBC.hi = SET(BIT_5, registerBC.hi);
		break;
	case 0xe9:
		// SET 5 C
		registerBC.lo = SET(BIT_5, registerBC.lo);
		break;
	case 0xea:
		// SET 5 D
		registerDE.hi = SET(BIT_5, registerDE.hi);
		break;
	case 0xeb:
		// SET 5 E
		registerDE.lo = SET(BIT_5, registerDE.lo);
		break;
	case 0xec:
		// SET 5 H
		registerHL.hi = SET(BIT_5, registerHL.hi);
		break;
	case 0xed:
		// SET 5 L
		registerHL.lo = SET(BIT_5, registerHL.lo);
		break;
	case 0xee:
		// SET 5 (HL)
		clock += 8;
		writeMemory(registerHL.pair, SET(BIT_5, readMemory(registerHL.pair)));
		break;
	case 0xef:
		// SET 5 A
		registerAF.hi = SET(BIT_5, registerAF.hi);
		break;
	case 0xf0:
		// SET 6 B
		registerBC.hi = SET(BIT_6, registerBC.hi);
		break;
	case 0xf1:
		// SET 6 C
		registerBC.lo = SET(BIT_6, registerBC.lo);
		break;
	case 0xf2:
		// SET 6 D
		registerDE.hi = SET(BIT_6, registerDE.hi);
		break;
	case 0xf3:
		// SET 6 E
		registerDE.lo = SET(BIT_6, registerDE.lo);
		break;
	case 0xf4:
		// SET 6 H
		registerHL.hi = SET(BIT_6, registerHL.hi);
		break;
	case 0xf5:
		// SET 6 L
		registerHL.lo = SET(BIT_6, registerHL.lo);
		break;
	case 0xf6:
		// SET 6 (HL)
		clock += 8;
		writeMemory(registerHL.pair, SET(BIT_6, readMemory(registerHL.pair)));
		break;
	case 0xf7:
		// SET 6 A
		registerAF.hi = SET(BIT_6, registerAF.hi);
		break;
	case 0xf8:
		// SET 7 B
		registerBC.hi = SET(BIT_7, registerBC.hi);
		break;
	case 0xf9:
		// SET 7 C
		registerBC.lo = SET(BIT_7, registerBC.lo);
		break;
	case 0xfa:
		// SET 7 D
		registerDE.hi = SET(BIT_7, registerDE.hi);
		break;
	case 0xfb:
		// SET 7 E
		registerDE.lo = SET(BIT_7, registerDE.lo);
		break;
	case 0xfc:
		// SET 7 H
		registerHL.hi = SET(BIT_7, registerHL.hi);
		break;
	case 0xfd:
		// SET 7 L
		registerHL.lo = SET(BIT_7, registerHL.lo);
		break;
	case 0xfe:
		// SET 7 (HL)
		clock += 8;
		writeMemory(registerHL.pair, SET(BIT_7, readMemory(registerHL.pair)));
		break;
	case 0xff:
		// SET 7 A
		registerAF.hi = SET(BIT_7, registerAF.hi);
		break;
	}
}

void CALL_Z(WORD operand)
{
	if (isFlagSet(FLAG_Z))
	{
		CALL(operand);
	}
	else
	{
		clock += 12;
	}
}

void CALL(WORD operand)
{
	clock += 24;

	PC.pair++;
	pushStack(PC.pair);
	JP(operand);
}

void RST_08()
{
	PC.pair++;
	pushStack(PC.pair);
	JP(0x0008);
}

void RET_NC()
{
	if (!isFlagSet(FLAG_C))
	{
		clock += 4;
		RET();
	}
	else
	{
		clock += 8;
	}
}

void POP_DE()
{
	clock += 12;
	LD_16(&registerDE, popStack());
}

void JP_NC(WORD operand)
{
	if (!isFlagSet(FLAG_C))
	{
		JP(operand);
	}
	else
	{
		clock += 12;
	}
}

void CALL_NC(WORD operand)
{
	if (!isFlagSet(FLAG_C))
	{
		CALL(operand);
	}
	else
	{
		clock += 12;
	}
}

void PUSH_DE()
{
	clock += 8;
	pushStack(registerDE.pair);
}

void RST_10()
{
	PC.pair++;
	pushStack(PC.pair);
	JP(0x0010);
}

void RET_C()
{
	if (isFlagSet(FLAG_C))
	{
		clock += 4;
		RET();
	}
	else
	{
		clock += 8;
	}
}

void RETI()
{
	WORD operand = popStack();
	JP(operand);
	interrupt.master = 1;
}

void JP_C(WORD operand)
{
	if (isFlagSet(FLAG_C))
	{
		JP(operand);
	}
	else
	{
		clock += 12;
	}
}

void CALL_C(WORD operand)
{
	if (isFlagSet(FLAG_C))
	{
		CALL(operand);
	}
	else
	{
		clock += 12;
	}
}

void RST_18()
{
	PC.pair++;
	pushStack(PC.pair);
	JP(0x0018);
}

void LD_FF02X_A(BYTE operand)
{
	clock += 12;
	writeMemory(0xFF00 + operand, registerAF.hi);
}

void POP_HL()
{
	clock += 12;
	LD_16(&registerHL, popStack());
}

void LD_FFC_A()
{
	clock += 8;
	writeMemory(0xFF00 + registerBC.lo, registerAF.hi);
}

void PUSH_HL()
{
	clock += 16;
	pushStack(registerHL.pair);
}

void RST_20()
{
	PC.pair++;
	pushStack(PC.pair);
	JP(0x0020);
}

void ADD_SP(SIGNED_BYTE operand)
{
	clock += 16;

	clearFlag(FLAG_N);
	clearFlag(FLAG_Z);

	int result = SP.pair + operand;
	int result_flag = (SP.pair & 0xFF) + operand;

	if (result_flag & 0xFF00)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	if ((((result_flag & 0xF) + (operand & 0xF)) & 0x10) == 0x10)
	{
		setFlag(FLAG_H);
	}
	else
	{
		clearFlag(FLAG_H);
	}

	SP.pair = (WORD)(result & 0xFFFF);
}

void JP_HL()
{
	clock += 4;
	PC.pair = registerHL.pair - 1;
}

void LD_04X_A(WORD operand)
{
	clock += 16;
	writeMemory(operand, registerAF.hi);
}

void RST_28()
{
	PC.pair++;
	pushStack(PC.pair);
	JP(0x0028);
}

void LD_A_FF02X(BYTE operand)
{
	clock += 12;
	LD(&registerAF.hi, readMemory(0xFF00 + operand));
}

void POP_AF()
{
	clock += 12;
	WORD val = popStack();
	val &= 0xFFF0;

	LD_16(&registerAF, val);
}

void LD_A_FFC()
{
	clock += 8;
	LD(&registerAF.hi, readMemory(0xFF00 + registerBC.lo));
}

void DI()
{
	clock += 4;
	interrupt.timer = 0x00;
}

void PUSH_AF()
{
	clock += 16;
	pushStack(registerAF.pair);
}

void RST_30()
{
	PC.pair++;
	pushStack(PC.pair);
	JP(0x0030);
}

void LD_HL_SP02X(SIGNED_BYTE operand)
{
	clock += 12;

	clearFlag(FLAG_Z);
	clearFlag(FLAG_N);

	int result = SP.pair + operand;
	int result_flag = (SP.pair & 0xFF) + operand;

	if (result_flag & 0xFF00)
	{
		setFlag(FLAG_C);
	}
	else
	{
		clearFlag(FLAG_C);
	}

	if ((((SP.pair & 0xF) + (operand & 0xF)) & 0x10) == 0x10)
	{
		setFlag(FLAG_H);
	}
	else
	{
		clearFlag(FLAG_H);
	}

	LD_16(&registerHL, (WORD)(result & 0xFFFF));
}

void LD_SP_HL()
{
	clock += 8;
	LD_16(&SP, registerHL.pair);
}

void LD_A_WORD(WORD operand)
{
	clock += 16;
	LD(&registerAF.hi, readMemory(operand));
}

void EI()
{
	clock += 4;
	interrupt.timer = 0x01;
}

void RST_38()
{
	PC.pair++;
	pushStack(PC.pair);;
	JP(0x0038);
}

void INC_HL_P()
{
	clock += 12;
	writeMemory(registerHL.pair, INC(readMemory(registerHL.pair)));
}

void DEC_HL_P()
{
	clock += 12;
	writeMemory(registerHL.pair, DEC(readMemory(registerHL.pair)));
}
