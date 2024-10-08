//------------------------------------------------------------
// register scheduler
//------------------------------------------------------------

#include "scheduler.h"

bool Scheduler::start(int numregs)
{
	numRegisters = numregs;
	
	if (registers = new int[numRegisters])
	{
		for (int i = 0; i < numRegisters; i++) registers[i] = 0;
		return TRUE;
	}
	
	return FALSE;
}


int Scheduler::useFirstFor(int ticks)
{
	for (int i = 0; i < numRegisters; i++)
	{
		if (registers[i] == 0)
		{
			registers[i] = ticks;
			return i;
		}
	}
	
	return -1;
}


void Scheduler::tick()
{
	for (int i = 0; i < numRegisters; i++) if (registers[i] > 0) registers[i]--;
}


Scheduler::~Scheduler()
{
	if (registers) delete registers;
}