/* User defined function. */

#include "main.h"
#include "compiler.h"
#include "logger.h"

#include <proto/dos.h>

extern Compiler *Comp;

void Function::print()
{
	if (resultsToFrame)
	{
		Printf("%s: (%ld/%ld), frame[%ld]\n", name(), numArguments, numResults, frameSize);
	}
	else
	{
		Printf("%s: (%ld/%ld)\n", name(), numArguments, numResults);
	}
	for (InterInstruction *ii = code.first(); ii; ii = ii->next()) ii->print();
};

#define KNOWN_TYPES "bBwWlLpsx"

#define FS_STATE_SEARCH   0
#define FS_STATE_ARGTYPE  1
#define FS_STATE_RESTYPE  2

//---------------------------------------------------------------------------------------------
// Function signature is appended to the name and is defined as "(<argtypes>.<restypes>)".
// parseSignature() extract pointer to start of 'argtypes', pointer to start of 'restypes',
// counts arguments and results (these numbers will be later compared to calculated stack
// signature.

bool Function::parseSignature()
{
	const char *p = name();
	char x, state;

	state = FS_STATE_SEARCH;

	while (x = *p++)
	{
		switch (state)
		{
			case FS_STATE_SEARCH:
				if (x == '(')
				{
					argumentTypes = p;
					state = FS_STATE_ARGTYPE;
				}
			break;

			case FS_STATE_ARGTYPE:
				if (IsInString(x, KNOWN_TYPES)) numArguments++;
				else if (x == '.')
				{
					resultTypes = p;
					state = FS_STATE_RESTYPE;
				}
				else
				{
					if (x == ')') log.lineError(lineNum, "missing dot in signature of '%s'",
					 name());
					else log.lineError(lineNum, "unknown type '%lc' specified in arguments "
					 "of '%s'", x, name());
					return FALSE;
				}
			break;

			case FS_STATE_RESTYPE:
				if (IsInString(x, KNOWN_TYPES)) numResults++;
				else if (x == ')')
				{
					if (*p == 0x00) return TRUE;
					else
					{
						log.lineError(lineNum, "unexpected character '%lc' after signature "
						 "of '%s'", *p, name());
						return FALSE;
					}
				}
				else
				{
					log.lineError(lineNum, "unknown type '%lc' specified in results of '%s'",
					 x, name());
					return FALSE;
				}
			break;
		}
	}

	log.lineError(lineNum, "missing or incomplete signature in definition of '%s'", name());
	return FALSE;
}

//---------------------------------------------------------------------------------------------
// Calculates quantitative stack signature of the function. Then compares it with signature
// declared in function definition. Any discrepancy is reported as error. 

bool Function::stackSignature()
{
	bool signatureMatch = TRUE;
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
			int realArguments, realResults;

			if (ii->out.type == IIOP_LABEL)
			{
				Function *f = Comp->findFunction((const char*)ii->out.value);
				realResults = f->toFrame() ? 1 : f->numResults;
				realArguments = f->numArguments;
			}
			else if (ii->out.type == IIOP_SYSJUMP)
			{
				SysCall *sc = Comp->findSysCall((const char*)ii->out.value);
				realArguments = StrLen(sc->arguments) >> 1;
				realResults = StrLen(sc->results) >> 1;
			}
			else return FALSE;

			stackBalance -= realArguments;
			stackBalance += realResults;
			if (realArguments > maxStackDepth) maxStackDepth = realArguments;
			if (realResults > maxStackDepth) maxStackDepth = realResults;
		}

		if (stackBalance < pullDepth) pullDepth = stackBalance;
	}

	int calculatedArguments = -pullDepth;
	int calculatedResults = calculatedArguments + stackBalance;
	if (calculatedArguments > maxStackDepth) maxStackDepth = calculatedArguments;
	if (calculatedResults > maxStackDepth) maxStackDepth = calculatedResults;

	if (numArguments != calculatedArguments)
	{
		log.lineError(lineNum, "function '%s', declared number of arguments (%ld) does not "
		 "match real one (%ld)", name(), numArguments, calculatedArguments);
		signatureMatch = FALSE;
	}
	
	if (numResults != calculatedResults)
	{
		log.lineError(lineNum, "function '%s', declared number of results (%ld) does not "
		 "match real one (%ld)", name(), numResults, calculatedResults);
		signatureMatch = FALSE;
	}
	
	return signatureMatch;
}

//---------------------------------------------------------------------------------------------

bool Function::expand()
{
	InterInstruction *retn;
	int regnum;

	if (resultsToFrame)
	{
		if (numResults > 0) { frameSize = numResults;
		Printf("expanding %s(), frame size set to %ld\n", name(), frameSize); }
		else
		{
			log.error("code block %s() generates empty data frame", name());
			return FALSE;
		}
	}
	
	for (regnum = numArguments; regnum > 0; regnum--)
	{
		Operand opr(IIOP_FARGUMENT, regnum - 1);
		InterInstruction *ii = new InterInstruction(II_DROP, opr);
		if (ii) code.addHead(ii);
		else return FALSE;
	}

	retn = code.remTail();

	for (regnum = numResults; regnum > 0; regnum--)
	{
		Operand opr(resultsToFrame ? IIOP_FRAME : IIOP_FRESULT, regnum - 1);
		InterInstruction *ii = new InterInstruction(II_PULL, opr);
		if (ii) code.addTail(ii);
		else return FALSE;
	}

	//-----------------------------------------------------------------------------------
	// For data frame functions return A4 contents as the result. Then replace number of
	// virtual results (which go into the data frame) with 1 (frame address).
	//-----------------------------------------------------------------------------------
	
	if (resultsToFrame)
	{
		Operand op1(IIOP_ADDRREG, 4);
		Operand op2(IIOP_FRESULT, 0);
		InterInstruction *ii;
		ii = new InterInstruction(II_DROP, op1);
		if (ii) code.addTail(ii);
		else return FALSE;
		ii = new InterInstruction(II_PULL, op2);
		if (ii) code.addTail(ii);
		else return FALSE;
		numResults = 1;
	}
	
	code.addTail(retn);
	return TRUE;
}

//---------------------------------------------------------------------------------------------
 
void Function::expandAllCalls()
{
	for(InterInstruction *ii = code.first(); ii; ii = ii->next())
	{
		if (ii->code == II_JSBR)
		{
			if (ii->out.type == IIOP_LABEL) expandCall(ii);
			else if (ii->out.type == IIOP_SYSJUMP) expandSysCall(ii);
		}
	}
}

//---------------------------------------------------------------------------------------------

bool Function::expandCall(InterInstruction *call)
{
	InterInstruction *ii;
	Function *called;
	int regnum;

	called = Comp->findFunction((const char*)call->out.value);

	for (regnum = called->numArguments; regnum > 0; regnum--)
	{
		Operand opr(IIOP_CARGUMENT, regnum - 1);
		ii = new InterInstruction(II_PULL, opr);
		if (ii) ii->insertBefore(call);
		else return FALSE;
	}

	//-------------------------------------------------------------------------
	// For data frame function add a new frame to the list of frames to be
	// created in bss section. Then load frame address to a4 register.
	//-------------------------------------------------------------------------

	if (called->toFrame())
	{
		Printf("called %s() with frame[%ld]\n", called->name(), called->getFrameSize());

		if (const char *frameLabel = Comp->addDataFrame(called->getFrameSize()))
		{
			Operand op1(IIOP_LABEL, (int)frameLabel);
			Operand op2(IIOP_ADDRREG, 4);

			if (InterInstruction *ii = new InterInstruction(II_LDEA, op1, op2))
			{
				ii->insertBefore(call);
			}
			else return FALSE;
		}
		else return FALSE;
	}

	//-------------------------------------------------------------------------
	// Functions returning a data frame put their results in the data frame.
	// The only result being put on stack is the data frame address, so number
	// of arguments to expand here is always 1.
	//-------------------------------------------------------------------------

	int realResults = called->numResults;
	if (called->toFrame()) realResults = 1;

	for (regnum = realResults; regnum > 0; regnum--)
	{
		Operand opr(IIOP_CRESULT, regnum - 1);
		ii = new InterInstruction(II_DROP, opr);
		if (ii) ii->insertAfter(call);
		else return FALSE;
	}

	return TRUE;
}

//---------------------------------------------------------------------------------------------

bool Function::expandSysCall(InterInstruction *call)
{
	if (SysCall *sc = Comp->findSysCall((const char*)call->out.value))
	{
		InterInstruction *insertPoint;
		const char *p;
		char regType, regNumber;
		int opType;
		
		p = sc->arguments;
		insertPoint = call;
		
		while((regType = *p++) && (regNumber = *p++))
		{
			opType = IIOP_DATAREG;
			if (regType == 'a') opType = IIOP_ADDRREG;
			regNumber -= '0';
			Operand op(opType, regNumber);

			if (InterInstruction *ii = new InterInstruction(II_PULL, op))
			{
				ii->insertBefore(insertPoint);
				insertPoint = ii;
			}
			else return FALSE;
		}

		p = sc->results;
		insertPoint = call;
		
		while((regType = *p++) && (regNumber = *p++))
		{
			opType = IIOP_DATAREG;
			if (regType == 'a') opType = IIOP_ADDRREG;
			regNumber -= '0';
			Operand op(opType, regNumber);

			if (InterInstruction *ii = new InterInstruction(II_DROP, op))
			{
				ii->insertAfter(insertPoint);
				insertPoint = ii;
			}
			else return FALSE;
		}
	}

	return TRUE;
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
