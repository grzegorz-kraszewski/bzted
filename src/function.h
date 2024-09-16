/* User defined function. */

#ifndef BZTED_FUNCTION_H
#define BZTED_FUNCTION_H

#include "rplist.h"
#include "inter.h"


struct PushPullBlock
{
	InterInstruction *push;
	InterInstruction *pull;
};


class Function : public RpNode<Function>
{
	RpList<InterInstruction> code;
	int numArguments;
	int numResults;
	int maxStackDepth;
	int firstFreeRegister;

	void expandCall(InterInstruction *ii);
	InterInstruction* findPushPullBlock(PushPullBlock &ppblock);
	void replacePushPullBlock(PushPullBlock &ppblock);
	public:

	const char *name;

	Function(const char *fname) { name = fname; }
	void addCode(InterInstruction *ii) { code.addTail(ii); }
	void stackSignature();
	void expand();
	void expandAllCalls();
	int replaceAllPushPullBlocks();
	void print();
};

#endif  /* BZTED_FUNCTION_H */
