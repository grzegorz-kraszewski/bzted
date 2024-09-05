/* definitions of built-in operators (functions) */

#include "builtins.h"
#include "function.h"

const struct FuncPin OneInt[1] = { { BTYPE_INT, BREG_Dn } };
const struct FuncPin TwoInt[2] = { { BTYPE_INT, BREG_Dn }, { BTYPE_INT, BREG_Dn } };


/*-------------------------------------------------------------------------------------------*/

static BOOL OpGenDyadic(Function *function, int op)
{
	Operand opr1 = { IIOP_REGISTER, II_D + 1 };
	Operand opr2 = { IIOP_REGISTER, II_D + 0 }; 
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

static BOOL OpGenMonadic(Function *function, int op)
{
	Operand opr1 = { IIOP_REGISTER, II_D + 0 };
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

static BOOL OpGenPlus(Function *function)
{
	return OpGenDyadic(function, II_ADDL);
}

/*-------------------------------------------------------------------------------------------*/

BOOL OpGenMinus(Function *function)
{
	return OpGenDyadic(function, II_SUBL);
}

/*-------------------------------------------------------------------------------------------*/

BOOL OpGenAnd(Function *function)
{
	return OpGenDyadic(function, II_ANDL);
}

/*-------------------------------------------------------------------------------------------*/

BOOL OpGenOr(Function *function)
{
	return OpGenDyadic(function, II_ORRL);
}

/*-------------------------------------------------------------------------------------------*/

BOOL OpGenEor(Function *function)
{
	return OpGenDyadic(function, II_EORL);
}

/*-------------------------------------------------------------------------------------------*/

BOOL OpGenNot(Function *function)
{
	return OpGenMonadic(function, II_NOTL);
}

/*-------------------------------------------------------------------------------------------*/

BOOL OpGenFuncEnd(Function *function)
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

BOOL OpGenDup(Function *function)
{
	Operand opr = { IIOP_REGISTER, II_D + 0 };
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

BOOL OpGenOver(Function *function)
{
	Operand opr1 = { IIOP_REGISTER, II_D + 0 };
	Operand opr2 = { IIOP_REGISTER, II_D + 1 };
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

BOOL OpGenFlip(Function *function)
{
	Operand opr1 = { IIOP_REGISTER, II_D + 0 };
	Operand opr2 = { IIOP_REGISTER, II_D + 1 };
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


/*-------------------------------------------------------------------------------------------*/

BOOL OpGenFuncStart(Function *function)
{
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/
/* Array must be sorted by operator name. */

struct KeyedPair<Operator> BuiltIns[11] = {
	{ "&", { 2, 1, 0, TwoInt, OneInt, OpGenAnd } },
	{ "+", { 2, 1, 0, TwoInt, OneInt, OpGenPlus } },
	{ ",", { 0, 1, 0, NULL, NULL, OpGenOver } },
	{ "-", { 2, 1, 0, TwoInt, OneInt, OpGenMinus } },
	{ ".", { 1, 1, 0, OneInt, OneInt, OpGenDup } },
	{ ":", { 1, 1, 0, OneInt, OneInt, OpGenFlip } },
	{ "^", { 2, 1, 0, TwoInt, OneInt, OpGenEor } },
	{ "{", { 0, 0, 0, NULL, NULL, OpGenFuncStart } },
	{ "|", { 2, 1, 0, TwoInt, OneInt, OpGenOr } },
	{ "}", { 0, 0, 0, NULL, NULL, OpGenFuncEnd } },
	{ "~", { 1, 1, 0, OneInt, OneInt, OpGenNot } }
};