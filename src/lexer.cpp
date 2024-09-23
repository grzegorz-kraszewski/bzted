//----------------------------------
// lexer
//   * determines basic token types
//   * parses numeric literals
//----------------------------------

#include "compiler.h"
#include "lexer.h"
#include "main.h"
#include "logger.h"

bool Lexer::lex()
{
	BOOL success = TRUE;

	for (Token *t = tokens->first(); success && t; t = t->next())
	{
		char c = t->text[0];

		switch(c)
		{
			case '$':   success = t->parseHexNumber(); break;
			case '%':
				if (t->textSize > 1) success = t->parseBinNumber();
				else success = t->parseIdentifier();
			break;
			case 0x22:
			case 0x27:  success = t->parseString(); break;
			case '-':
				if (t->textSize > 1) success = t->parseDecNumber();
				else success = t->parseIdentifier();
			break;
			case '+':
				if (t->textSize > 1) success = t->parseDecNumber();
				else success = t->parseIdentifier();
			break;
			default:
				if (IsDigit(c)) success = t->parseDecNumber();
				else success = t->parseIdentifier();
			break;
		}
	}

	collectDefinitions();
	updateIdentifiers();
	return success;
}

//---------------------------------------------------------------------------------------------

void Lexer::collectDefinitions()
{
	Token *ahead;

	for (Token *token = tokens->first(); token; token = token->next())
	{
		if (token->type != TT_DEF) continue;
		ahead = token->next();
		if (!ahead) continue;
		if (ahead->type != TT_OPR) continue;

		if ((StrCmp(ahead->text, "{") == 0) || (StrCmp(ahead->text, "[") == 0))
		{
			Comp->addFunction(token->text);
		}
	}
}

//---------------------------------------------------------------------------------------------

void Lexer::updateIdentifiers()
{
	for (Token *token = tokens->first(); token; token = token->next())
	{
		if (token->type == TT_IDN)
		{
			if (Comp->isFunction(token->text)) token->type = TT_FNC;
			else log.error("%ld: '%s', unknown identifer", token->lineNum, token->text);
		}
	}
}
