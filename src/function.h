/* User defined function. */

#ifndef BZTED_FUNCTION_H
#define BZTED_FUNCTION_H

#include "syslist.h"
#include "inter.h"


struct PushPullBlock
{
	InterInstruction *push;
	InterInstruction *pull;
};

struct RegisterUsage
{
	int reg;
	int usage;
};


class Function
{
	public:

	Function *succ;
	Function *pred;
	const char *name;

	Function(const char *fname) { name = fname; }
	void addCode(InterInstruction *ii) { code.addtail(ii); }
	void stackSignature();
	void expand();
	void expandCall(InterInstruction *ii);
	void expandAllCalls();
	void print();

	/* optimizer */

	void updateRegisterUsage(RegisterUsage *regarray, int count, InterInstruction *instr);
    void registerUsageOverBlock(RegisterUsage *regarray, int count, InterInstruction *start,
		InterInstruction *end);
	InterInstruction* findPushPullBlock(PushPullBlock &ppblock, InterInstruction *ii);
	int optimizePushPullBlock(PushPullBlock &ppblock);
	int optimizeAllPushPullBlocks();
	void optimizeMovesToSelf();
	InterInstruction* findMoveCascade();
	void optimizeMoveCascades();
	InterInstruction* findMoveToDyadic();
	void optimizeMovesToDyadic();


	BOOL optimize();

	private:

	SysList<InterInstruction> code;
	int numArguments;
	int numResults;

//	LONG flags;
//	const FuncPin *inputs;
//	const FuncPin *outputs;
};

#endif  /* BZTED_FUNCTION_H */
