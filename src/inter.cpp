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
	Printf("\t\t%s", CodeNames[code]);

	if (arg)
	{
		if (flags & II_IMMEDIATE) Printf(" #%ld,", arg);
		else
		{
			if (arg < II_A) Printf(" d%ld,", arg - 1);
			else Printf(" a%ld,", arg - II_A);
		}
	}

	if (out)
	{
		if (!arg) PutStr(" ");
		if (out < II_A) Printf("d%ld", out - 1);
		else Printf("a%ld", out - II_A);
	}

	if (label) Printf(" %s", label);
	PutStr("\n");
} 
