/* User defined function. */

#include "main.h"
#include "compiler.h"

#include <proto/dos.h>

extern Compiler *Comp;

void Function::print()
{
	SysListIter<InterInstruction> iii(code);
	InterInstruction *ii;

	Printf("%s: (%ld/%ld)\n", name, numArguments, numResults);
	while (ii = iii++) ii->print();
};

//---------------------------------------------------------------------------------------------

void Function::stackSignature()
{
	InterInstruction *ii;
	SysListIter<InterInstruction> insns(code);

	int stackBalance = 0, pullDepth = 0;

	while (ii = insns++)
	{
		if (ii->code == II_PULL)
		{
			stackBalance--;
		}
		else if ((ii->code == II_PUSH) || (ii->code == II_DROP))
		{
			stackBalance++;
		}
		else if (ii->code == II_JSBR)
		{
			Function *f = Comp->functions.find(ii->label);
			stackBalance -= f->numArguments;
			stackBalance += f->numResults;
		}

		if (stackBalance < pullDepth) pullDepth = stackBalance;
	}

	numArguments = -pullDepth;
	numResults = numArguments + stackBalance;
}

//---------------------------------------------------------------------------------------------

void Function::expand()
{
	InterInstruction *retn;
	int regnum;

	Printf("expanding '%s', (%ld/%ld).\n", name, numArguments, numResults);
	for (regnum = numArguments; regnum > 0; regnum--)
	{
		InterInstruction *ii = new InterInstruction(II_DROP, II_D + regnum - 1);
		if (ii) code.addhead(ii);
	}

	retn = code.remtail();

	for (regnum = numResults; regnum > 0; regnum--)
	{
		InterInstruction *ii = new InterInstruction(II_PULL, II_D + regnum - 1);
		if (ii) code.addtail(ii);
	}

	code.addtail(retn);
}

//---------------------------------------------------------------------------------------------
 
void Function::expandCalls()
{
}
