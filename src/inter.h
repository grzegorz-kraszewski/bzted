/* definitions for intermediate code */

/* codes */

#define II_MOVE   1
#define II_COPY   2
#define II_PUSH   3
#define II_PULL   4
#define II_DROP   5
#define II_ADDL   6
#define II_SUBL   7
#define II_JSBR   8
#define II_RETN   9
#define II_NOTL  10
#define II_ANDL  11
#define II_ORRL  12
#define II_EORL  13

/* registers (for arg and out) */

#define II_D     1
#define II_A     257

/* flags */

#define II_IMMEDIATE  0x00000001

#define IIOP_NONE        0
#define IIOP_REGISTER    1
#define IIOP_IMMEDIATE   2  

struct Operand
{
	int type;     /* one of IIOP_ */
	int value;
};


class InterInstruction
{
	public:

	InterInstruction *succ;
	InterInstruction *pred;
	int code;
	Operand arg;
	Operand out;
	const char *label;
	int flags;

	InterInstruction(int c)
	{
		code = c;
		arg.type = IIOP_NONE;
		arg.value = 0;
		out.type = IIOP_NONE;
		out.type = 0;
		label = NULL;
		flags = 0;
	}

	InterInstruction(int c, Operand &o)
	{ 
		code = c;
		arg.type = IIOP_NONE;
		arg.value = 0;
		out = o;
		label = NULL;
		flags = 0;
	} 

	InterInstruction(int c, Operand &a, Operand &o)
	{ 
		code = c;
		arg = a;
		out = o;
		label = NULL;
		flags = 0;
	}

	InterInstruction(int c, const char *l)
	{
		code = c;
		arg.type = IIOP_NONE;
		arg.value = 0;
		out.type = IIOP_NONE;
		out.type = 0;
		label = l;
		flags = 0;
	}

	void print();
	BOOL isDyadic();
};
