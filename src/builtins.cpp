/* definitions of built-in operators (functions) */

#include "builtins.h"
#include "function.h"
#include "syscalls.h"

/*-------------------------------------------------------------------------------------------*/

static bool OpGenDyadic(Function *function, int op)
{
	Operand opr1(IIOP_VIRTUAL, 0);
	Operand opr2(IIOP_VIRTUAL, 1); 
	InterInstruction *ii0 = new InterInstruction(II_PULL, opr1);
	InterInstruction *ii1 = new InterInstruction(II_PULL, opr2);
	InterInstruction *ii2 = new InterInstruction(op, opr1, opr2);
	InterInstruction *ii3 = new InterInstruction(II_DROP, opr2);

	if (ii0 && ii1 && ii2 && ii3)
	{
		function->addCode(ii0);
		function->addCode(ii1);
		function->addCode(ii2);
		function->addCode(ii3);
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------*/

static bool OpGenMonadic(Function *function, int op)
{
	Operand opr1(IIOP_VIRTUAL, 0);
	InterInstruction *ii0 = new InterInstruction(II_PULL, opr1);
	InterInstruction *ii1 = new InterInstruction(op, opr1);
	InterInstruction *ii2 = new InterInstruction(II_DROP, opr1);

	if (ii0 && ii1 && ii2)
	{
		function->addCode(ii0);
		function->addCode(ii1);
		function->addCode(ii2);
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------*/

static bool OpGenPlus(Function *function)
{
	return OpGenDyadic(function, II_ADDL);
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenMinus(Function *function)
{
	return OpGenDyadic(function, II_SUBL);
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenAnd(Function *function)
{
	return OpGenDyadic(function, II_ANDL);
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenOr(Function *function)
{
	return OpGenDyadic(function, II_ORRL);
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenEor(Function *function)
{
	return OpGenDyadic(function, II_EORL);
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenNot(Function *function)
{
	return OpGenMonadic(function, II_NOTL);
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenFuncEnd(Function *function)
{
	InterInstruction *ii0 = new InterInstruction(II_RETN);

	if (ii0)
	{
		function->addCode(ii0);
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenDup(Function *function)
{
	Operand opr(IIOP_VIRTUAL, 0);
	InterInstruction *ii0 = new InterInstruction(II_PULL, opr);
	InterInstruction *ii1 = new InterInstruction(II_PUSH, opr);
	InterInstruction *ii2 = new InterInstruction(II_DROP, opr);

	if (ii0 && ii1 && ii2)
	{
		function->addCode(ii0);
		function->addCode(ii1);
		function->addCode(ii2);
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenOver(Function *function)
{
	Operand opr1(IIOP_VIRTUAL, 0);
	Operand opr2(IIOP_VIRTUAL, 1);
	InterInstruction *ii0 = new InterInstruction(II_PULL, opr1);
	InterInstruction *ii1 = new InterInstruction(II_PULL, opr2);
	InterInstruction *ii2 = new InterInstruction(II_PUSH, opr2);
	InterInstruction *ii3 = new InterInstruction(II_DROP, opr1);
	InterInstruction *ii4 = new InterInstruction(II_DROP, opr2);

	if (ii0 && ii1 && ii2 && ii3 && ii4)
	{
		function->addCode(ii0);
		function->addCode(ii1);
		function->addCode(ii2);
		function->addCode(ii3);
		function->addCode(ii4);
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenFlip(Function *function)
{
	Operand opr1(IIOP_VIRTUAL, 0);
	Operand opr2(IIOP_VIRTUAL, 1);
	InterInstruction *ii0 = new InterInstruction(II_PULL, opr1);
	InterInstruction *ii1 = new InterInstruction(II_PULL, opr2);
	InterInstruction *ii2 = new InterInstruction(II_DROP, opr1);
	InterInstruction *ii3 = new InterInstruction(II_DROP, opr2);

	if (ii0 && ii1 && ii2 && ii3)
	{
		function->addCode(ii0);
		function->addCode(ii1);
		function->addCode(ii2);
		function->addCode(ii3);
		return TRUE;
	}

	return FALSE;
}

//---------------------------------------------------------------------------------------------

bool OpGenPop(Function *function)
{
	Operand opr1(IIOP_VIRTUAL, 0);
	InterInstruction *ii0 = new InterInstruction(II_PULL, opr1);

	if (ii0)
	{
		function->addCode(ii0);
		return TRUE;
	}

	return FALSE;
}

//---------------------------------------------------------------------------------------------

bool OpGenMul(Function *function)
{
	return GenerateSysCall("SMult32", function);
}

//---------------------------------------------------------------------------------------------

bool OpGenDiv(Function *function)
{
	if (GenerateSysCall("SDivMod32", function))
	{
		return OpGenPop(function);   // remove unused modulus result
	}

	return FALSE;
}

//---------------------------------------------------------------------------------------------

bool OpGenMod(Function *function)
{
	if (GenerateSysCall("SDivMod32", function))
	{
		Operand opr1(IIOP_VIRTUAL, 0);  // discard division result
		Operand opr2(IIOP_VIRTUAL, 1);
		InterInstruction *ii0 = new InterInstruction(II_PULL, opr1);
		InterInstruction *ii1 = new InterInstruction(II_PULL, opr2);
		InterInstruction *ii2 = new InterInstruction(II_DROP, opr1);

		if (ii0 && ii1 && ii2)
		{
			function->addCode(ii0);
			function->addCode(ii1);
			function->addCode(ii2);
			return TRUE;
		}		
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------*/

bool OpGenFuncStart(Function *function)
{
	function = function;
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/
/* Array must be sorted by operator name. */

struct KeyedPair<Operator> BuiltIns[17] = {
	{ "%", { 2, 1, OpGenMod } },
	{ "&", { 2, 1, OpGenAnd } },
	{ "+", { 2, 1, OpGenPlus } },
	{ ",", { 0, 1, OpGenOver } },
	{ "-", { 2, 1, OpGenMinus } },
	{ ".", { 1, 1, OpGenDup } },
	{ ":", { 1, 1, OpGenFlip } },
	{ "[", { 0, 0, OpGenFuncStart } },
	{ "\\", { 1, 0, OpGenPop } },
	{ "]", { 0, 0, OpGenFuncEnd } },
	{ "^", { 2, 1, OpGenEor } },
	{ "{", { 0, 0, OpGenFuncStart } },
	{ "|", { 2, 1, OpGenOr } },
	{ "}", { 0, 0, OpGenFuncEnd } },
	{ "~", { 1, 1, OpGenNot } },
	{ "×", { 2, 1, OpGenMul } },
	{ "÷", { 2, 1, OpGenDiv } }
};