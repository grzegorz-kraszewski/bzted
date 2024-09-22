#include "compiler.h"
#include "logger.h"

#include <proto/exec.h>
#include <proto/dos.h>


const char LastChar(const char *s)
{
	if (!*s) return 0x00;
	while (*s) s++;
	return *(--s);
}

/*-------------------------------------------------------------------------------------------*/

bool Token::append(char *buf, LONG len)
{
	int newsize = textSize + len;
	char *newtext = new char[newsize + 1];

	if (newtext)
	{
		if (text)
		{
			CopyMem(text, newtext, textSize);
			delete text;
		}

		CopyMem(buf, newtext + textSize, len);
		newtext[textSize + len] = 0x00;
		textSize = newsize;
		text = newtext;
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------*/

bool Token::parseDecNumber()
{
	LONG val = 0;
	BOOL leadzero = TRUE;
	BOOL positive = TRUE;
	char c, *p = text;

	if (*p == '+') p++;                          // ignore leading '+'
	if (*p == '-') { positive = FALSE; p++; }

	while (c = *p++)
	{
		if (leadzero && (c == '0')) continue;
		else leadzero = FALSE;
		if ((c >= '0') && (c <= '9')) c -= '0';
		else
		{
			log.error("%ld: '%s', unexpected character in decimal number", lineNum, text);
			return FALSE;
		}

		if (positive)
		{
			if ((val < 214748364) || ((val == 214748364) && (c < 8)))
			{
				val *= 10;
				val += c;
			}
			else
			{
				log.error("%ld: '%s', decimal number out of 32-bit range", lineNum, text);
				return FALSE;
			}
		}
		else
		{
			if ((val > -214748364) || ((val == -214748364) && (c < 9)))
			{
				val *= 10;
				val -= c;
			}
			else
			{
				log.error("%ld: '%s', decimal number out of 32-bit range", lineNum, text);
				return FALSE;
			}
		}
	}

	intValue = val;
	type = TT_INT;
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

bool Token::parseHexNumber()
{
	LONG val = 0;
	WORD digit = 0;
	BOOL leadzero = TRUE;
	char c, *p = text + 1;      // skip '$' prefix

	if (!(*p))
	{
		log.error("%ld: '%s', no hexadecimal digits after '$' prefix", lineNum, text);
		return FALSE;
	}

	while ((c = *p++) && (digit < 8))
	{
		if (leadzero && (c == '0')) continue;
		else leadzero = FALSE;
		val <<= 4;
		if ((c >= '0') && (c <= '9')) c -= '0';
		else if ((c >= 'A') && (c <= 'F')) c -= 55;
		else if ((c >= 'a') && (c <= 'f')) c -= 87;
		else
		{
			log.error("%ld: '%s', unexpected character in hexadecimal number", lineNum, text);
			return FALSE;
		}

		val += c;
		digit++;			
	};

	if (c)
	{
		log.error("%ld: '%s', hexadecimal number too big for 32 bits", lineNum, text);
		return FALSE;
	}

	intValue = val;
	type = TT_INT;
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

bool Token::parseBinNumber()
{
	LONG val = 0;
	WORD digit = 0;
	BOOL leadzero = TRUE;
	char c, *p = text + 1;      // skip '%' prefix

	if (!(*p))
	{
		log.error("%ld: '%s', no binary digits after '%' prefix", lineNum, text);
		return FALSE;
	}

	while ((c = *p++) && (digit < 32))
	{
		if (leadzero && (c == '0')) continue;
		else leadzero = FALSE;
		val <<= 1;
		if (c == '1') val |= 1;
		else if (c != '0')
		{
			log.error("%ld: '%s', unexpected character in binary number", lineNum, text);
			return FALSE;
		}

		digit++;			
	};

	if (c)
	{
		log.error("%ld: '%s', binary number too big for 32 bits", lineNum, text);
		return FALSE;
	}

	intValue = val;
	type = TT_INT;
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

bool Token::parseString()
{
	char *p = text;
	char delimiter = *p;
	char *m = p + 1;

	while (*m != delimiter) *p++ = *m++;
	*p = 0x00;
	textSize -= 2;

	type = TT_STR;
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

bool Token::parseIdentifier()
{
	// Operators before definitions, so operator ':' 
	// is not taken as definition with empty name.
	//-----------------------------------------------

	if (Comp->isOperator(text)) { type = TT_OPR; return TRUE; }

	if (LastChar(text) == ':') return parseDefinition();
	
	if (Comp->isSysCall(text)) { type = TT_SYS; return TRUE; }

	type = TT_IDN;
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

bool Token::parseDefinition()
{
	char *p = text;

	while (*p) p++;
	*--p = 0x00;
	textSize -= 1;
	type = TT_DEF;
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

bool Token::translate(Function *function)
{
	BOOL success = TRUE;
	switch (type)
	{
		case TT_INT:
		{
			Operand op1(IIOP_IMMEDIATE, intValue);
			Operand op2(IIOP_VIRTUAL, 0);
			InterInstruction *ii0 = new InterInstruction(II_DMOV, op1, op2);
			InterInstruction *ii1 = new InterInstruction(II_DROP, op2);

			if (ii0 && ii1)
			{
				function->addCode(ii0);
				function->addCode(ii1);
			}
			else success = FALSE;
		}
		break;

		case TT_OPR:
			success = Comp->findOperator(text)->generator(function);
		break;

		case TT_FNC:
		{
			Operand op(IIOP_LABEL, (int)text);
			InterInstruction *ii = new InterInstruction(II_JSBR, op);
			if (ii) function->addCode(ii);
			else success = FALSE;
		}
		break;
		
		case TT_SYS:
		{
			success = GenerateSysCall(text, function);
		}
		break;

		default:
			success = FALSE;		
	}

	return success;
}

//---------------------------------------------------------------------------------------------

void Token::print()
{
	switch (type)
	{
		case TT_INT:
			Printf("<%s> `%s` [%ld] = %ld\n", &type, text, textSize, intValue);
		break;

		default:
			Printf("<%s> `%s` [%ld]\n", &type, text, textSize);
	} 
}

