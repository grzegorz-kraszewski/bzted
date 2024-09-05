//-----------------------------
// intermediate code optimizer
//-----------------------------

// Curently all optimizer functions are methods of Function class, but have been placed in
// a separate file.

#include "main.h"
#include "function.h"

#define REGU_NONE 0
#define REGU_SRC  1
#define REGU_DEST 2
#define REGU_BOTH (REGU_SRC | REGU_DEST)




void Function::updateRegisterUsage(RegisterUsage *regarray, int count, InterInstruction *instr)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if ((instr->arg.type == IIOP_REGISTER) && (instr->arg.value == regarray[i].reg))
		{
			regarray[i].usage |= REGU_SRC;
		}

		if ((instr->out.type == IIOP_REGISTER) && (instr->out.value == regarray[i].reg))
		{
			if ((instr->code == II_MOVE) || (instr->code == II_COPY)) regarray[i].usage |= REGU_DEST;
			else regarray[i].usage |= REGU_BOTH;
		}
	}
}

//---------------------------------------------------------------------------------------------
// For registers specified in RegisterUsage array, scans code block between 'start' to 'end',
// both excluded. Determines usage of specified registers in the block. The function assumes
// that both 'start' and 'end' are on the code list, 'start' before 'end'.

void Function::registerUsageOverBlock(RegisterUsage *regarray, int count, InterInstruction
	*start,	InterInstruction *end)
{
	for(start = code.next(start); start != end; start = code.next(start))
	{
		updateRegisterUsage(regarray, count, start);
	}	
}


InterInstruction* Function::findPushPullBlock(PushPullBlock &ppblock, InterInstruction *ii)
{
	ppblock.push = NULL;
	ppblock.pull = NULL;

	while (ii)
	{
		if (ii->code == II_PULL)
		{
			if (ppblock.push)
			{
				ppblock.pull = ii;
				return code.next(ii);
			}
		}
		else if ((ii->code == II_DROP) || (ii->code == II_PUSH))
		{
			ppblock.push = ii;
		}

		ii = code.next(ii);
	}

	return NULL;
}

//---------------------------------------------------------------------------------------------

int Function::optimizePushPullBlock(PushPullBlock &ppblock)
{
	RegisterUsage regusage[2];

	regusage[0].reg = ppblock.push->out.value;   /* PUSH|PULL|DROP always have register operand */
	regusage[0].usage = REGU_NONE;
	regusage[1].reg = ppblock.pull->out.value;
	regusage[1].usage = REGU_NONE;

	registerUsageOverBlock(regusage, 2, ppblock.push, ppblock.pull);

	Printf("optimizer: register usage [d%ld: %ld, d%ld: %ld].\n", 
		regusage[0].reg - 1, regusage[0].usage, regusage[1].reg - 1, regusage[1].usage);

	if ((regusage[0].usage == REGU_NONE) && (regusage[1].usage == REGU_NONE))
	{
		int newop = (ppblock.push->code == II_DROP) ? II_MOVE : II_COPY;
		ppblock.push->code = newop;
		ppblock.push->arg.type = IIOP_REGISTER;
		ppblock.push->arg.value = regusage[0].reg;
		ppblock.push->out.type = IIOP_REGISTER;
		ppblock.push->out.value = regusage[1].reg;				
		code.remove(ppblock.pull);
		PutStr("eliminated.\n");
		return 1;
	}			

	return 0;
}

//---------------------------------------------------------------------------------------------

int Function::optimizeAllPushPullBlocks()
{
	int optimizedBlocks = 0;
	InterInstruction *ii = code.first();
	PushPullBlock ppblock;

	while (ii = findPushPullBlock(ppblock, ii))
	{
		PutStr("optimizer: PUSH|DROP - PULL block:\n");
		ppblock.push->print();
		ppblock.pull->print(); 
		optimizedBlocks += optimizePushPullBlock(ppblock);
	}

	return optimizedBlocks;
}

//---------------------------------------------------------------------------------------------

void Function::optimizeMovesToSelf()
{
	InterInstruction *ii;

	for (ii = code.first(); ii; ii = code.next(ii))
	{
		if ((ii->code == II_MOVE) || (ii->code == II_COPY))
		{
			if ((ii->arg.type == IIOP_REGISTER) && (ii->out.type == IIOP_REGISTER))
			{
				if (ii->arg.value == ii->out.value)
				{
					PutStr("optimizer: MOVE|COPY to self:\n");
					ii->print();
					PutStr("eliminated.\n");
					code.remove(ii);
				}
			}
		}
	}
}

//---------------------------------------------------------------------------------------------

InterInstruction* Function::findMoveCascade()
{
	InterInstruction *ii, *iia;

	for (ii = code.first(); ii; ii = code.next(ii))
	{
		if (ii->code == II_MOVE)
		{
			if (iia = code.next(ii))
			{
				if (iia->code == II_MOVE)
				{
					if ((ii->out.type == IIOP_REGISTER) && (iia->arg.type = IIOP_REGISTER))
					{
						if (ii->out.value == iia->arg.value)
						{
							PutStr("optimizer: move cascade:\n");
							ii->print();
							iia->print();
							return ii;
						}
					}
				}
			}
		}
	}

	return NULL;	
}

//---------------------------------------------------------------------------------------------

void Function::optimizeMoveCascades()
{
	InterInstruction *ii, *iia;

	while (ii = findMoveCascade())
	{
		iia = code.next(ii);               /* findMoveCascade() made sure it exists */
		ii->out.value = iia->out.value;
		code.remove(iia);
		PutStr("eliminated.\n");
	}
}

//---------------------------------------------------------------------------------------------

InterInstruction* Function::findMoveToDyadic()
{
	InterInstruction *ii, *iia;

	for (ii = code.first(); ii; ii = code.next(ii))
	{
		if ((ii->code == II_MOVE) || (ii->code == II_COPY))
		{
			if (iia = code.next(ii))
			{
				if (iia->isDyadic())
				{
					if ((ii->out.type == IIOP_REGISTER) && (iia->arg.type = IIOP_REGISTER))
					{
						if (ii->out.value == iia->arg.value)
						PutStr("optimizer: move/copy to dyadic\n");
						ii->print();
						iia->print();
						return ii;
					}
				}
			}
		}
	}

	return NULL;
}

//---------------------------------------------------------------------------------------------

void Function::optimizeMovesToDyadic()
{
	InterInstruction *ii, *iia;

	while (ii = findMoveToDyadic())
	{
		RegisterUsage regusage[1] = { ii->out.value, REGU_NONE };

		iia = code.next(ii);
		registerUsageOverBlock(regusage, 1, iia, code.last());

		if (!(regusage[0].usage & REGU_SRC))
		{
			iia->arg = ii->arg;
			code.remove(ii);
			PutStr("eliminated.\n");
		}
	}
}


BOOL Function::optimize()
{
	while (optimizeAllPushPullBlocks())
	{
		optimizeMovesToSelf();
		optimizeMoveCascades();
		optimizeMovesToDyadic();
	}

	return FALSE;
}