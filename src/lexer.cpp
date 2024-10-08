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

	for (Token *t = tokens.first(); success && t; t = t->next())
	{
		char c = t->text[0];

		switch(c)
		{
			case '$':
				success = t->parseHexNumber();
			break;

			case '%':
				if (t->textSize > 1) success = t->parseBinNumber();
				else success = t->parseIdentifier();
			break;

			case 0x22:
			case 0x27:
				success = t->parseString();
			break;

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

	if (success)
	{
		if (success = collectDefinitions())
		{
			success = updateIdentifiers();
		}
	}

	return success;
}

//---------------------------------------------------------------------------------------------

bool Lexer::collectDefinitions()
{
	Token *ahead;
	bool success = TRUE;

	for (Token *token = tokens.first(); token; token = token->next())
	{
		if (token->type == TT_DEF)
		{
			if (ahead = token->next())
			{
				if (ahead->type == TT_OPR)
				{
					if (StrCmp(ahead->text, "{") == 0)
					{
						//-----------------------------
						// regular function definition
						//-----------------------------

						if (!(Comp->addFunction(token->text, token->lineNum))) success = FALSE;
					}
					else if (StrCmp(ahead->text, "[") == 0)
					{

						//---------------------------------
						// data frame generator definition
						//---------------------------------

						if (!(Comp->addFunction(token->text, token->lineNum))) success = FALSE;
					}
					else
					{
						log.lineError(ahead->lineNum, "unexpected operator '%s' in '%s'",
						 ahead->text, token->text);
						success = FALSE;
					}
				}
				else
				{
					log.lineError(ahead->lineNum, "unexpected token '%s' in '%s'",
					 ahead->text, token->text);
					success = FALSE;
				}
			}
			else
			{
				log.lineError(token->lineNum, "unexpected end of code in '%s'",
				 token->text);
				success = FALSE;
			}
		}
	}

	return success;
}

//---------------------------------------------------------------------------------------------
// Searches for each identifier token in the array of function definitions. 

bool Lexer::updateIdentifiers()
{
	bool result = TRUE;

	for (Token *token = tokens.first(); token; token = token->next())
	{
		if (token->type == TT_IDN)
		{
			if (Comp->isFunction(token->text)) token->type = TT_FNC;
			else
			{
				log.lineError(token->lineNum, "unknown identifer '%s'", token->text);
				result = FALSE;
			}
		}
	}

	return result;
}
