/* User defined function. */

#ifndef BZTED_FUNCTION_H
#define BZTED_FUNCTION_H

#include "rplist.h"
#include "inter.h"


class Optimizer;


struct PushPullBlock
{
	InterInstruction *push;
	InterInstruction *pull;
};


class Function : public RpNode<Function>
{
	friend class Optimizer;

	RpList<InterInstruction> code;
	int numArguments;
	int numResults;
	int maxStackDepth;
	int firstFreeRegister;
	bool resultsToFrame;
	int frameSize;
	
	bool expandCall(InterInstruction *ii);
	void expandSysCall(InterInstruction *call);
	InterInstruction* findPushPullBlock(PushPullBlock &ppblock);
	void replacePushPullBlock(PushPullBlock &ppblock);
	
	public:

	const char *name;

	Function(const char *fname)
	{
		name = fname; 
		resultsToFrame = 0;
	}
	
	void setResultMode(bool mode) { resultsToFrame = mode; }
	bool toFrame() { return resultsToFrame; }
	int getFrameSize() { return frameSize; }
	void addCode(InterInstruction *ii) { code.addTail(ii); }
	void stackSignature();
	bool expand();
	void expandAllCalls();
	int replaceAllPushPullBlocks();
	void print();
};

#endif  /* BZTED_FUNCTION_H */
