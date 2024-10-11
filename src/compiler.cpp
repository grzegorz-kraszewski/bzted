#include <proto/exec.h>
#include <proto/dos.h>

#include "compiler.h"
#include "logger.h"
#include "scanner.h"
#include "lexer.h"
#include "optimizer.h"


//---------------------------------------------------------------------------------------------

void UsedSysCall::generate(BPTR file)
{
	FPrintf(file, "%-23s = %ld\t; %s\n", name(), offset, libName);
}

//---------------------------------------------------------------------------------------------

bool Compiler::scan(const char *fileName)
{
	Scanner scanner(tokens);

	log.setModule("scanner");
	return scanner.scan(fileName);
}

//---------------------------------------------------------------------------------------------

bool Compiler::lex()
{
	Lexer lexer(tokens);

	log.setModule("lexer");
	return lexer.lex();
}

/*-------------------------------------------------------------------------------------------*/

bool Compiler::translate()
{
	Token *token = tokens.first();

	log.setModule("translator");
	transResult = TRUE;
	if (!token) { log.error ("no tokens in code"); return FALSE; }
	while (token = translateDefinition(token));
	return transResult;
}

/*-------------------------------------------------------------------------------------------*/

Token* Compiler::translateDefinition(Token *token)
{
	Function *function;

	if (token->type != TT_DEF)
	{
		log.error("%ld: '%s', code outside of function", token->lineNum, token->text);
		transResult = FALSE;
		return NULL;
	}

	if (function = functions.find(token->text))
	{
		return translateCodeBlock(token->next(), function);
	}
	else
	{
		log.error("%ld: '%s', unknown type of definition", token->lineNum, token->text);
		transResult = FALSE;
		return NULL;
	}
}

//---------------------------------------------------------------------------------------------
// Token passed is the opening bracket of the block.

Token* Compiler::translateCodeBlock(Token *token, Function *function)
{
	const char *rightClosing = "";
	const char *wrongClosing = "";

	if (StrCmp(token->text, "{") == 0)
	{
		function->setResultMode(RESULTS_TO_STACK);
		rightClosing = "}";
		wrongClosing = "]";
	}
		
	if (StrCmp(token->text, "[") == 0)
	{
		function->setResultMode(RESULTS_TO_FRAME);
		rightClosing = "]";
		wrongClosing = "}";
	}

	token = token->next();   // the first token of function body

	while (token)
	{
		switch (token->type)
		{
			case TT_INT:
			case TT_STR:
			case TT_FNC:
			case TT_SYS:
				if (!(transResult = token->translate(function))) return NULL;
			break;

			case TT_OPR:
			{
				if ((StrCmp(token->text, "{") == 0) || (StrCmp(token->text, "[") == 0)) 
				{
					if (char *label = new char[8])
					{
						getUniqueLabel(label);

						if (Function *f2 = addFunction(label, token->lineNum))
						{
							token = translateCodeBlock(token, f2);
							
							// adding JSBR to anonymous code block
							// the code may be inlined later
							
							Operand op(IIOP_LABEL, (int)label);
							InterInstruction *ii = new InterInstruction(II_JSBR, op);
							if (ii) function->addCode(ii);
							else { transResult = FALSE; return NULL; }
							
							if (token) continue;
							else return NULL;
						}
					}
				}
				else
				{
					if (!(transResult = token->translate(function))) return NULL;
					

					if (StrCmp(token->text, rightClosing) == 0)
					{
						if (function->stackCalculations())
						{
							return token->next();
						}
						else
						{
							transResult = FALSE;
							return NULL;
						}
					}

					if (StrCmp(token->text, wrongClosing) == 0)
					{
						log.error("%ld: '%s', incorrect nesting of code blocks", token->lineNum,
							token->text);
						transResult = FALSE;
						return NULL;
					}
				}
			}
			break;

			case TT_DEF:
				log.error("%ld: '%s', nested definitions are not allowed", token->lineNum, token->text);
				transResult = FALSE;
				return NULL;
			break;
		}

		token = token->next();
	}	

	log.error("%ld: unexpected end of file inside function", tokens.last()->lineNum);
	return NULL;
}

//---------------------------------------------------------------------------------------------

bool Compiler::transform()
{
	log.setModule("transformer");
	
	for (Function *f = functions.first(); f; f = f->next()) if (!(f->expand())) return FALSE;
	for (Function *f = functions.first(); f; f = f->next())	f->expandAllCalls();
	for (Function *f = functions.first(); f; f = f->next()) f->replaceAllPushPullBlocks();
	
	return TRUE;
}

//---------------------------------------------------------------------------------------------

bool Compiler::optimize()
{
	bool success = TRUE;

	log.setModule("optimizer");
		
	for  (Function *f = functions.first(); f && success; f = f->next())
	{
		Optimizer opt(f);
		success = opt.optimizeFunction();
	}
	
	return success;
}

//---------------------------------------------------------------------------------------------

bool Compiler::isFunction(const char *name)
{
	return findFunction(name) ? TRUE : FALSE;
}

//---------------------------------------------------------------------------------------------

bool Compiler::isOperator(const char *name)
{
	return operators.find(name) ? TRUE : FALSE;
}

//---------------------------------------------------------------------------------------------

bool Compiler::isSysCall(const char *name)
{
	return sysCalls.find(name) ? TRUE : FALSE;
}

//---------------------------------------------------------------------------------------------

void Compiler::dumpTokens()
{
	for (Token *t = tokens.first(); t; t = t->next()) t->print();
}

//---------------------------------------------------------------------------------------------

void Compiler::dumpFunctions()
{
	for (Function *f = functions.first(); f; f = f->next()) f->print();
}
//---------------------------------------------------------------------------------------------

Function* Compiler::addFunction(const char *name, int line)
{
	if (Function *f = new Function(name, line))
	{
		if (f->parseSignature())
		{
			functions.addTail(f);
			return f;
		}
	}
	return NULL;
}

//---------------------------------------------------------------------------------------------
// 7-character label, null terminated (minimum buffer size: 8 bytes)

void Compiler::getUniqueLabel(char *label)
{
	unsigned int x = uniqueSeed;
	
	for (int i = 0; i < 7; i++)
	{
		*label++ = (x & 0xF) + 'g';
		x >>= 4;
	}

	*label = 0x00;
	uniqueSeed += 0xF4243;	
}

//---------------------------------------------------------------------------------------------

bool Compiler::addSysLibrary(const char* libname, int minver)
{
	LibraryToOpen *lib;
	
	lib = sysLibraries.find(libname);

	if (!lib)
	{
		if (!(lib = new LibraryToOpen(libname, minver))) return FALSE;
		sysLibraries.addTail(lib);
	}

	lib->bumpVersion(minver);
	return TRUE;
}

//---------------------------------------------------------------------------------------------

bool Compiler::generate(const char *inFileName, const char *outFileName)
{
	BPTR asmFile;
	bool success = FALSE;
	
	log.setModule("generator");
	
	if (asmFile = Open(outFileName, MODE_NEWFILE))
	{
		//-----------------------------------
		// add syscalls used in startup code
		//-----------------------------------
		
		useSysCall("OpenLibrary", "exec.library", -552);
		useSysCall("CloseLibrary", "exec.library", -414);
		useSysCall("Forbid", "exec.library", -132);
		useSysCall("FindTask", "exec.library", -294);
		useSysCall("GetMsg", "exec.library", -372);
		useSysCall("ReplyMsg", "exec.library", -378);
		useSysCall("WaitPort", "exec.library", -384);
		
		FPrintf(asmFile, ";\n; Compiled with Bzted %s from %s.\n;\n\n", BZTVER, inFileName); 
		generateLibOffsets(asmFile);
		generateStartup(asmFile);
		generateLibOpenClose(asmFile);
		generateBss(asmFile);
		Close(asmFile);
		success = TRUE;	
	}	
	
	return success;
}

//---------------------------------------------------------------------------------------------

const char* Compiler::determineMainFunctionName()
{
	Function *main;

	if (!(main = functions.find("Main")))
	{
		if (!(main = functions.find("main")))
		{
			main = functions.first();
			log.warning("no Main() or main() function defined, program starts from %s()",
			 main->name());			
		}
	}

	return main->name();
}

//---------------------------------------------------------------------------------------------

void Compiler::generateLibOffsets(BPTR asmFile)
{
	for (UsedSysCall *uc = usedSysCalls.first(); uc; uc = uc->next())
	{
		uc->generate(asmFile);
	}
	
	FPuts(asmFile, "\n");
}

//---------------------------------------------------------------------------------------------

void Compiler::generateStartup(BPTR asmFile)
{
	FPuts(asmFile,
		"pr_CLI                  = 172\n"
		"pr_MsgPort              = 92\n"
		"\n_Start:\n"
		"\t\tMOVEM.L\td2/d3/a2/a6,-(sp)\n"
		"\t\tCLR.L\td2\n"
		"\t\tSUBA.L\ta1,a1\n"
		"\t\tMOVEA.L\t4,a6\n"
		"\t\tJSR\tFindTask(a6)\n"
		"\t\tMOVEA.L\td0,a2\n"
		"\t\tTST.L\tpr_CLI(a2)\n"
		"\t\tBNE.S\t.1\n"
		"\t\tLEA\tpr_MsgPort(a2),a0\n"
		"\t\tJSR\tWaitPort(a6)\n"
		"\t\tLEA\tpr_MsgPort(a2),a0\n"
		"\t\tJSR\tGetMsg(a6)\n"
		"\t\tMOVE.L\td0,d2\n"
		".1:\t\tBSR.S\tOpenLibs\n"
		"\t\tBEQ.S\t.3\n"
	);

	FPrintf(asmFile, "\t\tJSR\t%s\n", determineMainFunctionName());

	FPuts(asmFile,
		".3:\t\tBSR.S\tCloseLibs\n"
		"\t\tMOVE.L\td0,d3\n"
		"\t\tTST.L\td2\n"
		"\t\tBEQ.S\t.2\n"
		"\t\tJSR\tForbid(a6)\n"
		"\t\tMOVEA.L\td2,a1\n"
		"\t\tJSR\tReplyMsg(a6)\n"
		".2:\t\tMOVE.L\td3,d0\n"
		"\t\tMOVEM.L\t(sp)+,d2/d3/a2/a6\n"
		"\t\tRTS\n\n"
	);
}

//---------------------------------------------------------------------------------------------

void Compiler::generateLibOpenClose(BPTR asmFile)
{
	int libCounter = 0;
	
	FPuts(asmFile,
		"OpenLibs:\n"
		"\t\tMOVEM.L\ta2/d2,-(sp)\n"
		"\t\tMOVEQ\t#1,d2\n"
		"\t\tLEA\tLibData,a2\n"
		".0:\t\tMOVE.L\t(a2)+,d0\n"
		"\t\tBEQ.S\t.1\n"
		"\t\tMOVEA.L\td0,a1\n"
		"\t\tMOVE.L\t(a2)+,d0\n"
		"\t\tJSR\tOpenLibrary(a6)\n"
		"\t\tTST.L\td0\n"
		"\t\tBEQ.S\t.2\n"
		"\t\tMOVEA.L\t(a2)+,a0\n"
		"\t\tMOVE.L\td0,(a0)\n"
		"\t\tBRA.S\t.0\n"
		".2:\t\tMOVEQ\t#0,d2\n"
		".1:\t\tMOVE.L\td2,d0\n"		
		"\t\tMOVEM.L\t(sp)+,a2/d2\n"
		"\t\tRTS\n\n");

	FPuts(asmFile,
		"CloseLibs:\n"
		"\t\tMOVE.L\ta2,-(sp)\n"
		"\t\tLEA\tLibData,a2\n"
		".0:\t\tTST.L\t(a2)\n"
		"\t\tBEQ.S\t.1\n"
		"\t\tMOVEA.L\t8(a2),a0\n"
		"\t\tMOVE.L\t(a0),d0\n"
		"\t\tBEQ.S\t.2\n"
		"\t\tMOVEA.L\td0,a1\n"
		"\t\tJSR\tCloseLibrary(a6)\n"
		".2:\t\tLEA\t12(a2),a2\n"
		"\t\tBRA.S\t.0\n"
		".1:\t\tMOVEA.L\t(sp)+,a2\n"
		"\t\tRTS\n\n");
	
	FPuts(asmFile, "LibData:\n");

	for (LibraryToOpen *lib = sysLibraries.first(); lib; lib = lib->next())
	{
		FPrintf(asmFile, "\t\tDC.L\tlibname%ld,%ld,%s\n", libCounter++, lib->getVersion(),
		 lib->getBase());
	}	
	
	FPuts(asmFile, "\t\tDC.L\t0\n");

	libCounter = 0;
	
	for (LibraryToOpen *lib = sysLibraries.first(); lib; lib = lib->next())
	{
		const char *pad = "";
		if ((StrLen(lib->name) & 1) == 0) pad = ",0";
		FPrintf(asmFile, "libname%ld:\n\t\tDC.B\t\"%s\",0%s\n", libCounter++, lib->name, pad);
	}	

}

//---------------------------------------------------------------------------------------------

void Compiler::generateBss(BPTR asmFile)
{
	FPuts(asmFile, "\n\t\tbss\n\n");

	//---------------------------
	// bases of system libraries
	//---------------------------
	
	for (LibraryToOpen *lib = sysLibraries.first(); lib; lib = lib->next())
	{
		FPrintf(asmFile, "%s:\n\t\tDS.L\t1\n", lib->getBase());
	}
	
	//-------------
	// data frames
	//-------------
	
	for (DataFrame *df = dataFrames.first(); df; df = df->next())
	{
		FPrintf(asmFile, "%s:\n\t\tDS.L\t%ld\n", df->label, df->size);
	}
}

//---------------------------------------------------------------------------------------------

bool Compiler::useSysCall(const char *callName, const char *libName, int offset)
{
	UsedSysCall *uc;
	
	uc = usedSysCalls.find(callName);
	if (uc) return TRUE;
	uc = new UsedSysCall(callName, libName, offset);
	if (!uc) return FALSE;
	usedSysCalls.addTail(uc);
	return TRUE;
}

//---------------------------------------------------------------------------------------------

const char* Compiler::addDataFrame(int size)
{
	if (char *label = new char[8])
	{
		getUniqueLabel(label);

		if (DataFrame *df = new DataFrame(label, size))
		{
			dataFrames.addTail(df);
			return label;
		}

		delete label;
	}

	return FALSE;
}

//---------------------------------------------------------------------------------------------
// Finds the first function matching the name, ignoring the signature.

Function* Compiler::findFunction(const char *key)
{
	for (Function *f = functions.first(); f; f = f->next())
	{
		const char *n = f->name();

		while (*key && (*key++ == *n++));

		if (*key == 0x00) return f;
	}

	return NULL;
}
