/* intermediate code */

#include "main.h"
#include "inter.h"
#include "strutils.h"

#include <proto/dos.h>


const char *CodeNames[II_INSTRUCTION_COUNT + 1] = { NULL,
	"MOVE", "COPY", "DMOV", "DCPY", "PUSH", "PULL", "DROP", "ADDL", "SUBL", "JSBR",
	"RETN", "NOTL",	"ANDL", "ORRL", "EORL", "EXCH" };


char* Operand::makeString(char *buf)
{
	switch (type)
	{
		case IIOP_VIRTUAL:      FmtPut(buf, "v%ld", value); break;
		case IIOP_EDGE:         FmtPut(buf, "e%ld", value); break;
		case IIOP_DATAREG:      FmtPut(buf, "d%ld", value); break;
		case IIOP_ADDRREG:      FmtPut(buf, "a%ld", value); break;
		case IIOP_IMMEDIATE:    FmtPut(buf, "#%ld", value); break;
	}

	return buf;
}


void InterInstruction::print()  /* a bit slow method */
{
	char buf[16];

	Printf("%08lx\t%s", this, CodeNames[code]);

	if (arg.type != IIOP_NONE) Printf(" %s,", arg.makeString(buf));
	if (out.type != IIOP_NONE)
	{
		if (arg.type == IIOP_NONE) PutStr(" ");
		PutStr(out.makeString(buf));
	}

	if (label) Printf(" %s", label);
	PutStr("\n");
} 

//---------------------------------------------------------------------------------------------
