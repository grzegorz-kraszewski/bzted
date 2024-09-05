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

	for (regnum = numArguments; regnum > 0; regnum--)
	{
		Operand opr = { IIOP_REGISTER, II_D + regnum - 1 };
		InterInstruction *ii = new InterInstruction(II_DROP, opr);
		if (ii) code.addhead(ii);
	}

	retn = code.remtail();

	for (regnum = numResults; regnum > 0; regnum--)
	{
		Operand opr = { IIOP_REGISTER, II_D + regnum - 1 };
		InterInstruction *ii = new InterInstruction(II_PULL, opr);
		if (ii) code.addtail(ii);
	}

	code.addtail(retn);
}

//---------------------------------------------------------------------------------------------
 
void Function::expandAllCalls()
{
	SysListIter<InterInstruction> insns(code);
	InterInstruction *ii;

	while(ii = insns++)
	{
		if (ii->code == II_JSBR) expandCall(ii);
	}
}

//---------------------------------------------------------------------------------------------

void Function::expandCall(InterInstruction *call)
{
	InterInstruction *ii;
	Function *called;
	int regnum;

	called = Comp->functions.find(call->label);

	for (regnum = called->numArguments; regnum > 0; regnum--)
	{
		Operand opr = { IIOP_REGISTER, II_D + regnum - 1 };
		ii = new InterInstruction(II_PULL, opr);
		if (ii) code.insertBefore(call, ii);
	}

	for (regnum = called->numResults; regnum > 0; regnum--)
	{
		Operand opr = { IIOP_REGISTER, II_D + regnum - 1 };
		ii = new InterInstruction(II_DROP, opr);
		if (ii) code.insertAfter(call, ii);
	}	
}
