#include "main.h"
#include "token.h"
#include "function.h"
#include "builtins.h"

class Compiler
{
	SysList<Token> tokens;
	SysList<InterInstruction> intercode;
	char buf[64];
	int linenum;
	int bufpos;
	int tokencount;
	BOOL havetoken;
	BOOL comment;
	char strmode;

	BOOL processChar(char c);
	BOOL addChar(char c);
	BOOL flush();

	public:

	KeyedSysList<Function> functions;
	KeyedArray<Operator> operators;

	Compiler()
	{
		linenum = 1;
		bufpos = 0;
		tokencount = 0;
		havetoken = FALSE;
		comment = FALSE;
		strmode = 0;
		operators.fetch(BuiltIns, 11);
	}

	void scan(char *filename);
	void lex();
	void grabDefinitions();
	void updateIdentifiers();
	void expandArgsResults();
	void compileCode();
	Token* compileFunction(Token *t, Function *f);
	Token* compileDefinition(Token *t);
	void optimizeCode();
	void dumpTokens();
	void dumpFunctions();
	BOOL generalErr(const char *msg);
	Token* compileErr(Token *token, const char *msg);
};
