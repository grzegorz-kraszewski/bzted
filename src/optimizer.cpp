//-----------------------------
// intermediate code optimizer
//-----------------------------

// Curently all optimizer functions are methods of Function class, but have been placed in
// a separate file.

#include "main.h"
#include "function.h"

InterInstruction* Function::lookFor(int instruction, InterInstruction *from)
{
	do
	{
		if (from->code == instruction) return from;
	}
	while (from = code.next(from));
 
	return NULL;
}

BOOL Function::optimize()
{
	InterInstruction *drop, *pull;
	int dropRegister, pullRegister;

	drop = lookFor(II_DROP, code.first());
	pull = lookFor(II_PULL, drop);

	if (drop && pull)
	{
		Printf("optimizer: %08lx DROP d%ld; %08lx PULL d%ld in %s().\n", drop, drop->out - 1, pull, 
			pull->out - 1, name);
	}

	return FALSE;
}