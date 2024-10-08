//------------------------------------------------------------
// register scheduler
//------------------------------------------------------------

#include <exec/types.h>

class Scheduler
{
	int *registers;
	int numRegisters;
	
	public:
	
	Scheduler() { registers = NULL; numRegisters = 0; }
	bool start(int numregs);
	int useFirstFor(int ticks);
	void tick();
	~Scheduler();
};
