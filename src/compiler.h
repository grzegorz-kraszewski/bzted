#include "main.h"
#include "rplist.h"
#include "keyedarray.h"
#include "token.h"
#include "function.h"
#include "builtins.h"

class Compiler
{
	RpList<Token> tokens;
	RpKeyedList<Function> functions;
	KeyedArray<Operator> operators;
	//void expandArgsResults();
	Token* translateDefinition(Token *t);
	Token* translateFunction(Token *t, Function *f);

	public:

	Compiler()
	{
		operators.fetch(BuiltIns, 11);
	}

	void scan(const char *fileName);
	void lex();
	void translate();
	void transform();
	void dumpTokens();
	void dumpFunctions();
	bool isFunction(const char *name);
	bool isOperator(const char *name);
	void addFunction(const char *name);
	Operator* findOperator(const char *name) { return operators.find(name); }
	Function* findFunction(const char *name) { return functions.find(name); }
};
