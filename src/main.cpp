#include <proto/exec.h>
#include <proto/dos.h>

#include <workbench/startup.h>

#include "compiler.h"
#include "logger.h"

extern Library *SysBase, *DOSBase;

extern int MemCounter;

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

int Main(WBStartup *wbmsg)
{
	Compiler compiler;
	Comp = &compiler;
	wbmsg = wbmsg;
	RDArgs *args;
	LONG argArray[2] = { 0, 0 };
	int result = RETURN_ERROR;
	
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
			log.setSource(inputName);

			if (compiler.scan(inputName))
			{
				log.verbose("memory usage: %ld bytes", MemCounter);

				if (compiler.lex())
				{
					log.verbose("memory usage: %ld bytes", MemCounter);
					compiler.dumpTokens();

					if (compiler.translate())
					{
						log.verbose("memory usage: %ld bytes", MemCounter);
						compiler.dumpFunctions();
						result = RETURN_OK;
					}
				}
			}
		}


/*
			if (!compiler.transform()) return RETURN_ERROR;
			log.verbose("memory usage: %ld bytes", MemCounter);
			compiler.dumpFunctions();
			if (!compiler.optimize()) return RETURN_ERROR;
			log.verbose("memory usage: %ld bytes", MemCounter);
			compiler.dumpFunctions();
			if (!compiler.generate(inputName, outputName)) return RETURN_ERROR;
			log.verbose("memory usage: %ld bytes", MemCounter);
*/

		FreeArgs(args);
	}
	else PrintFault(IoErr(), "bzted: arguments");

	return result;
}
