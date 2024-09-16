#include <proto/exec.h>
#include <proto/dos.h>

#include "compiler.h"
#include "logger.h"
#include "scanner.h"
#include "lexer.h"
#include "optimizer.h"


//---------------------------------------------------------------------------------------------

void Compiler::scan(const char *fileName)
{
	Scanner scanner(&tokens);

	log.setModule("scanner");
	scanner.scan(fileName);
}

//---------------------------------------------------------------------------------------------

void Compiler::lex()
{
	Lexer lexer(&tokens);

	log.setModule("lexer");
	lexer.lex();
}

/*-------------------------------------------------------------------------------------------*/

void Compiler::translate()
{
	Token *token = tokens.first();

	if (!token) { log.error ("no tokens in code"); return; }
	while (token = translateDefinition(token));
}

/*-------------------------------------------------------------------------------------------*/

Token* Compiler::translateDefinition(Token *token)
{
	Function *function;

	if (token->type != TT_DEF)
	{
		log.error("%ld: '%s', code outside of function", token->lineNum, token->text);
		return NULL;
	}

	if (function = functions.find(token->text)) return translateFunction(token, function);
	else
	{
		log.error("%ld: '%s', unknown type of definition", token->lineNum, token->text);
		return NULL;
	}
}

//---------------------------------------------------------------------------------------------
// Token passed is the definition name.

Token* Compiler::translateFunction(Token *token, Function *function)
{
	token = token->next();   // opening bracket (verified earlier)
	token = token->next();   // the first token of function body

	while (token)
	{
		switch (token->type)
		{
			case TT_INT:
			case TT_STR:
			case TT_FNC:
			case TT_SYS:
				if (!token->translate(function)) return NULL;
			break;

			case TT_OPR:
				if (StrCmp(token->text, "{") == 0)
				{
					log.error("%ld: '{', nested anonymous functions not yet implemented", token->lineNum);
					return NULL;
				}
				else
				{
					if (!token->translate(function)) return NULL;
					if (StrCmp(token->text, "}") == 0)
					{
						function->stackSignature();
						return token->next();
					}
				}
			break;

			case TT_DEF:
				log.error("%ld: '%s', nested definitions are not allowed", token->lineNum, token->text);
				return NULL;
			break;
		}

		token = token->next();
	}	

	log.error("%ld: unexpected end of file inside function", tokens.last()->lineNum);
	return NULL;
}

//---------------------------------------------------------------------------------------------

void Compiler::transform()
{
	for  (Function *f = functions.first(); f; f = f->next())
	{
		f->expand();
		f->expandAllCalls();
		f->replaceAllPushPullBlocks();
	}	
}

//---------------------------------------------------------------------------------------------

void Compiler::optimize()
{
	for  (Function *f = functions.first(); f; f = f->next())
	{
		Optimizer opt(f);

		opt.optimizeFunction();
	}
}

//---------------------------------------------------------------------------------------------

bool Compiler::isFunction(const char *name)
{
	return functions.find(name) ? TRUE : FALSE;
}

//---------------------------------------------------------------------------------------------

bool Compiler::isOperator(const char *name)
{
	return operators.find(name) ? TRUE : FALSE;
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

void Compiler::addFunction(const char *name)
{
	if (Function *f = new Function(name)) functions.addTail(f);
}

//---------------------------------------------------------------------------------------------
