/* intermediate code */

#include "main.h"
#include "inter.h"

#include <proto/dos.h>


#define II_MOVE   1
#define II_COPY   2
#define II_PUSH   3
#define II_PULL   4
#define II_DROP   5
#define II_ADDL   6
#define II_SUBL   7
#define II_JMPS   8
#define II_RETN   9
#define II_NOTL  10
#define II_ANDL  11
#define II_ORRL  12
#define II_EORL  13
#define II_EXCH  14


const char *CodeNames[] = { NULL,
	"MOVE", "COPY", "PUSH", "PULL", "DROP", "ADDL", "SUBL", "JSBR", "RETN", "NOTL",
	"ANDL", "ORRL", "EORL", "EXCH" };


void InterInstruction::print()  /* a bit slow method */
{
	Printf("%08lx\t%s", this, CodeNames[code]);

	if (arg.type != IIOP_NONE)
	{
		if (arg.type == IIOP_IMMEDIATE) Printf(" #%ld,", arg.value);
		else
		{
			if (arg.value < II_A) Printf(" d%ld,", arg.value - 1);
			else Printf(" a%ld,", arg.value - II_A);
		}
	}

	if (out.type != IIOP_NONE)
	{
		if (arg.type == IIOP_NONE) PutStr(" ");
		if (out.value < II_A) Printf("d%ld", out.value - 1);
		else Printf("a%ld", out.value - II_A);
	}

	if (label) Printf(" %s", label);
	PutStr("\n");
} 

//---------------------------------------------------------------------------------------------

BOOL InterInstruction::isDyadic()
{
	if (arg.type == IIOP_NONE) return FALSE;
	if (code == II_COPY) return FALSE;
	if (code == II_MOVE) return FALSE;
	if (code == II_EXCH) return FALSE;
	return TRUE;
}