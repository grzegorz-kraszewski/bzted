/* definitions for intermediate code */

#ifndef BZTED_INTER_H
#define BZTED_INTER_H

#include <dos/dos.h>

#include "rplist.h"

/* codes */

#define II_MOVE   1
#define II_COPY   2
#define II_DMOV   3
#define II_DCPY   4
#define II_PUSH   5
#define II_PULL   6
#define II_DROP   7
#define II_ADDL   8
#define II_SUBL   9
#define II_JSBR  10
#define II_RETN  11
#define II_NOTL  12
#define II_ANDL  13
#define II_ORRL  14
#define II_EORL  15
#define II_LDEA  16


#define II_INSTRUCTION_COUNT 16

/* flags */

#define IIOP_NONE        0
#define IIOP_VIRTUAL     1
#define IIOP_EDGE        2
#define IIOP_FARGUMENT   3
#define IIOP_FRESULT     4
#define IIOP_CARGUMENT   5
#define IIOP_CRESULT     6
#define IIOP_DATAREG     7
#define IIOP_ADDRREG     8
#define IIOP_IMMEDIATE   9 
#define IIOP_MEMREG     10
#define IIOP_FRAME      11
#define IIOP_LABEL      12
#define IIOP_SYSJUMP    13

/* errors of generator */

#define EGEN_INSTRUCTION_NOT_IMPLEMENTED   1


extern const char* CodeNames[II_INSTRUCTION_COUNT + 1];

class Operand
{
	public:

	int type;     /* one of IIOP_ */
	int value;

	Operand() { type = IIOP_NONE; value = 0; }
	Operand(int t, int v) { type = t; value = v; }
	bool operator==(const Operand &op) const { return ((type == op.type) && (value == op.value)); }
	char* makeString(char *dest);
};


class InterInstruction : public RpNode<InterInstruction>
{
	public:

	int code;
	Operand arg;
	Operand out;
	int flags;

	InterInstruction(int c) : arg(), out()
	{
		code = c;
		flags = 0;
	}

	InterInstruction(int c, Operand &o) : arg(), out(o)
	{ 
		code = c;
		flags = 0;
	} 

	InterInstruction(int c, Operand &a, Operand &o) : arg(a), out(o)
	{ 
		code = c;
		flags = 0;
	}

	bool isDyadic();
	void print();
	void emit(BPTR outFile, const char* instruction);
	void emit(BPTR outFile, const char* instruction, const char* operand1);
	void emit(BPTR outFile, const char* instruction, const char* operand1, const char *operand2);
	int generate(BPTR outFile);
	void generateOperand(const Operand &opr, char *buffer);
};

#endif  /* BZTED_INTER_H */
