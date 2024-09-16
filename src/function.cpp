/* User defined function. */

#include "main.h"
#include "compiler.h"

#include <proto/dos.h>

extern Compiler *Comp;

void Function::print()
{
	Printf("%s: (%ld/%ld)\n", name, numArguments, numResults);
	for (InterInstruction *ii = code.first(); ii; ii = ii->next()) ii->print();
};

//---------------------------------------------------------------------------------------------

void Function::stackSignature()
{
	int stackBalance = 0, pullDepth = 0;

	maxStackDepth = 0;

	for (InterInstruction *ii = code.first(); ii; ii = ii->next())
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
			Function *f = Comp->findFunction(ii->label);
			stackBalance -= f->numArguments;
			stackBalance += f->numResults;
			if (f->numArguments > maxStackDepth) maxStackDepth = f->numArguments;
			if (f->numResults > maxStackDepth) maxStackDepth = f->numResults;
		}

		if (stackBalance < pullDepth) pullDepth = stackBalance;
	}

	numArguments = -pullDepth;
	numResults = numArguments + stackBalance;
	if (numArguments > maxStackDepth) maxStackDepth = numArguments;
	if (numResults > maxStackDepth) maxStackDepth = numResults;
	
}

//---------------------------------------------------------------------------------------------


void Function::expand()
{
	InterInstruction *retn;
	int regnum;

	for (regnum = numArguments; regnum > 0; regnum--)
	{
		Operand opr(IIOP_FARGUMENT, regnum - 1);
		InterInstruction *ii = new InterInstruction(II_DROP, opr);
		if (ii) code.addHead(ii);
	}

	retn = code.remTail();

	for (regnum = numResults; regnum > 0; regnum--)
	{
		Operand opr(IIOP_FRESULT, regnum - 1);
		InterInstruction *ii = new InterInstruction(II_PULL, opr);
		if (ii) code.addTail(ii);
	}

	code.addTail(retn);
}

//---------------------------------------------------------------------------------------------
 
void Function::expandAllCalls()
{
	for(InterInstruction *ii = code.first(); ii; ii = ii->next())
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

	called = Comp->findFunction(call->label);

	for (regnum = called->numArguments; regnum > 0; regnum--)
	{
		Operand opr(IIOP_CARGUMENT, regnum - 1);
		ii = new InterInstruction(II_PULL, opr);
		if (ii) ii->insertBefore(call);
	}

	for (regnum = called->numResults; regnum > 0; regnum--)
	{
		Operand opr(IIOP_CRESULT, regnum - 1);
		ii = new InterInstruction(II_DROP, opr);
		if (ii) ii->insertAfter(call);
	}	
}

//---------------------------------------------------------------------------------------------

InterInstruction* Function::findPushPullBlock(PushPullBlock &ppblock)
{
	ppblock.push = NULL;
	ppblock.pull = NULL;
	InterInstruction *ii = code.first();
	
	while (ii)
	{
		if (ii->code == II_PULL)
		{
			if (ppblock.push)
			{
				ppblock.pull = ii;
				return ii->next();
			}
		}
		else if ((ii->code == II_DROP) || (ii->code == II_PUSH))
		{
			ppblock.push = ii;
		}

		ii = ii->next();
	}

	return NULL;
}

//---------------------------------------------------------------------------------------------

void Function::replacePushPullBlock(PushPullBlock &ppblock)
{
	int newop = (ppblock.push->code == II_DROP) ? II_MOVE : II_COPY;
	ppblock.push->code = newop;
	ppblock.push->arg = ppblock.push->out; 
	ppblock.push->out.type = IIOP_VIRTUAL;
	ppblock.push->out.value = firstFreeRegister;

	ppblock.pull->code = II_MOVE;
	ppblock.pull->arg.type = IIOP_VIRTUAL;
	ppblock.pull->arg.value = firstFreeRegister++;
}

//---------------------------------------------------------------------------------------------

int Function::replaceAllPushPullBlocks()
{
	int replacedBlocks = 0;
	InterInstruction *ii;
	PushPullBlock ppblock;

	firstFreeRegister = maxStackDepth;

	while (ii = findPushPullBlock(ppblock))
	{
		replacePushPullBlock(ppblock); 
		replacedBlocks++;
	}

	return replacedBlocks;
}
