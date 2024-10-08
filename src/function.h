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


class Function : public RpNamedNode<Function>
{
	friend class Optimizer;

	RpList<InterInstruction> code;
	int numArguments;
	int numResults;
	const char *argumentTypes;
	const char *resultTypes;
	int maxStackDepth;
	int firstFreeRegister;
	bool resultsToFrame;
	int frameSize;
	int lineNum;
	
	bool expandCall(InterInstruction *ii);
	bool expandSysCall(InterInstruction *call);
	InterInstruction* findPushPullBlock(PushPullBlock &ppblock);
	void replacePushPullBlock(PushPullBlock &ppblock);
	
	public:

	Function(const char *name, int line) : RpNamedNode<Function>(name)
	{
		resultsToFrame = 0;
		numArguments = 0;
		numResults = 0;
		lineNum = line;
	}
	
	bool parseSignature();
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
