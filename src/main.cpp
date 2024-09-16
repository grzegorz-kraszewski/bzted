#include <proto/exec.h>
#include <proto/dos.h>

#include <workbench/startup.h>

#include "compiler.h"
#include "logger.h"

extern Library *SysBase, *DOSBase;

Logger log;
Compiler *Comp;

/*---------------------------------------------------------------------------*/

LONG Main(WBStartup *wbmsg)
{
	Compiler compiler;
	Comp = &compiler;
	wbmsg = wbmsg;

	log.setLevel(LOGLEVEL_DEBUG);
	compiler.scan("test.bzt");
	compiler.lex();
	compiler.dumpTokens();
	compiler.translate();
	compiler.transform();
	compiler.dumpFunctions();
	compiler.optimize();
	compiler.dumpFunctions();
	return 0;
}