/* User defined function. */

#ifndef BZTED_FUNCTION_H
#define BZTED_FUNCTION_H

#include "syslist.h"
#include "inter.h"

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

	private:

	SysList<InterInstruction> code;
	int numArguments;
	int numResults;

//	LONG flags;
//	const FuncPin *inputs;
//	const FuncPin *outputs;
};

#endif  /* BZTED_FUNCTION_H */
