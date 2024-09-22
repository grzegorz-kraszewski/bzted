#include "main.h"
#include "rplist.h"
#include "keyedarray.h"
#include "token.h"
#include "function.h"
#include "builtins.h"
#include "syscalls.h"

class UsedSysCall : public RpNamedNode<UsedSysCall>
{
	public:
	
	const char *libName;
	int offset;
	
	UsedSysCall(const char *cn, const char *ln, int off) : RpNamedNode<UsedSysCall>(cn)
	{
		libName = ln;
		offset = off;
	}
	
	void generate(BPTR file);
};


class Compiler
{
	RpList<Token> tokens;
	RpKeyedList<Function> functions;
	KeyedArray<Operator> operators;
	KeyedArray<SysCall> sysCalls;
	KeyedArray<const char*> libraryBases;
	RpKeyedList<LibraryToOpen> sysLibraries;
	RpNamedList<UsedSysCall> usedSysCalls;
	unsigned int uniqueSeed;
	bool transResult;
	
	Token* translateDefinition(Token *t);
	Token* translateCodeBlock(Token *t, Function *f);
	void generateLibOffsets(BPTR asmFile);
	void generateStartup(BPTR asmFile);
	void generateLibOpenClose(BPTR asmFile);
	void generateBss(BPTR asmFile);
	const char* determineMainFunctionName();

	public:

	Compiler()
	{
		operators.fetch(BuiltIns, 13);
		sysCalls.fetch(SysCalls, 22); 
		libraryBases.fetch(BaseNames, 3);
		uniqueSeed = 0x000F4243;
	}

	bool scan(const char *fileName);
	bool lex();
	bool translate();
	bool transform();
	bool optimize();
	bool generate(const char *inFileName, const char *outFileName);
	void dumpTokens();
	void dumpFunctions();
	bool isFunction(const char *name);
	bool isOperator(const char *name);
	bool isSysCall(const char *name);
	Function* addFunction(const char *name);
	Operator* findOperator(const char *name) { return operators.find(name); }
	Function* findFunction(const char *name) { return functions.find(name); }
	SysCall* findSysCall(const char *name) { return sysCalls.find(name); }
	void getUniqueLabel(char *label);
	bool addSysLibrary(const char *libname, int minver);
	bool useSysCall(const char *cn, const char *ln, int off);
	const char* getLibraryBaseName(const char *libname) { return *libraryBases.find(libname); }
};
