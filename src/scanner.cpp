//---------------------------
// source scanner
//   * reads source file
//   * cuts file into tokens
//   * discards comments
//   * extracts strings
//---------------------------

#include "scanner.h"
#include "logger.h"
#include "main.h"

#include <proto/dos.h>

#define IsBracket(c) IsInString((c), "[]{}")

//---------------------------------------------------------------------------------------------

bool Scanner::scan(const char *filename)
{
	BPTR file;
	int chr, error;
	bool result = FALSE;
	
	if (file = Open(filename, MODE_OLDFILE))
	{
		while (((chr = FGetC(file)) >= 0) && processChar((char)chr));

		if (error = IoErr()) log.error(log.fault(error, "error reading file"));
		else
		{
			if (stringMode) log.error("unterminated string at end of file (missing `%lc`)",
				stringMode);
			else
			{
				log.info("scanning complete, %ld lines, %ld tokens", lineNum, tokenCount);
				result = TRUE;
			}
		}

		Close(file);
	}
	else log.error(log.fault(IoErr(), "opening source file"));

	return result;
}

//---------------------------------------------------------------------------------------------

bool Scanner::processChar(char c)
{
	if (c == 0x0A)
	{
		lineNum++;
		comment = FALSE;
	}

	if (c == 0x00)
	{
		log.lineError(lineNum, "unexpected character 0x00 (binary file?)");
		return FALSE;
	}

	if (comment) return TRUE;

	if (c == '#')
	{
		comment = TRUE;
		return TRUE;
	}

	if (!stringMode)
	{
		if ((c == 0x22) || (c == 0x27))
		{
			if (bufPos > 0)
			{
				log.lineError(lineNum, "missing space before string");
				return FALSE;
			}
			else
			{
				stringMode = c;
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

		if (c == stringMode)
		{
			flush();
			stringMode = 0;
		}
	}

	return TRUE;
}

//---------------------------------------------------------------------------------------------

bool Scanner::addChar(char c)
{
	if (!haveToken)
	{
		Token *t = new Token;

		if (t)
		{
			t->lineNum = lineNum;
			tokens->addTail(t);
			haveToken = TRUE;
			tokenCount++;
		}
		else return FALSE;
	}

	buf[bufPos++] = c;

	if (bufPos >= 64)
	{
		tokens->last()->append(buf, 64);
		bufPos = 0;
	}

	return TRUE;
}

//---------------------------------------------------------------------------------------------

bool Scanner::flush()
{
	bool success = TRUE;

	if (bufPos > 0)
	{
		success = tokens->last()->append(buf, bufPos);
		bufPos = 0;
	}

	haveToken = FALSE;

	return success;
}
