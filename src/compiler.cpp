#include <proto/exec.h>
#include <proto/dos.h>

#include "compiler.h"


#define IsBracket(c) IsInString((c), "()[]{}")


LONG IsSpace(char c)
{
	return ((c == 0x20) || (c == 0x09) || (c == 0x0A) || (c == 0x0D));
}


LONG IsDigit(char c)
{
	return ((c >= '0') && (c <= '9'));	
}


BOOL IsInString(char c, char *s)
{
	char x;

	while (x = *s++) if (x == c) break;
	return x;
}


/*-------------------------------------------------------------------------------------------*/

void Compiler::scan(char *filename)
{
	BPTR file;
	int chr, error;
			
	if (file = Open(filename, MODE_OLDFILE))
	{
		while (((chr = FGetC(file)) >= 0) && processChar((char)chr));

		if (error = IoErr()) PrintFault(error, "error reading file");
		else
		{
			if (strmode) Printf("Unterminated string at end of file (missing `%lc`).\n", strmode);
			else Printf("Scanning complete, %ld lines, %ld tokens, %ld bytes used.\n", linenum,
			tokencount, tokencount * sizeof(Token));
		}

		Close(file);
	}
	else PrintFault(IoErr(), "error opening source code");
}

/*-------------------------------------------------------------------------------------------*/

void Compiler::lex()
{
	SysListIter<Token> tks(tokens);
	Token *t;
	BOOL success = TRUE;

	while (success && (t = tks++))
	{
		char c = t->text[0];

		switch(c)
		{
			case '$':   success = t->parseHexNumber(); break;
			case '%':   success = t->parseBinNumber(); break;
			case 0x22:
			case 0x27:  success = t->parseString(); break;
			case '-':
				if (t->textsize > 1) success = t->parseDecNumber();
				else success = t->parseIdentifier();
			break;
			case '+':
				if (t->textsize > 1) success = t->parseDecNumber();
				else success = t->parseIdentifier();
			break;
			default:
				if (IsDigit(c)) success = t->parseDecNumber();
				else success = t->parseIdentifier();
			break;
		}
	}

	grabDefinitions();
	updateIdentifiers();
}

/*-------------------------------------------------------------------------------------------*/

void Compiler::grabDefinitions()
{
	SysListIter<Token> tks(tokens);
	Token *token, *ahead;
	Function *newFunc;

	while (token = tks++)
	{
		if (token->type != TT_DEF) continue;
		ahead = tokens.next(token);
		if (!ahead) continue;
		if (ahead->type != TT_OPR) continue;
		if (StrCmp(ahead->text, "{") != 0) continue;		
		newFunc = new Function(token->text);
		if (!newFunc) return; /* compileErr(token, "out of memory"); */
		functions.addtail(newFunc);
	}
}

/*-------------------------------------------------------------------------------------------*/

void Compiler::updateIdentifiers()
{
	SysListIter<Token> tks(tokens);
	Token *token;

	while (token = tks++)
	{
		if (token->type == TT_IDN)
		{
			if (functions.find(token->text)) token->type = TT_FNC;
			else compileErr(token, "unknown identifer");
		}
	}
}

/*-------------------------------------------------------------------------------------------*/

void Compiler::compileCode()
{
	Token *token = tokens.first();
	const char *functionName = NULL;
	BOOL success = TRUE;

	if (!token) { generalErr("no tokens in code"); return; }
	while (token = compileDefinition(token));
}

/*-------------------------------------------------------------------------------------------*/

Token* Compiler::compileDefinition(Token *token)
{
	const char *objectName = token->text;
	Function *function;
	Token *ahead;

	if (token->type != TT_DEF) return compileErr(token, "code outside of function");
	if (function = functions.find(token->text)) return compileFunction(token, function);
	else return compileErr(token, "unknown type of definition");

}

//---------------------------------------------------------------------------------------------
// Token passed is the definition name.

Token* Compiler::compileFunction(Token *token, Function *function)
{
	token = tokens.next(token);   // opening bracket (verified earlier)
	token = tokens.next(token);   // the first token of function body
	while (token)
	{
		switch (token->type)
		{
			case TT_INT:
			case TT_STR:
			case TT_FNC:
			case TT_SYS:
				if (!token->compile(function)) return NULL;
			break;

			case TT_OPR:
				if (StrCmp(token->text, "{") == 0)
					return compileErr(token, "nested anonymous functions not yet implemented");
				else
				{
					if (!token->compile(function)) return NULL;
					if (StrCmp(token->text, "}") == 0)
					{
						function->stackSignature();
						return token->next();
					}
				}
			break;

			case TT_DEF:
				return compileErr(token, "nested definitions are not allowed");
		}

		token = tokens.next(token);
	}	

	return compileErr(token->pred, "unexpected end of file inside function");
}

/*-------------------------------------------------------------------------------------------*/

void Compiler::expandArgsResults()
{
	SysListIter<Function> fncs(functions);
	Function *f;

	while (f = fncs++)
	{
		f->expand();
		f->expandAllCalls();
	}	
}

//---------------------------------------------------------------------------------------------


BOOL Compiler::processChar(char c)
{
	if (c == 0x0A)
	{
		linenum++;
		comment = FALSE;
	}

	if (c == 0x00)
	{
		Printf("Unexpected character NUL (0x00) in line %ld (binary file?).\n", linenum);
		return FALSE;
	}

	if (comment) return TRUE;

	if (c == '#')
	{
		comment = TRUE;
		return TRUE;
	}

	if (!strmode)
	{
		if ((c == 0x22) || (c == 0x27))
		{
			if (bufpos > 0)
			{
				Printf("Missing space before string in line %ld.\n", linenum);
				return FALSE;
			}
			else
			{
				strmode = c;
				addChar(c);
			}
		}
		else if (IsBracket(c))
		{
			flush();
			addChar(c);
			flush();
		}
		else if (IsSpace(c)) flush();
		else addChar(c);
	}
	else
	{
		addChar(c);

		if (c == strmode)
		{
			flush();
			strmode = 0;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Compiler::addChar(char c)
{
	if (!havetoken)
	{
		Token *t = new Token;

		if (t)
		{
			t->linenum = linenum;
			tokens.addtail(t);
			havetoken = TRUE;
			tokencount++;
		}
		else return FALSE;
	}

	buf[bufpos++] = c;

	if (bufpos >= 64)
	{
		tokens.last()->append(buf, 64);
		bufpos = 0;
	}

	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Compiler::flush()
{
	BOOL success = TRUE;

	if (bufpos > 0)
	{
		success = tokens.last()->append(buf, bufpos);
		bufpos = 0;
	}

	havetoken = FALSE;
}

/*-------------------------------------------------------------------------------------------*/

void Compiler::dumpTokens()
{
	Token *t;

	for (t = tokens.first(); t; t = t->next())
	{
		switch (t->type)
		{
			case MAKE_ID3('i','n','t'):
				Printf("<%s> `%s` [%ld] = %ld\n", &t->type, t->text, t->textsize, t->intval);
			break;
			default:
				Printf("<%s> `%s` [%ld]\n", &t->type, t->text, t->textsize);
		}
	} 
}

/*-------------------------------------------------------------------------------------------*/

void Compiler::dumpFunctions()
{
	Function *f;

	for (f = functions.first(); f; f = functions.next(f)) f->print();
}

/*-------------------------------------------------------------------------------------------*/

Token* Compiler::compileErr(Token *token, const char *msg)
{
	Printf("Error in line %ld: '%s' %s.\n", token->linenum, token->text, msg);
	return NULL;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Compiler::generalErr(const char *msg)
{
	Printf("Error: %s.\n", msg);
	return FALSE;
}
