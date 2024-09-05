/* definitions for intermediate code */

#ifndef BZTED_INTER_H
#define BZTED_INTER_H

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


class InterInstruction
{
	public:

	InterInstruction *succ;
	InterInstruction *pred;
	int code;
	int arg;
	int out;
	const char *label;
	int flags;

	InterInstruction(int c)
	{
		code = c; arg = 0; out = 0; label = NULL; flags = 0;
	}

	InterInstruction(int c, int o)
	{ 
		code = c; arg = NULL; out = o; label = NULL; flags = 0;
	} 

	InterInstruction(int c, int a, int o)
	{ 
		code = c; arg = a, out = o; label = NULL; flags = 0;
	}

	InterInstruction(int c, int a, int o, int f)
	{
		code = c; arg = a, out = o; label = NULL; flags = f;
	}

	InterInstruction(int c, const char *l)
	{
		code = c; arg = 0, out = 0; label = l; flags = 0;
	}

	void print();
};

/*
  Virtual stack and register instructions:

  DROP - pushes a register on the stack, value in register is no more used.
  PUSH - pushes a register on the stack, but value in register will be reused.
  PULL - pulls top of stack into a register, top of stack is removed.
  MOVE - moves register x to register y, discarding value in register x (no more used).
  COPY - moves register x to register y, but register x value stays in place and will be reused.

*/

#endif  /* BZTED_INTER_H */
