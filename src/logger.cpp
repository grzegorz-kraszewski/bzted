// Logging facility
//------------------

#include "main.h"
#include "logger.h"
#include "strutils.h"

#include <proto/dos.h>

const char* LevelNames[5] = { "debug", "verbose", "info", "warning", "error" };

//---------------------------------------------------------------------------------------------

void Logger::log(int lvl, const char *msg, int *args)
{
	if (lvl < level) return;

	const char *colorPrefix = "";
	const char *colorSuffix = "";

	if (lvl == LOGLEVEL_ERROR)
	{
		colorPrefix = "\x1B[1m\x1B[32m";
		colorSuffix = "\x1B[0m";
	}

	if (lvl == LOGLEVEL_WARNING)
	{
		colorPrefix = "\x1B[1m";
		colorSuffix = "\x1B[0m";
	}


	if (char *msg1 = VFmtNew(msg, args))
	{
		Printf("%s%s%s [%s]: %s.\n", colorPrefix, LevelNames[lvl - 1], colorSuffix, module, msg1);
		delete msg1;
	}
}

//---------------------------------------------------------------------------------------------

void Logger::outOfMemory()
{
	PutStr("\x1B[1m\x1B[32merror\x1B[0m: out of memory.\n");
}

//---------------------------------------------------------------------------------------------

char* Logger::fault(int syserror, const char *prefix)
{
	Fault(syserror, prefix, faultBuffer, 96);
	return faultBuffer;
}
