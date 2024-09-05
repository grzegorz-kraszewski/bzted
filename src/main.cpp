#include <proto/exec.h>
#include <proto/dos.h>

#include <workbench/startup.h>

#include "main.h"
#include "compiler.h"

extern Library *SysBase, *DOSBase;

Compiler *Comp;

/*---------------------------------------------------------------------------*/

LONG Main(WBStartup *wbmsg)
{
	Compiler compiler;
	Comp = &compiler;

	compiler.scan("test.bzt");
	compiler.lex();
	compiler.dumpTokens();
	PutStr("---------------------------\n");
	compiler.compileCode();
	PutStr("Function expanding...\n");
	compiler.expandArgsResults();
	compiler.dumpFunctions();
	return 0;
}