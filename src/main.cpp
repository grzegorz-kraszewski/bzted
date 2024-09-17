#include <proto/exec.h>
#include <proto/dos.h>

#include <workbench/startup.h>

#include "compiler.h"
#include "logger.h"

extern Library *SysBase, *DOSBase;

Logger log;
Compiler *Comp;


//---------------------------------------------------------------------------------------------
// Removes ".bzt" extension from the input name (if exists) and appends ".asm" extenstion

static char* createOutputName(const char *input)
{
	char *output;
	int inputLength = StrLen(input);

	if (StrCmp(input + inputLength - 4, ".bzt") == 0)
	{
		if (output = StrClone(input))
		{
			StrCopy(".asm", output + inputLength - 4);
		}
	}
	else
	{
		if (output = new char[inputLength + 5])
		{
			StrCopy(".asm", StrCopy(input, output));
		}
	}
	
	return output;
}


//-----------------------------------------------------------------------------

LONG Main(WBStartup *wbmsg)
{
	Compiler compiler;
	Comp = &compiler;
	wbmsg = wbmsg;
	RDArgs *args;
	LONG argArray[2] = { 0, 0 };

	wbmsg = wbmsg;
	log.setLevel(LOGLEVEL_DEBUG);

	if (args = ReadArgs("FROM/A,TO", argArray, NULL))
	{
		char *inputName, *outputName;
	
		inputName = (char*)argArray[0];
		outputName = (char*)argArray[1];

		if (!outputName) outputName = createOutputName(inputName);

		if (outputName)
		{
			compiler.scan(inputName);
			compiler.lex();
			compiler.dumpTokens();
			compiler.translate();
			compiler.transform();
			compiler.dumpFunctions();
			compiler.optimize();
			compiler.dumpFunctions();
		}

		FreeArgs(args);
	}
	else PrintFault(IoErr(), "bzted: arguments");

	return 0;
}
